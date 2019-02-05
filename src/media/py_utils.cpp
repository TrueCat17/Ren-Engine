#include "py_utils.h"
#include "py_utils/make_func.h"

#include <iostream>


#include "gv.h"
#include "logger.h"
#include "config.h"

#include "gui/screen/screen.h"

#include "image_manipulator.h"
#include "music.h"

#include "parser/parser.h"

#include "utils/game.h"
#include "utils/math.h"
#include "utils/mouse.h"
#include "utils/utils.h"


typedef std::tuple<const String, const String, int> PyCode;

static std::map<String, PyObject*> constObjects;
static std::map<PyCode, PyCodeObject*> compiledObjects;

PyObject* PyUtils::sysExcInfo = nullptr;
PyObject* PyUtils::formatTraceback = nullptr;


PyUtils* PyUtils::obj = nullptr;

const String PyUtils::True = "True";
const String PyUtils::False = "False";
const String PyUtils::None = "None";

PyObject *PyUtils::global = nullptr;
PyObject *PyUtils::tuple1 = nullptr;
std::recursive_mutex PyUtils::pyExecMutex;


PyCodeObject* PyUtils::getCompileObject(const String &code, const String &fileName, size_t numLine) {
	std::tuple<const String&, const String&, int> pyCode(code, fileName, numLine);

	auto i = compiledObjects.find(pyCode);
	if (i != compiledObjects.end()) {
		return i->second;
	}


	String indent;
	if (numLine > 1) {
		indent.resize(numLine - 1, '\n');
	}

	const String tmp = indent + code;

	PyObject *t;
	{
		std::lock_guard g(PyUtils::pyExecMutex);
		t = Py_CompileString(tmp.c_str(), fileName.c_str(), Py_file_input);
	}
	PyCodeObject *co = reinterpret_cast<PyCodeObject*>(t);

	std::tuple<const String, const String, int> constPyCode(code, fileName, numLine);
	compiledObjects[constPyCode] = co;
	return co;
}


static PyObject* getMouse() {
	PyObject *res = PyTuple_New(2);
	PyTuple_SET_ITEM(res, 0, PyInt_FromLong(Mouse::getX()));
	PyTuple_SET_ITEM(res, 1, PyInt_FromLong(Mouse::getY()));
	return res;
}
static PyObject* getLocalMouse() {
	PyObject *res = PyTuple_New(2);
	PyTuple_SET_ITEM(res, 0, PyInt_FromLong(Mouse::getLocalX()));
	PyTuple_SET_ITEM(res, 1, PyInt_FromLong(Mouse::getLocalY()));
	return res;
}

template<typename T>
static void setGlobalValue(const char *key, T t) {
	PyObject *res;
	if constexpr (std::is_same<T, bool>::value) {
		res = t ? Py_True : Py_False;
	}else
	if constexpr (std::is_same<T, int>::value) {
		res = PyInt_FromLong(t);
	}else
	if constexpr (std::is_same<T, double>::value) {
		res = PyFloat_FromDouble(t);
	}else {
		Utils::outMsg("PyUtils::setGlobalValue", "Expected types bool, int or double");
		res = Py_None;
		Py_INCREF(res);
	}

	PyDict_SetItemString(PyUtils::global, key, res);

	if constexpr (!std::is_same<T, bool>::value) {
		Py_DECREF(res);
	}
}

template<typename T>
static void setGlobalFunc(const char *key, T t) {
	PyObject *pyFunc = makeFuncImpl(key, t);

	PyDict_SetItemString(PyUtils::global, key, pyFunc);
	Py_DECREF(pyFunc);
}

static long ftoi(double d) {
	return long(d);
}

