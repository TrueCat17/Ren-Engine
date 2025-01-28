#include "py_utils.h"
#include "py_utils/make_func.h"
#include "py_utils/absolute.h"


#include <deque>
#include <map>
#include <mutex>


#include "gui/gui.h"
#include "logger.h"

#include "media/py_set_globals.h"

#include "parser/mods.h"

#include "utils/algo.h"
#include "utils/string.h"
#include "utils/utils.h"


using PyCode = std::tuple<const std::string, const std::string, uint32_t>;
static std::map<PyCode, PyObject*> compiledObjects;

static std::map<std::string, PyObject*> constObjects;

static const std::string True = "True";
static const std::string False = "False";
static const std::string None = "None";


static PyObject *tracebackModule = nullptr;
static PyObject *formatTraceback = nullptr;
static PyObject *builtinDict = nullptr;
static PyObject *builtinStr = nullptr;


PyObject *PyUtils::global = nullptr;
PyObject *PyUtils::tuple1 = nullptr;
PyObject *PyUtils::spaceStr = nullptr;
PyObject *PyUtils::spaceStrJoin = nullptr;



static void finalizeInterpreterImpl() {
	if (!Py_IsInitialized()) return;

	for (auto &i : constObjects) {
		Py_DECREF(i.second);
	}
	constObjects.clear();

#if PY_MINOR_VERSION >= 12
	for (auto &i : compiledObjects) {
		Py_DECREF(i.second);
	}
	compiledObjects.clear();
#endif

	clearPyWrappers();
	Mods::clearList();
	GUI::clearScreenTimes();

	if (PyUtils::tuple1) {
		PyTuple_SET_ITEM(PyUtils::tuple1, 0, nullptr);
		Py_DECREF(PyUtils::tuple1);
	}
	Py_DECREF(builtinStr);
	Py_DECREF(tracebackModule);
	Py_DECREF(formatTraceback);

	Py_DECREF(PyUtils::spaceStr);
	Py_DECREF(PyUtils::spaceStrJoin);

	Py_Finalize();
}

static void initInterpreterImpl() {
	finalizeInterpreterImpl();
	Py_Initialize();

	PyUtils::tuple1 = PyTuple_New(1);
	builtinStr = PyUnicode_FromString("builtins");

	tracebackModule = PyImport_ImportModule("traceback");
	if (!tracebackModule) {
		PyErr_Print();
		std::abort();
	}
	formatTraceback = PyObject_GetAttrString(tracebackModule, "format_tb");
	if (!formatTraceback) {
		Utils::outMsg("PyUtils::initInterpreter", "traceback.format_tb == nullptr");
		std::abort();
	}

	PyObject *builtinModule = PyImport_AddModule("builtins");
	builtinDict = PyModule_GetDict(builtinModule);
	if (!builtinDict) {
		Utils::outMsg("PyUtils::initInterpreter", "builtins == nullptr");
		std::abort();
	}

	PyObject *main = PyImport_AddModule("__main__");
	PyUtils::global = PyModule_GetDict(main);

	PyUtils::spaceStr = PyUnicode_FromString(" ");
	PyUtils::spaceStrJoin = PyObject_GetAttrString(PyUtils::spaceStr, "join");

	if (!PyAbsolute_PreInit()) {
		Utils::outMsg("PyUtils::initInterpreter", "Failure on call PyAbsolute_PreInit()");
	}
	if (PyType_Ready(&PyAbsolute_Type) < 0) {
		Utils::outMsg("PyUtils::initInterpreter", "Can't initialize absolute type");
	}else {
		PyDict_SetItemString(builtinDict, "absolute", (PyObject*)&PyAbsolute_Type);
	}

	PySetGlobals::set(PyUtils::global, builtinDict, builtinStr);
}


static std::thread::id pythonThreadId;
static size_t funcsForPyThreadIndex = 0;
static std::deque<std::pair<const std::function<void()>*, size_t>> funcsForPyThread;
static std::deque<size_t> funcsForPyThreadCalced;
static std::mutex funcsForPyThreadMutex;
static std::mutex funcsForPyThreadCalcedMutex;

