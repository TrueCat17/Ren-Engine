#include "py_utils.h"
#include "py_utils/make_func.h"
#include "py_utils/absolute.h"


#include "gv.h"
#include "logger.h"
#include "config.h"

#include "gui/screen/screen.h"

#include "image_manipulator.h"
#include "music.h"
#include "scenario.h"
#include "translation.h"

#include "parser/mods.h"
#include "parser/parser.h"

#include "utils/game.h"
#include "utils/math.h"
#include "utils/mouse.h"
#include "utils/stage.h"
#include "utils/string.h"
#include "utils/path_finder.h"
#include "utils/utils.h"


typedef std::tuple<const std::string, const std::string, uint32_t> PyCode;

static std::map<std::string, PyObject*> constObjects;
static std::map<PyCode, PyCodeObject*> compiledObjects;

static const std::string True = "True";
static const std::string False = "False";
static const std::string None = "None";


PyObject* PyUtils::formatTraceback = nullptr;


PyObject *PyUtils::global = nullptr;
PyObject *PyUtils::tuple1 = nullptr;
std::recursive_mutex PyUtils::pyExecMutex;


PyCodeObject* PyUtils::getCompileObject(const std::string &code, const std::string &fileName, size_t numLine) {
	std::tuple<const std::string&, const std::string&, size_t> refPyCode(code, fileName, numLine);

	std::lock_guard g(pyExecMutex);

	auto i = compiledObjects.find(refPyCode);
	if (i != compiledObjects.end()) {
		return i->second;
	}


	std::string indentCode;
	indentCode.reserve(numLine - 1 + code.size());
	indentCode.insert(0, numLine - 1, '\n');
	indentCode += code;

	PyObject *t = Py_CompileStringFlags(indentCode.c_str(), fileName.c_str(), Py_file_input, nullptr);
	PyCodeObject *co = reinterpret_cast<PyCodeObject*>(t);

	compiledObjects[refPyCode] = co;
	return co;
}


static PyObject* getMouse() {
	PyObject *res = PyTuple_New(2);
	PyTuple_SET_ITEM(res, 0, PyInt_FromLong(Mouse::getX() - Stage::x));
	PyTuple_SET_ITEM(res, 1, PyInt_FromLong(Mouse::getY() - Stage::y));
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
	PyObject *res = convertToPy(t);
	PyDict_SetItemString(PyUtils::global, key, res);
	Py_DECREF(res);
}

template<typename T>
static void setGlobalFunc(const char *key, T t) {
	PyObject *pyFunc = makeFuncImpl(key, t);

	PyDict_SetItemString(PyUtils::global, key, pyFunc);
	--pyFunc->ob_refcnt;
}

static long ftoi(double d) {
	return long(d);
}
static std::string getCurrentMod() {
	return GV::mainExecNode->params;
}

