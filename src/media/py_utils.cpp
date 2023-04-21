#include "py_utils.h"
#include "py_utils/make_func.h"
#include "py_utils/absolute.h"


#include "gv.h"
#include "logger.h"
#include "config.h"

#include "gui/gui.h"
#include "gui/screen/screen.h"

#include "image_manipulator.h"
#include "music.h"
#include "scenario.h"
#include "translation.h"

#include "parser/mods.h"

#include "utils/game.h"
#include "utils/math.h"
#include "utils/mouse.h"
#include "utils/stage.h"
#include "utils/string.h"
#include "utils/path_finder.h"
#include "utils/utils.h"


typedef std::tuple<const std::string, const std::string, uint32_t> PyCode;

static std::map<std::string, PyObject*> constObjects;
static std::map<PyCode, PyObject*> compiledObjects;

static const std::string True = "True";
static const std::string False = "False";
static const std::string None = "None";


static PyObject* formatTraceback = nullptr;
static PyObject* builtinDict = nullptr;
static PyObject* builtinStr = nullptr;


PyObject *PyUtils::global = nullptr;
PyObject *PyUtils::tuple1 = nullptr;
std::recursive_mutex PyUtils::pyExecMutex;


PyObject* PyUtils::getCompileObject(const std::string &code, const std::string &fileName, size_t numLine) {
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

	PyObject *res = Py_CompileStringFlags(indentCode.c_str(), fileName.c_str(), Py_file_input, nullptr);
	if (!res) {
		PyUtils::errorProcessing(code);
	}

	return compiledObjects[refPyCode] = res;
}


static PyObject* getMouse() {
	PyObject *res = PyTuple_New(2);
	PyTuple_SET_ITEM(res, 0, PyLong_FromLong(Mouse::getX() - Stage::x));
	PyTuple_SET_ITEM(res, 1, PyLong_FromLong(Mouse::getY() - Stage::y));
	return res;
}
static PyObject* getLocalMouse() {
	PyObject *res = PyTuple_New(2);
	PyTuple_SET_ITEM(res, 0, PyLong_FromLong(Mouse::getLocalX()));
	PyTuple_SET_ITEM(res, 1, PyLong_FromLong(Mouse::getLocalY()));
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
	PyObject *pyFunc = makeFuncImpl(key, t, builtinStr);

	PyDict_SetItemString(builtinDict, key, pyFunc);
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
	Py_XDECREF(builtinStr);
	Mods::clearList();
	Py_Finalize();


	Py_Initialize();

	tuple1 = PyTuple_New(1);
	builtinStr = PyUnicode_FromString("builtins");

	PyObject *tracebackModule = PyImport_ImportModule("traceback");
	if (!tracebackModule) {
		PyErr_Print();
		std::abort();
	}
	formatTraceback = PyObject_GetAttrString(tracebackModule, "format_tb");
	if (!formatTraceback) {
		Utils::outMsg("PyUtils::init", "traceback.format_tb == nullptr");
		std::abort();
	}

	PyObject *builtinModule = PyImport_AddModule("builtins");
	builtinDict = PyModule_GetDict(builtinModule);
	if (!builtinDict) {
		Utils::outMsg("PyUtils::init", "builtins == nullptr");
		std::abort();
	}

	PyObject *main = PyImport_AddModule("__main__");
	global = PyModule_GetDict(main);


	if (PyType_Ready(&PyAbsolute_Type) < 0) {
		Utils::outMsg("PyUtils::init", "Can't initialize absolute type");
	}else {
		PyDict_SetItemString(builtinDict, "absolute", (PyObject*)&PyAbsolute_Type);
	}
	setGlobalFunc("get_engine_version", Utils::getVersion);

	setGlobalValue("need_save", false);
	setGlobalValue("need_screenshot", false);
	setGlobalValue("screenshot_width", 640);
	setGlobalValue("screenshot_height", 640 / String::toDouble(Config::get("window_w_div_h")));

	setGlobalFunc("ftoi", ftoi);
	setGlobalFunc("get_md5", Utils::md5);

	setGlobalFunc("get_current_mod", getCurrentMod);
	setGlobalFunc("get_mods", Mods::getList);
	setGlobalFunc("_out_msg", Utils::outMsg);

	setGlobalFunc("_register_channel", Music::registerChannel);
	setGlobalFunc("_has_channel", Music::hasChannel);
	setGlobalFunc("_get_audio_len", Music::getAudioLen);
	setGlobalFunc("_set_mixer_volume", Music::setMixerVolume);
	setGlobalFunc("_set_volume_on_channel", Music::setVolumeOnChannel);
	setGlobalFunc("_get_pos_on_channel", Music::getPosOnChannel);
	setGlobalFunc("_set_pos_on_channel", Music::setPosOnChannel);
	setGlobalFunc("_get_pause_on_channel", Music::getPauseOnChannel);
	setGlobalFunc("_set_pause_on_channel", Music::setPauseOnChannel);
	setGlobalFunc("_play", Music::play);
	setGlobalFunc("_stop", Music::stop);

	setGlobalFunc("image_was_registered", Utils::imageWasRegistered);
	setGlobalFunc("get_image", Utils::getImageDeclAt);

	setGlobalFunc("_show_screen", Screen::addToShow);
	setGlobalFunc("hide_screen", Screen::addToHide);
	setGlobalFunc("has_screen", Screen::hasScreen);
	setGlobalFunc("_log_screen_code", Screen::logScreenCode);
	setGlobalFunc("_SL_check_events", Screen::checkScreenEvents);

	setGlobalFunc("start_mod", Game::startMod);
	setGlobalFunc("get_mod_start_time", Game::getModStartTime);
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

	setGlobalFunc("get_clipboard_text", Utils::getClipboardText);
	setGlobalFunc("set_clipboard_text", Utils::setClipboardText);

	setGlobalFunc("get_screen_times", GUI::getScreenTimes);

	setGlobalFunc("set_scale_quality", Config::setScaleQuality);
	setGlobalFunc("get_scale_quality", Config::getScaleQuality);
}