static void initImpl() {
	Utils::setThreadName("python");
	pythonThreadId = std::this_thread::get_id();

	while (true) {
		if (!funcsForPyThread.empty()) {
			const std::function<void()> *func;
			size_t funcId;
			{
				std::lock_guard g(funcsForPyThreadMutex);
				auto &p = funcsForPyThread.front();
				func = p.first;
				funcId = p.second;
				funcsForPyThread.pop_front();
			}
			(*func)();
			{
				std::lock_guard g(funcsForPyThreadCalcedMutex);
				funcsForPyThreadCalced.push_back(funcId);
			}
		}else {
			Utils::sleep(50 * 1e-6);
		}
	}
}
void PyUtils::init() {
	std::thread(initImpl).detach();
}

void PyUtils::callInPythonThread(const std::function<void()> &func) {
	if (std::this_thread::get_id() == pythonThreadId) {
		func();
		return;
	}

	size_t funcId;
	{
		std::lock_guard g(funcsForPyThreadMutex);
		funcId = ++funcsForPyThreadIndex;
		funcsForPyThread.push_back({&func, funcId});
	}
	while (true) {
		{
			std::lock_guard g(funcsForPyThreadCalcedMutex);
			if (!funcsForPyThreadCalced.empty() && funcsForPyThreadCalced.front() == funcId) {
				funcsForPyThreadCalced.pop_front();
				return;
			}
		}
		Utils::sleep(10 * 1e-6);
	}
}


void PyUtils::finalizeInterpreter() {
	callInPythonThread(finalizeInterpreterImpl);
}
void PyUtils::initInterpreter() {
	callInPythonThread(initInterpreterImpl);
}




static PyObject* getCompileObjectImpl(const std::string &code, const std::string &fileName, uint32_t numLine) {
	std::tuple<const std::string&, const std::string&, uint32_t> refPyCode(code, fileName, numLine);

	auto i = compiledObjects.find(refPyCode);
	if (i != compiledObjects.end()) {
		return i->second;
	}

	std::string indentCode;
	indentCode.reserve(numLine - 1 + code.size());
	indentCode.insert(0, numLine - 1, '\n');
	indentCode += code;

	PyObject *res = Py_CompileStringFlags(indentCode.c_str(), fileName.c_str(), Py_file_input, nullptr);
	if (!res) {
		PyUtils::errorProcessing(code);
	}

	return compiledObjects[refPyCode] = res;
}
PyObject* PyUtils::getCompileObject(const std::string &code, const std::string &fileName, uint32_t numLine) {
	PyObject *res;
	callInPythonThread([&]() {
		res = getCompileObjectImpl(code, fileName, numLine);
	});
	return res;
}




static void getRealString(const std::string_view &code, std::string &fileName, std::string &numLineStr) {
	size_t numLine = size_t(String::toInt(numLineStr));
	size_t commentEnd = 0;
	for (size_t i = 0; i < numLine; ++i) {
		commentEnd = code.find('\n', commentEnd) + 1;
	}
	size_t commentStart = code.find_last_of('#', commentEnd) + 1;
	if (!commentStart) return;

	commentEnd = code.find('\n', commentStart);
	std::string comment = Algo::clear(code.substr(commentStart, commentEnd - commentStart));

	std::vector<std::string> parts = String::split(comment, "|");
	if (parts.size() != 3) return;
	if (parts[0] != "_SL_REAL") return;

	fileName = parts[1];
	numLineStr = parts[2];
}
static void fixToRealString(std::string &str, const std::string_view &code,
                            size_t fileNameStart, size_t fileNameEnd,
                            size_t numLineStart, size_t numLineEnd
) {
	std::string fileName = str.substr(fileNameStart, fileNameEnd - fileNameStart);
	std::string numLineStr = str.substr(numLineStart, numLineEnd - numLineStart);
	getRealString(code, fileName, numLineStr);

	str.erase(numLineStart, numLineEnd - numLineStart);
	str.insert(numLineStart, numLineStr);

	str.erase(fileNameStart, fileNameEnd - fileNameStart);
	str.insert(fileNameStart, fileName);
}

