#include "py_utils.h"

#include <iostream>

#include <boost/python.hpp>
#include <Python.h>


#include "gv.h"
#include "logger.h"

#include "gui/screen/screen.h"

#include "image.h"
#include "music.h"

#include "parser/parser.h"

#include "utils/game.h"
#include "utils/mouse.h"


std::map<PyCode, PyCodeObject*> PyUtils::compiledObjects;

const String PyUtils::True = "True";
const String PyUtils::False = "False";
const String PyUtils::None = "None";

std::mutex PyUtils::pyExecMutex;


PyCodeObject* PyUtils::getCompileObject(const String &code, const String &fileName, size_t numLine, bool lock) {
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

	if (lock) GV::pyUtils->pyExecMutex.lock();
	PyObject *t = Py_CompileString(tmp.c_str(), fileName.c_str(), Py_file_input);
	if (lock) GV::pyUtils->pyExecMutex.unlock();

	PyCodeObject *co = reinterpret_cast<PyCodeObject*>(t);

	std::tuple<const String, const String, int> constPyCode(code, fileName, numLine);
	compiledObjects[constPyCode] = co;
	return co;
}


PyUtils::PyUtils() {
	Py_Initialize();

	mainModule = py::import("__main__");
	pythonGlobal = mainModule.attr("__dict__");


	pythonGlobal["need_save"] = false;
	pythonGlobal["screenshotting"] = false;

	pythonGlobal["get_mods"] = py::make_function(Parser::getMods);
	pythonGlobal["_out_msg"] = py::make_function(Utils::outMsg);

	pythonGlobal["_register_channel"] = py::make_function(Music::registerChannel);
	pythonGlobal["_has_channel"] = py::make_function(Music::hasChannel);
	pythonGlobal["_set_volume"] = py::make_function(Music::setVolume);
	pythonGlobal["_set_mixer_volume"] = py::make_function(Music::setMixerVolume);
	pythonGlobal["_play"] = py::make_function(Music::play);
	pythonGlobal["_stop"] = py::make_function(Music::stop);

	pythonGlobal["image_was_registered"] = py::make_function(Utils::imageWasRegistered);
	pythonGlobal["get_image_code"] = py::make_function(Utils::getImageCode);
	pythonGlobal["get_image_decl_at"] = py::make_function(Utils::getImageDeclAt);

	pythonGlobal["show_screen"] = py::make_function(Screen::addToShow);
	pythonGlobal["hide_screen"] = py::make_function(Screen::addToHide);
	pythonGlobal["has_screen"] = py::make_function(Screen::hasScreen);

	pythonGlobal["start_mod"] = py::make_function(Game::startMod);
	pythonGlobal["_load"] = py::make_function(Game::load);
	pythonGlobal["exit_from_game"] = py::make_function(Game::exitFromGame);

	pythonGlobal["_has_label"] = py::make_function(Game::hasLabel);

	pythonGlobal["_get_from_hard_config"] = py::make_function(Game::getFromConfig);
	pythonGlobal["get_args"] = py::make_function(Game::getArgs);

	pythonGlobal["_sin"] = py::make_function(Utils::getSin);
	pythonGlobal["_cos"] = py::make_function(Utils::getCos);

	pythonGlobal["get_fps"] = py::make_function(Game::getFps);
	pythonGlobal["set_fps"] = py::make_function(Game::setFps);

	pythonGlobal["get_stage_width"] = py::make_function(Game::getStageWidth);
	pythonGlobal["get_stage_height"] = py::make_function(Game::getStageHeight);

	pythonGlobal["set_stage_size"] = py::make_function(Game::setStageSize);
	pythonGlobal["set_fullscreen"] = py::make_function(Game::setFullscreen);

	pythonGlobal["load_image"] = py::make_function(Image::loadImage);
	pythonGlobal["get_texture_width"] = py::make_function(Game::getTextureWidth);
	pythonGlobal["get_texture_height"] = py::make_function(Game::getTextureHeight);
	pythonGlobal["get_pixel"] = py::make_function(Game::getPixel);

	pythonGlobal["get_mouse"] = py::make_function(PyUtils::getMouse);
	pythonGlobal["get_local_mouse"] = py::make_function(PyUtils::getLocalMouse);
	pythonGlobal["get_mouse_down"] = py::make_function(Mouse::getMouseDown);
}