PyUtils::PyUtils() {
	Py_Initialize();

	tuple1 = PyTuple_New(1);

	PyObject *sysModule = PyImport_AddModule("sys");
	PyObject *sysDict = PyModule_GetDict(sysModule);
	sysExcInfo = PyDict_GetItemString(sysDict, "exc_info");
	assert(sysExcInfo);

	PyObject *tracebackModule = PyImport_AddModule("traceback");
	PyObject *tracebackDict = PyModule_GetDict(tracebackModule);
	formatTraceback = PyDict_GetItemString(tracebackDict, "format_tb");
	assert(formatTraceback);


	PyObject *main = PyImport_AddModule("__main__");
	global = PyModule_GetDict(main);


	setGlobalValue("need_save", false);
	setGlobalValue("save_screenshotting", false);
	setGlobalValue("need_screenshot", false);
	setGlobalValue("screenshot_width", 640);
	setGlobalValue("screenshot_height", 640 / Config::get("window_w_div_h").toDouble());

	setGlobalFunc("ftoi", ftoi);

	setGlobalFunc("get_mods", Parser::getMods);
	setGlobalFunc("_out_msg", Utils::outMsg);

	setGlobalFunc("_register_channel", Music::registerChannel);
	setGlobalFunc("_has_channel", Music::hasChannel);
	setGlobalFunc("_set_volume", Music::setVolume);
	setGlobalFunc("_set_mixer_volume", Music::setMixerVolume);
	setGlobalFunc("_play", Music::play);
	setGlobalFunc("_stop", Music::stop);

	setGlobalFunc("image_was_registered", Utils::imageWasRegistered);
	setGlobalFunc("get_image_code", Utils::getImageCode);
	setGlobalFunc("get_image_decl_at", Utils::getImageDeclAt);

	setGlobalFunc("show_screen", Screen::addToShow);
	setGlobalFunc("hide_screen", Screen::addToHide);
	setGlobalFunc("has_screen", Screen::hasScreen);
	setGlobalFunc("_SL_check_events", Screen::checkScreenEvents);
	setGlobalFunc("_SL_error_processing", Screen::errorProcessing);

	setGlobalFunc("start_mod", Game::startMod);
	setGlobalFunc("get_mod_start_time", Game::getModStartTime);
	setGlobalFunc("get_can_autosave", Game::getCanAutoSave);
	setGlobalFunc("set_can_autosave", Game::setCanAutoSave);
	setGlobalFunc("_load", Game::load);
	setGlobalFunc("exit_from_game", Game::exitFromGame);

	setGlobalFunc("_has_label", Game::hasLabel);
	setGlobalFunc("_jump_next", Node::jumpNext);

	setGlobalFunc("_get_from_hard_config", Game::getFromConfig);
	setGlobalFunc("get_args", Game::getArgs);

	setGlobalFunc("_sin", Math::getSin);
	setGlobalFunc("_cos", Math::getCos);

	setGlobalFunc("get_fps", Game::getFps);
	setGlobalFunc("set_fps", Game::setFps);

	setGlobalFunc("get_stage_width", Game::getStageWidth);
	setGlobalFunc("get_stage_height", Game::getStageHeight);

	setGlobalFunc("set_stage_size", Game::setStageSize);
	setGlobalFunc("set_fullscreen", Game::setFullscreen);

	setGlobalFunc("save_image", ImageManipulator::save);
	setGlobalFunc("load_image", ImageManipulator::loadImage);
	setGlobalFunc("get_texture_width", Game::getTextureWidth);
	setGlobalFunc("get_texture_height", Game::getTextureHeight);
	setGlobalFunc("get_pixel", Game::getPixel);

	setGlobalFunc("get_mouse", getMouse);
	setGlobalFunc("get_local_mouse", getLocalMouse);
	setGlobalFunc("get_mouse_down", Mouse::getMouseDown);
	setGlobalFunc("set_can_mouse_hide", Mouse::setCanHide);
}
PyUtils::~PyUtils() {
	constObjects.clear();

	sysExcInfo = nullptr;
	formatTraceback = nullptr;

	global = nullptr;
	PyUtils::obj = nullptr;

	Py_DECREF(tuple1);
	tuple1 = nullptr;

	clearPyWrappers();
	Py_Finalize();
}


static void getRealString(size_t numLine, const String &code, String &fileName, String &numLineStr) {
	size_t commentEnd = 0;
	for (size_t i = 0; i < numLine; ++i) {
		commentEnd = code.find('\n', commentEnd) + 1;
	}
	size_t commentStart = code.find_last_of('#', commentEnd) + 1;
	if (!commentStart) return;

	commentEnd = code.find('\n', commentStart);
	String comment = code.substr(commentStart, commentEnd - commentStart);

	size_t numLineStart = comment.find_first_not_of(' ');
	size_t numLineEnd = comment.find_first_of(' ', numLineStart);
	numLineStr = comment.substr(numLineStart, numLineEnd - numLineStart);

	size_t fileNameStart = comment.find_first_not_of(' ', numLineEnd);
	size_t fileNameEnd = comment.size();
	fileName = comment.substr(fileNameStart, fileNameEnd - fileNameStart);
}