static void errorProcessingImpl(const std::string &code) {
	PyObject *pyType, *pyValue, *pyTraceback;
	PyErr_Fetch(&pyType, &pyValue, &pyTraceback);
	if (!pyType) {
		Utils::outMsg("PyUtils::errorProcessing", "pyType == nullptr");
		return;
	}

	PyErr_NormalizeException(&pyType, &pyValue, &pyTraceback);

	std::string typeStr = PyUtils::objToStr(pyType);
	std::string valueStr = PyUtils::objToStr(pyValue);

	std::string traceback;
	if (pyTraceback && pyTraceback != Py_None) {
		PyTuple_SET_ITEM(PyUtils::tuple1, 0, pyTraceback);
		PyObject *res = PyObject_Call(formatTraceback, PyUtils::tuple1, nullptr);
		PyTuple_SET_ITEM(PyUtils::tuple1, 0, nullptr);

		size_t len = size_t(Py_SIZE(res));
		for (size_t i = 0; i < len; ++i) {
			PyObject *item = PyList_GET_ITEM(res, i);
			std::string str = PyUtils::objToStr(item);

			size_t fileNameStart = str.find("_SL_FILE_");
			if (fileNameStart != size_t(-1)) {
				size_t fileNameEnd = str.find('"', fileNameStart);

				size_t numLineStart = str.find("line ") + 5;
				size_t numLineEnd = str.find(',', numLineStart);

				fixToRealString(str, code, fileNameStart, fileNameEnd, numLineStart, numLineEnd);
			}
			traceback += str;
		}

		Py_DECREF(res);
	}else {
		size_t fileNameStart = valueStr.find_last_of('(') + 1;
		size_t fileNameEnd = valueStr.find_last_of(',');

		if (pyType == PyExc_SyntaxError) {
			PyObject *pyFileName = PyObject_GetAttrString(pyValue, "filename");
			if (pyFileName) {
				std::string fileNameWithFullPath = PyUtils::objToStr(pyFileName);
				Py_DECREF(pyFileName);

				valueStr.erase(fileNameStart, fileNameEnd - fileNameStart);
				valueStr.insert(fileNameStart, fileNameWithFullPath);
				fileNameEnd += fileNameWithFullPath.size() - (fileNameEnd - fileNameStart);
			}
		}

		if (String::startsWith(std::string_view(valueStr).substr(fileNameStart), "_SL_FILE_")) {
			size_t numLineStart = valueStr.find_last_of(' ') + 1;
			size_t numLineEnd = valueStr.size() - 1;

			fixToRealString(valueStr, code, fileNameStart, fileNameEnd, numLineStart, numLineEnd);
		}
	}

	const std::string out =
		"Python Error (" + typeStr + "):\n"
		"\t" + valueStr + "\n" +

	    (traceback.empty() ? "" : "Traceback:\n" + traceback + "\n") +

		"Code:\n" +
		code + "\n\n";

	Logger::log(out);

	Utils::outMsg("Python", "See details in var/log.txt");

	Py_XDECREF(pyType);
	Py_XDECREF(pyValue);
	Py_XDECREF(pyTraceback);
}

void PyUtils::errorProcessing(const std::string &code) {
	callInPythonThread([&]() {
		errorProcessingImpl(code);
	});
}



static std::map<std::string, bool> isConstExprCache;
static bool isConstExprImpl(const std::string &code) {
	auto it = isConstExprCache.find(code);
	if (it != isConstExprCache.end()) {
		return it->second;
	}

	{
		size_t start = code.find_first_not_of(' ');
		if (start != size_t(-1)) {
			size_t end = code.find_last_not_of(' ');
			std::string_view s = code;
			s = s.substr(start, end - start + 1);

			if (s == True || s == False || s == None) {
				return isConstExprCache[code] = true;
			}
		}
	}

	bool hex = false;
	bool q1 = false;
	bool q2 = false;
	char prev = 0;
	for (char c : code) {
		if (c == '\'' && !q2 && prev != '\\') q1 = !q1;
		if (c == '"'  && !q1 && prev != '\\') q2 = !q2;

		if (!q1 && !q2) {
			if (c == '_') {
				return isConstExprCache[code] = false;
			}

			if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
				if ((c == 'X' || c == 'x') && prev == '0') {
					hex = true;
				}else {
					if (!hex || ((c > 'F' && c <= 'Z') || (c > 'f' && c <= 'z'))) {
						return isConstExprCache[code] = false;
					}
				}
			}else
			if (c < '0' || c > '9') {
				hex = false;
			}
		}

		if (prev == '\\' && c == '\\') {
			prev = 0;
		}else {
			prev = c;
		}
	}

	return isConstExprCache[code] = true;
}

