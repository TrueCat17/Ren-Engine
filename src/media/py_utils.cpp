#include "py_utils.h"

#include <iostream>
#include <mutex>

#include <boost/python.hpp>
#include <Python.h>


#include "gv.h"

#include "gui/screen/screen.h"

#include "music_channel.h"

#include "parser/parser.h"

#include "utils/game.h"
#include "utils/utils.h"


std::map<String, PyCodeObject*> PyUtils::compiledObjects;

PyCodeObject* PyUtils::getCompileObject(const String& code) {
	if (compiledObjects.find(code) != compiledObjects.end()) {
		return compiledObjects[code];
	}

	PyObject *t = Py_CompileString(code.c_str(), "<string>", Py_file_input);
	PyCodeObject *co = reinterpret_cast<PyCodeObject*>(t);

	compiledObjects[code] = co;
	return co;
}


PyUtils::PyUtils() {
	Py_Initialize();

	mainModule = py::import("__main__");
	pythonGlobal = mainModule.attr("__dict__");


	pythonGlobal["_out_msg"] = py::make_function(Utils::outMsg);
	pythonGlobal["_get_mods"] = py::make_function(Parser::getMods);

	pythonGlobal["_register_channel"] = py::make_function(MusicChannel::make);
	pythonGlobal["_set_volume"] = py::make_function(MusicChannel::setVolume);
	pythonGlobal["_play"] = py::make_function(MusicChannel::play);
	pythonGlobal["_stop"] = py::make_function(MusicChannel::stop);

	pythonGlobal["get_image_code"] = py::make_function(Utils::getImageCode);
	pythonGlobal["show_screen"] = py::make_function(Screen::addToShowSimply);
	pythonGlobal["hide_screen"] = py::make_function(Screen::addToHideSimply);

	pythonGlobal["start_mod"] = py::make_function(Game::startMod);
	pythonGlobal["exit_from_game"] = py::make_function(Game::exitFromGame);

	pythonGlobal["_get_from_hard_config"] = py::make_function(Game::getFromConfig);
	pythonGlobal["set_fps"] = py::make_function(Game::setFps);
	pythonGlobal["get_fps"] = py::make_function(Game::getFps);

	pythonGlobal["get_stage_width"] = py::make_function(Game::getStageWidth);
	pythonGlobal["get_stage_height"] = py::make_function(Game::getStageHeight);

	pythonGlobal["get_texture_width"] = py::make_function(Game::getTextureWidth);
	pythonGlobal["get_texture_height"] = py::make_function(Game::getTextureHeight);
}

PyUtils::~PyUtils() {
	Py_Finalize();
}


String PyUtils::exec(String code, bool retRes) {
	if (!code) return "";

	if (code.isNumber()) {
		return code;
	}
	if (code.isSimpleString()) {
		return code.substr(1, code.size() - 2);
	}
	if (code == "True" || code == "False" || code == "None") {
		return code;
	}


	static std::map<String, String> constExprs;
	bool isConst = true;
	if (retRes) {
		for (char c : code) {
			if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
				isConst = false;
				break;
			}
		}
		if (isConst) {
			auto i = constExprs.find(code);
			if (i != constExprs.end()) {
				return i->second;
			}
		}
	}else {
		isConst = false;
	}

	String res = "empty";
	String execCode = code;
	if (retRes) {
		execCode = "res = str(" + code + ")";
	}


	static std::mutex pyExecGuard;

	pyExecGuard.lock();
	try {
		PyCodeObject *co = getCompileObject(execCode);
		if (co) {
			if (!PyEval_EvalCode(co, GV::pyUtils->pythonGlobal.ptr(), nullptr)) {
				throw py::error_already_set();
			}

			if (retRes) {
				py::object resObj = GV::pyUtils->pythonGlobal["res"];
				const std::string resStr = py::extract<const std::string>(resObj);
				res = resStr;
				if (isConst && constExprs.size() < 100000) {
					constExprs[code] = res;
				}
			}
		}else {
			std::cout << "Python Compile Error:\n";
			throw py::error_already_set();
		}
	}catch (py::error_already_set) {
		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
		PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
		if (!ptype || !pvalue || !ptraceback) {
			std::cout << "Python Unknown Error\n"
						 "Code: " << code << '\n';
			pyExecGuard.unlock();
			return res;
		}

		py::handle<> htype(ptype);
		std::string excType = py::extract<std::string>(py::str(htype));
		if (excType == "<type 'exceptions.StopIteration'>") {
			pyExecGuard.unlock();
			Py_DecRef(pvalue);
			Py_DecRef(ptraceback);
			throw StopException();
		}

		py::handle<> hvalue(py::allow_null(pvalue));
		std::string excValue = py::extract<std::string>(py::str(hvalue));

		std::string traceback;
		if (!String(excValue).startsWith("invalid syntax")) {
			py::handle<> htraceback(py::allow_null(ptraceback));
			GV::pyUtils->pythonGlobal["traceback"] = htraceback;

			String code = "traceback_str = get_traceback(traceback)";
			try {
				py::exec(code.c_str(), GV::pyUtils->pythonGlobal);
				traceback = py::extract<std::string>(GV::pyUtils->pythonGlobal["traceback_str"]);
			}catch (py::error_already_set) {
				traceback = "Error on call get_traceback\n";
			}
		}

		std::cout << "Python Error (" + excType + "):\n"
				  << '\t' << excValue << '\n';

		if (traceback.size()) {
			std::cout << "Traceback:\n"
					  << '\t' << traceback;
		}

		std::cout << "Code:\n"
				  << code << "\n\n";
	}catch (...) {
		std::cout << "Python Unknown Error\n";
		std::cout << "Code:\n" << code << '\n';
	}
	pyExecGuard.unlock();

	return res;
}