void PyUtils::init() {
	std::lock_guard g(pyExecMutex);

	//clear
	constObjects.clear();
	clearPyWrappers();
	if (tuple1) {
		PyTuple_SET_ITEM(tuple1, 0, nullptr);
		Py_DECREF(PyUtils::tuple1);
	}
	Mods::clearList();
	Py_Finalize();


	Py_Initialize();

	tuple1 = PyTuple_New(1);

	PyObject *tracebackModule = PyImport_AddModule("traceback");
	PyObject *tracebackDict = PyModule_GetDict(tracebackModule);
	formatTraceback = PyDict_GetItemString(tracebackDict, "format_tb");
	if (!formatTraceback) throw std::runtime_error("traceback.format_tb == nullptr");


	PyObject *main = PyImport_AddModule("__main__");
	global = PyModule_GetDict(main);


	if (PyType_Ready(&PyAbsolute_Type) < 0) {
		Utils::outMsg("PyUtils::init", "Can't initialize absolute type");
	}else {
		PyObject *builtinModule = PyImport_AddModule("__builtin__");
		PyObject *builtinDict = PyModule_GetDict(builtinModule);
		PyDict_SetItemString(builtinDict, "absolute", (PyObject*)&PyAbsolute_Type);
	}

	setGlobalValue("need_save", false);
	setGlobalValue("save_screenshotting", false);
	setGlobalValue("need_screenshot", false);
	setGlobalValue("screenshot_width", 640);
	setGlobalValue("screenshot_height", 640 / String::toDouble(Config::get("window_w_div_h")));

	setGlobalFunc("ftoi", ftoi);

	setGlobalFunc("get_current_mod", getCurrentMod);
	setGlobalFunc("get_mods", Mods::getList);
	setGlobalFunc("_out_msg", Utils::outMsg);

	setGlobalFunc("_register_channel", Music::registerChannel);
	setGlobalFunc("_has_channel", Music::hasChannel);
	setGlobalFunc("_set_volume", Music::setVolume);
	setGlobalFunc("_set_mixer_volume", Music::setMixerVolume);
	setGlobalFunc("_play", Music::play);
	setGlobalFunc("_stop", Music::stop);

	setGlobalFunc("image_was_registered", Utils::imageWasRegistered);
	setGlobalFunc("get_image", Utils::getImageDeclAt);

	setGlobalFunc("show_screen", Screen::addToShow);
	setGlobalFunc("hide_screen", Screen::addToHide);
	setGlobalFunc("has_screen", Screen::hasScreen);
	setGlobalFunc("_log_screen_code", Screen::logScreenCode);
	setGlobalFunc("_SL_check_events", Screen::checkScreenEvents);

	setGlobalFunc("start_mod", Game::startMod);
	setGlobalFunc("get_mod_start_time", Game::getModStartTime);
	setGlobalFunc("get_can_autosave", Game::getCanAutoSave);
	setGlobalFunc("set_can_autosave", Game::setCanAutoSave);
	setGlobalFunc("_load", Game::load);
	setGlobalFunc("exit_from_game", Game::exitFromGame);

	setGlobalFunc("_has_label", Game::hasLabel);
	setGlobalFunc("_get_all_labels", Game::getAllLabels);
	setGlobalFunc("_jump_next", Scenario::jumpNext);

	setGlobalFunc("_get_from_hard_config", Game::getFromConfig);
	setGlobalFunc("get_args", Game::getArgs);

	setGlobalFunc("_sin", Math::getSin);
	setGlobalFunc("_cos", Math::getCos);

	setGlobalFunc("get_fps", Game::getFps);
	setGlobalFunc("set_fps", Game::setFps);

	setGlobalFunc("get_stage_width", Stage::getStageWidth);
	setGlobalFunc("get_stage_height", Stage::getStageHeight);

	setGlobalFunc("set_stage_size", Stage::setStageSize);
	setGlobalFunc("set_fullscreen", Stage::setFullscreen);

	setGlobalFunc("save_image", ImageManipulator::save);
	setGlobalFunc("load_image", ImageManipulator::loadImage);
	setGlobalFunc("get_image_width", Game::getImageWidth);
	setGlobalFunc("get_image_height", Game::getImageHeight);
	setGlobalFunc("get_image_pixel", Game::getImagePixel);

	setGlobalFunc("get_mouse", getMouse);
	setGlobalFunc("get_local_mouse", getLocalMouse);
	setGlobalFunc("get_mouse_down", Mouse::getMouseDown);
	setGlobalFunc("set_can_mouse_hide", Mouse::setCanHide);

	setGlobalFunc("path_update_location", PathFinder::updateLocation);
	setGlobalFunc("path_on_location", PathFinder::findPath);
	setGlobalFunc("path_between_locations", PathFinder::findPathBetweenLocations);

	setGlobalFunc("_set_lang", Translation::setLang);
	setGlobalFunc("_known_languages", Translation::getKnownLanguages);
	setGlobalFunc("_", Translation::get);

	setGlobalFunc("get_last_tick", Game::getLastTick);
	setGlobalFunc("get_game_time", Game::getGameTime);
}


static void getRealString(size_t numLine, const std::string &code, std::string &fileName, std::string &numLineStr) {
	size_t commentEnd = 0;
	for (size_t i = 0; i < numLine; ++i) {
		commentEnd = code.find('\n', commentEnd) + 1;
	}
	size_t commentStart = code.find_last_of('#', commentEnd) + 1;
	if (!commentStart) return;

	commentEnd = code.find('\n', commentStart);
	std::string comment = code.substr(commentStart, commentEnd - commentStart);

	size_t numLineStart = comment.find_first_not_of(' ');
	size_t numLineEnd = comment.find_first_of(' ', numLineStart);
	numLineStr = comment.substr(numLineStart, numLineEnd - numLineStart);

	size_t fileNameStart = comment.find_first_not_of(' ', numLineEnd);
	size_t fileNameEnd = comment.size();
	fileName = comment.substr(fileNameStart, fileNameEnd - fileNameStart);
}