py::list PyUtils::getMouse() {
	py::list res;
	res.append(Mouse::getX());
	res.append(Mouse::getY());
	return res;
}
py::list PyUtils::getLocalMouse() {
	py::list res;
	res.append(Mouse::getLocalX());
	res.append(Mouse::getLocalY());
	return res;
}

PyUtils::~PyUtils() {
	Py_Finalize();
}


void PyUtils::errorProcessing(const String &code) {
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);

	std::string excType = "NoneType";
	if (ptype) {
		py::handle<> htype(ptype);
		excType = py::extract<std::string>(py::str(htype));
	}
	if (excType == "<type 'exceptions.StopIteration'>") {
		Py_DecRef(pvalue);
		Py_DecRef(ptraceback);
		throw StopException();
	}

	std::string excValue = "None";
	if (pvalue) {
		py::handle<> hvalue(py::allow_null(pvalue));
		excValue = py::extract<const std::string>(py::str(hvalue));
	}

	std::string traceback = "None";
	if (ptraceback) {
		try {
			py::handle<> htraceback(py::allow_null(ptraceback));
			GV::pyUtils->pythonGlobal["traceback"] = htraceback;

			String code = "traceback_str = get_traceback(traceback)";
			py::exec(code.c_str(), GV::pyUtils->pythonGlobal);
			traceback = py::extract<const std::string>(GV::pyUtils->pythonGlobal["traceback_str"]);
		} catch (py::error_already_set) {
			traceback = "Error on get traceback\n";
		}
	}

	String out = "Python Error (" + excType + "):\n"
				 "\t" + excValue + "\n"

				 "Traceback:\n" + traceback + "\n"

				 "Code:\n" +
				 code + "\n\n";

	std::cout << out;
	Logger::log(out);
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
				return false;
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

	std::lock_guard<std::mutex> g(pyExecMutex);

	try {
		PyCodeObject *co;
		if (!retRes) {
			co = getCompileObject(code, fileName, numLine);
		}else {
			co = getCompileObject("res = str(" + code + ")", fileName, numLine);
		}

		if (co) {
			if (!PyEval_EvalCode(co, GV::pyUtils->pythonGlobal.ptr(), nullptr)) {
				throw py::error_already_set();
			}

			if (retRes) {
				py::object resObj = GV::pyUtils->pythonGlobal["res"];
				const std::string resStr = py::extract<const std::string>(resObj);
				res = resStr;
				if (isConst) {
					constExprs[code] = res;
				}
			}
		}else {
			std::cout << "Python Compile Error:\n";
			throw py::error_already_set();
		}
	}catch (py::error_already_set) {
		errorProcessing(code);
	}catch (std::exception &e) {
		std::cout << "std::exception: " << e.what() << '\n';
		std::cout << "on python code:\n" << code << '\n';
	}catch (...) {
		std::cout << "Python Unknown Error\n";
		std::cout << "Code:\n" << code << '\n';
	}

	return res;
}

py::object PyUtils::execRetObj(const String &fileName, size_t numLine, const String &code) {
	py::object res;
	if (!code) return res;

	static std::map<String, py::object> constExprs;

	bool isConst = isConstExpr(code);
	if (isConst) {
		auto i = constExprs.find(code);
		if (i != constExprs.end()) {
			return i->second;
		}
	}


	std::lock_guard<std::mutex> g(pyExecMutex);

	try {
		PyCodeObject *co;
		co = getCompileObject("res = " + code, fileName, numLine);

		if (co) {
			if (!PyEval_EvalCode(co, GV::pyUtils->pythonGlobal.ptr(), nullptr)) {
				throw py::error_already_set();
			}

			res = GV::pyUtils->pythonGlobal["res"];
			if (isConst) {
				constExprs[code] = res;
			}
		}else {
			std::cout << "Python Compile Error:\n";
			throw py::error_already_set();
		}
	}catch (py::error_already_set) {
		errorProcessing(code);
	}catch (std::exception &e) {
		std::cout << "std::exception: " << e.what() << '\n';
		std::cout << "on python code:\n" << code << '\n';
	}catch (...) {
		std::cout << "Python Unknown Error\n";
		std::cout << "Code:\n" << code << '\n';
	}

	return res;
}