bool PyUtils::isConstExpr(const std::string &code) {
	bool res;
	callInPythonThread([&]() {
		res = isConstExprImpl(code);
	});
	return res;
}



static std::map<std::string, std::string> constExprs;
static std::string execImpl(const std::string &fileName, uint32_t numLine, const std::string &code, bool retRes) {
	if (code.empty()) return "";

	if (String::isNumber(code)) {
		return code;
	}
	if (String::isSimpleString(code)) {
		return code.substr(1, code.size() - 2);
	}


	bool isConst = retRes && PyUtils::isConstExpr(code);
	if (isConst) {
		auto i = constExprs.find(code);
		if (i != constExprs.end()) {
			return i->second;
		}
	}

	std::string res = "empty";

	PyObject *co;
	if (!retRes) {
		co = getCompileObjectImpl(code, fileName, numLine);
	}else {
		co = getCompileObjectImpl("_res = str(" + code + ")", fileName, numLine);
	}

	if (co) {
		bool ok = PyEval_EvalCode(co, PyUtils::global, nullptr);

		if (ok) {
			if (retRes) {
				PyObject *resObj = PyDict_GetItemString(PyUtils::global, "_res");
				res = PyUtils::objToStr(resObj);

				if (isConst) {
					constExprs[code] = res;
				}
			}
		}else {
			errorProcessingImpl(code);
		}
	}

	return res;
}

std::string PyUtils::exec(const std::string &fileName, uint32_t numLine, const std::string &code, bool retRes) {
	std::string res;
	callInPythonThread([&]() {
		res = execImpl(fileName, numLine, code, retRes);
	});
	return res;
}

std::string PyUtils::execWithSetTmp(const std::string &fileName, uint32_t numLine,
                                    const std::string &code, const std::string &tmp,
                                    bool retRes)
{
	std::string res;
	callInPythonThread([&]() {
		PyObject *pyStr = PyUnicode_FromStringAndSize(tmp.c_str(), Py_ssize_t(tmp.size()));
		PyDict_SetItemString(PyUtils::global, "tmp", pyStr);
		Py_DECREF(pyStr);

		res = execImpl(fileName, numLine, code, retRes);
	});
	return res;
}



static PyObject* execRetObjImpl(const std::string &fileName, uint32_t numLine, const std::string &code) {
	if (code.empty()) return nullptr;

	bool isConst = PyUtils::isConstExpr(code);
	if (isConst) {
		auto i = constObjects.find(code);
		if (i != constObjects.end()) {
			Py_INCREF(i->second);
			return i->second;
		}
	}

	PyObject *res = nullptr;
	PyObject *co = getCompileObjectImpl("_res = " + code, fileName, numLine);
	if (co) {
		bool ok = PyEval_EvalCode(co, PyUtils::global, nullptr);

		if (ok) {
			res = PyDict_GetItemString(PyUtils::global, "_res");
			Py_INCREF(res);

			if (isConst) {
				constObjects[code] = res;
				Py_INCREF(res);
			}
		}else {
			errorProcessingImpl(code);
		}
	}

	return res;
}

PyObject* PyUtils::execRetObj(const std::string &fileName, uint32_t numLine, const std::string &code) {
	PyObject *res;
	callInPythonThread([&]() {
		res = execRetObjImpl(fileName, numLine, code);
	});
	return res;
}


static std::string objToStrImpl(PyObject *obj) {
	Py_ssize_t size;
	if (PyUnicode_CheckExact(obj)) {
		const char *data = PyUnicode_AsUTF8AndSize(obj, &size);
		return std::string(data, size_t(size));
	}

	PyObject *objStr = PyObject_Str(obj);
	const char *data = PyUnicode_AsUTF8AndSize(objStr, &size);
	std::string res(data, size_t(size));
	Py_DECREF(objStr);
	return res;
}

std::string PyUtils::objToStr(PyObject *obj) {
	std::string res;
	callInPythonThread([&]() {
		res = objToStrImpl(obj);
	});
	return res;
}