void PyUtils::errorProcessing(const std::string &code) {
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	if (!ptype) {
		Utils::outMsg("PyUtils::errorProcessing", "ptype == nullptr");
		return;
	}

	PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);

	PyObject *typeStrObj = PyObject_Str(ptype);
	std::string typeStr = PyString_AS_STRING(typeStrObj);

	PyObject *valueStrObj = PyObject_Str(pvalue);
	std::string valueStr = PyString_AS_STRING(valueStrObj);

	std::string traceback;
	if (ptraceback && ptraceback != Py_None) {
		PyTuple_SET_ITEM(tuple1, 0, ptraceback);
		PyObject *res = PyObject_Call(formatTraceback, tuple1, nullptr);

		size_t len = size_t(Py_SIZE(res));
		for (size_t i = 0; i < len; ++i) {
			PyObject *item = PyList_GET_ITEM(res, i);
			std::string str = PyString_AS_STRING(item);

			size_t fileNameStart = str.find("_SL_FILE_");
			if (fileNameStart != size_t(-1)) {
				size_t fileNameEnd = str.find('"', fileNameStart);
				std::string fileName = str.substr(fileNameStart, fileNameEnd - fileNameStart);

				size_t numLineStart = str.find("line ") + 5;
				size_t numLineEnd = str.find(',', numLineStart);
				std::string numLineStr = str.substr(numLineStart, numLineEnd - numLineStart);
				size_t numLine = size_t(String::toInt(numLineStr));

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
		std::string fileName = valueStr.substr(fileNameStart, fileNameEnd - fileNameStart);

		if (fileName.find("_SL_FILE_") != size_t(-1)) {
			size_t numLineStart = valueStr.find_last_of(' ') + 1;
			size_t numLineEnd = valueStr.size() - 1;
			std::string numLineStr = valueStr.substr(numLineStart, numLineEnd - numLineStart);
			size_t numLine = size_t(String::toInt(numLineStr));

			getRealString(numLine, code, fileName, numLineStr);

			valueStr.erase(numLineStart, numLineEnd - numLineStart);
			valueStr.insert(numLineStart, numLineStr);

			valueStr.erase(fileNameStart, fileNameEnd - fileNameStart);
			valueStr.insert(fileNameStart, fileName);
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
}

bool PyUtils::isConstExpr(const std::string &code, bool checkSimple) {
	if (checkSimple) {
		size_t start = code.find_first_not_of(' ');
		if (start != size_t(-1)) {
			size_t end = code.find_last_not_of(' ');
			std::string_view s = code;
			s = s.substr(start, end - start + 1);

			if (s == True || s == False || s == None) {
				return true;
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
			if (c == '_') return false;

			if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
				if ((c == 'X' || c == 'x') && prev == '0') {
					hex = true;
				}else {
					if (!hex && ((c > 'F' && c <= 'Z') || (c > 'f' && c <= 'z'))) return false;
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
	return true;
}

std::string PyUtils::exec(const std::string &fileName, size_t numLine, const std::string &code, bool retRes) {
	if (code.empty()) return "";

	if (String::isNumber(code)) {
		return code;
	}
	if (String::isSimpleString(code)) {
		return code.substr(1, code.size() - 2);
	}
	if (code == True || code == False || code == None) {
		return code;
	}


	static std::map<std::string, std::string> constExprs;
	bool isConst = retRes && isConstExpr(code, false);
	if (isConst) {
		auto i = constExprs.find(code);
		if (i != constExprs.end()) {
			return i->second;
		}
	}

	std::string res = "empty";

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
		errorProcessing(code);
	}

	return res;
}

PyObject* PyUtils::execRetObj(const std::string &fileName, size_t numLine, const std::string &code) {
	PyObject *res = nullptr;
	if (code.empty()) return res;

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
		errorProcessing(code);
	}

	return res;
}


std::string PyUtils::getMd5(const std::string &str) {
	std::lock_guard g(pyExecMutex);

	PyObject *module = PyImport_AddModule("_md5");
	PyObject *constructor = PyObject_GetAttrString(module, "new");

	PyObject *noArgs = PyTuple_New(0);

	PyObject *md5 = PyObject_Call(constructor, noArgs, nullptr);
	PyObject *update = PyObject_GetAttrString(md5, "update");
	PyObject *hexdigest = PyObject_GetAttrString(md5, "hexdigest");

	PyObject *pyStr = PyString_FromStringAndSize(str.c_str(), long(str.size()));
	PyTuple_SET_ITEM(tuple1, 0, pyStr);
	PyObject_Call(update, tuple1, nullptr);
	PyObject *pyRes = PyObject_Call(hexdigest, noArgs, nullptr);

	std::string res = PyString_AS_STRING(pyRes);

	Py_DECREF(md5);
	Py_DECREF(pyStr);
	Py_DECREF(pyRes);
	Py_DECREF(noArgs);

	return res;
}