static void getRealString(const std::string &code, std::string &fileName, std::string &numLineStr) {
	size_t numLine = size_t(String::toInt(numLineStr));
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
static void fixToRealString(std::string &str, const std::string &code,
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

void PyUtils::errorProcessing(const std::string &code) {
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
		PyTuple_SET_ITEM(tuple1, 0, pyTraceback);
		PyObject *res = PyObject_Call(formatTraceback, tuple1, nullptr);
		PyTuple_SET_ITEM(tuple1, 0, nullptr);

		size_t len = size_t(Py_SIZE(res));
		for (size_t i = 0; i < len; ++i) {
			PyObject *item = PyList_GET_ITEM(res, i);
			std::string str = PyUnicode_AsUTF8(item);

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

		if (valueStr.find("_SL_FILE_", fileNameStart, 1) != size_t(-1)) {
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
					if (!hex || ((c > 'F' && c <= 'Z') || (c > 'f' && c <= 'z'))) return false;
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

	PyObject *co;
	if (!retRes) {
		co = getCompileObject(code, fileName, numLine);
	}else {
		co = getCompileObject("_res = str(" + code + ")", fileName, numLine);
	}

	if (co) {
		bool ok = PyEval_EvalCode(co, global, nullptr);

		if (ok) {
			if (retRes) {
				PyObject *resObj = PyDict_GetItemString(global, "_res");
				res = PyUnicode_AsUTF8(resObj);

				if (isConst) {
					constExprs[code] = res;
				}
			}
		}else {
			errorProcessing(code);
		}
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

	PyObject *co = getCompileObject("res = " + code, fileName, numLine);
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
	}

	return res;
}


std::string PyUtils::objToStr(PyObject *obj) {
	PyObject *objStr = PyObject_Str(obj);
	std::string res = PyUnicode_AsUTF8(objStr);
	if (res.size() > 50) {
		res.erase(48);
		res += "..";
	}
	Py_DECREF(objStr);
	return res;
}