void PyUtils::errorProcessing(const String &code) {
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	if (!ptype) {
		Utils::outMsg("PyUtils::errorProcessing", "ptype == nullptr");
		return;
	}

	PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);

	if (ptype == PyExc_StopIteration) {
		Py_DECREF(ptype);
		Py_DECREF(pvalue);
		Py_DECREF(ptraceback);
		throw StopException();
	}

	PyObject *typeStrObj = PyObject_Str(ptype);
	String typeStr = PyString_AS_STRING(typeStrObj);

	PyObject *valueStrObj = PyObject_Str(pvalue);
	String valueStr = PyString_AS_STRING(valueStrObj);

	std::string traceback;
	if (ptraceback && ptraceback != Py_None) {
		PyObject *args = PyTuple_New(1);
		PyTuple_SET_ITEM(args, 0, ptraceback);
		PyObject *res = PyObject_Call(formatTraceback, args, nullptr);
		Py_DECREF(args);

		size_t len = size_t(Py_SIZE(res));
		for (size_t i = 0; i < len; ++i) {
			PyObject *item = PyList_GET_ITEM(res, i);
			String str = PyString_AS_STRING(item);

			size_t fileNameStart = str.find("_SL_FILE_");
			if (fileNameStart != size_t(-1)) {
				size_t fileNameEnd = str.find('"', fileNameStart);
				String fileName = str.substr(fileNameStart, fileNameEnd - fileNameStart);

				size_t numLineStart = str.find("line ") + 5;
				size_t numLineEnd = str.find(',', numLineStart);
				String numLineStr = str.substr(numLineStart, numLineEnd - numLineStart);
				size_t numLine = size_t(numLineStr.toInt());

				getRealString(numLine, code, fileName, numLineStr);

				str.erase(numLineStart, numLineEnd - numLineStart);
				str.insert(numLineStart, numLineStr);

				str.erase(fileNameStart, fileNameEnd - fileNameStart);
				str.insert(fileNameStart, fileName);
			}
			traceback += str;
		}

		Py_DECREF(res);
	}else {
		size_t fileNameStart = valueStr.find('(') + 1;
		size_t fileNameEnd = valueStr.find_last_of(',');
		String fileName = valueStr.substr(fileNameStart, fileNameEnd - fileNameStart);

		if (fileName.find("_SL_FILE_") != size_t(-1)) {
			size_t numLineStart = valueStr.find_last_of(' ') + 1;
			size_t numLineEnd = valueStr.size() - 1;
			String numLineStr = valueStr.substr(numLineStart, numLineEnd - numLineStart);
			size_t numLine = size_t(numLineStr.toInt());

			getRealString(numLine, code, fileName, numLineStr);

			valueStr.erase(numLineStart, numLineEnd - numLineStart);
			valueStr.insert(numLineStart, numLineStr);

			valueStr.erase(fileNameStart, fileNameEnd - fileNameStart);
			valueStr.insert(fileNameStart, fileName);
		}
	}

	const String out = String() +
		"Python Error (" + typeStr + "):\n"
		"\t" + valueStr + "\n" +

		String(!traceback.empty() ? "Traceback:\n" + traceback + "\n" : "") +

		"Code:\n" +
		code + "\n\n";

	std::cout << out;
	Logger::log(out);

	Utils::outMsg("Python", "See details in resources/log.txt");
}

bool PyUtils::isConstExpr(const String &code, bool checkSimple) {
	if (checkSimple) {
		if (code == True || code == False || code == None) {
			return true;
		}
	}


	bool q1 = false;
	bool q2 = false;
	char prev = 0;
	for (char c : code) {
		if (c == '\'' && !q2 && prev != '\\') q1 = !q1;
		if (c == '"'  && !q1 && prev != '\\') q2 = !q2;

		if (!q1 && !q2) {
			if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
				if (!((c == 'X' || c == 'x') && prev == '0')) {
					return false;
				}
			}
		}

		if (prev == '\\' && c == '\\') {
			prev = 0;
		}else {
			prev = c;
		}
	}
	return true;
}

String PyUtils::exec(const String &fileName, size_t numLine, const String &code, bool retRes) {
	if (!code) return "";

	if (code.isNumber()) {
		return code;
	}
	if (code.isSimpleString()) {
		return code.substr(1, code.size() - 2);
	}
	if (code == True || code == False || code == None) {
		return code;
	}


	static std::map<String, String> constExprs;
	bool isConst = retRes && isConstExpr(code, false);
	if (isConst) {
		auto i = constExprs.find(code);
		if (i != constExprs.end()) {
			return i->second;
		}
	}

	String res = "empty";

	std::lock_guard g(pyExecMutex);

	PyCodeObject *co;
	if (!retRes) {
		co = getCompileObject(code, fileName, numLine);
	}else {
		co = getCompileObject("res = str(" + code + ")", fileName, numLine);
	}

	if (co) {
		bool ok = PyEval_EvalCode(co, global, nullptr);

		if (ok) {
			if (retRes) {
				PyObject *resObj = PyDict_GetItemString(global, "res");
				res = PyString_AS_STRING(resObj);

				if (isConst) {
					constExprs[code] = res;
				}
			}
		}else {
			errorProcessing(code);
		}
	}else {
		std::cout << "Python Compile Error:\n";
		errorProcessing(code);
	}

	return res;
}

PyObject* PyUtils::execRetObj(const String &fileName, size_t numLine, const String &code) {
	PyObject *res = nullptr;
	if (!code) return res;

	bool isConst = isConstExpr(code);
	if (isConst) {
		auto i = constObjects.find(code);
		if (i != constObjects.end()) {
			return i->second;
		}
	}

	std::lock_guard g(pyExecMutex);

	PyCodeObject *co = getCompileObject("res = " + code, fileName, numLine);
	if (co) {
		bool ok = PyEval_EvalCode(co, global, nullptr);

		if (ok) {
			res = PyDict_GetItemString(global, "res");
			Py_INCREF(res);

			if (isConst) {
				constObjects[code] = res;
			}
		}else {
			errorProcessing(code);
		}
	}else {
		std::cout << "Python Compile Error:\n";
		errorProcessing(code);
	}

	return res;
}
