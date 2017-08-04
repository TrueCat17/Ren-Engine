#include "py_utils.h"

#include <iostream>

#include <boost/python.hpp>
#include <Python.h>


#include "gv.h"

#include "gui/screen/screen.h"

#include "music.h"

#include "parser/parser.h"

#include "utils/game.h"
#include "utils/mouse.h"
#include "utils/utils.h"


std::map<PyCode, PyCodeObject*> PyUtils::compiledObjects;
std::mutex PyUtils::pyExecGuard;

PyCodeObject* PyUtils::getCompileObject(const String code, const String fileName, size_t numLine) {
	PyCode pyCode(code, fileName, numLine);

	if (compiledObjects.find(pyCode) != compiledObjects.end()) {
		return compiledObjects[pyCode];
	}


	String indent;
	if (numLine > 1) {
		indent.resize(numLine - 1, '\n');
	}

	PyObject *t = Py_CompileString((indent + code).c_str(), fileName.c_str(), Py_file_input);
	PyCodeObject *co = reinterpret_cast<PyCodeObject*>(t);

	compiledObjects[pyCode] = co;
	return co;
}


PyUtils::PyUtils() {
	Py_Initialize();

	mainModule = py::import("__main__");
	pythonGlobal = mainModule.attr("__dict__");


	pythonGlobal["need_save"] = false;

	pythonGlobal["_out_msg"] = py::make_function(Utils::outMsg);
	pythonGlobal["_get_mods"] = py::make_function(Parser::getMods);

	pythonGlobal["_register_channel"] = py::make_function(Music::registerChannel);
	pythonGlobal["_set_volume"] = py::make_function(Music::setVolume);
	pythonGlobal["_set_mixer_volume"] = py::make_function(Music::setMixerVolume);
	pythonGlobal["_play"] = py::make_function(Music::play);
	pythonGlobal["_stop"] = py::make_function(Music::stop);

	pythonGlobal["image_was_registered"] = py::make_function(Utils::imageWasRegistered);
	pythonGlobal["get_image_code"] = py::make_function(Utils::getImageCode);
	pythonGlobal["get_image_decl_at"] = py::make_function(Utils::getImageDeclAt);

	pythonGlobal["show_screen"] = py::make_function(Screen::addToShowSimply);
	pythonGlobal["hide_screen"] = py::make_function(Screen::addToHideSimply);

	pythonGlobal["start_mod"] = py::make_function(Game::startMod);
	pythonGlobal["exit_from_game"] = py::make_function(Game::exitFromGame);

	pythonGlobal["_has_label"] = py::make_function(Game::hasLabel);

	pythonGlobal["_get_from_hard_config"] = py::make_function(Game::getFromConfig);
	pythonGlobal["get_args"] = py::make_function(Game::getArgs);

	pythonGlobal["set_fps"] = py::make_function(Game::setFps);
	pythonGlobal["get_fps"] = py::make_function(Game::getFps);

	pythonGlobal["get_stage_width"] = py::make_function(Game::getStageWidth);
	pythonGlobal["get_stage_height"] = py::make_function(Game::getStageHeight);

	pythonGlobal["get_texture_width"] = py::make_function(Game::getTextureWidth);
	pythonGlobal["get_texture_height"] = py::make_function(Game::getTextureHeight);
	pythonGlobal["get_pixel"] = py::make_function(Game::getPixel);

	pythonGlobal["get_mouse"] = py::make_function(PyUtils::getMouse);
}

py::list PyUtils::getMouse() {
	py::list res;
	res.append(Mouse::getX());
	res.append(Mouse::getY());
	return res;
}

PyUtils::~PyUtils() {
	Py_Finalize();
}


void PyUtils::errorProcessing(const String &code) {
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);

	py::handle<> htype(ptype);
	std::string excType = py::extract<std::string>(py::str(htype));
	if (excType == "<type 'exceptions.StopIteration'>") {
		Py_DecRef(pvalue);
		Py_DecRef(ptraceback);
		throw StopException();
	}

	if (!ptype || !pvalue || !ptraceback) {
		std::cout << "Python Unknown Error\n"
					 "Code: " << code << '\n';
	}

	py::handle<> hvalue(py::allow_null(pvalue));
	std::string excValue = py::extract<std::string>(py::str(hvalue));

	std::string traceback;

	try {
		if (!String(excValue).startsWith("invalid syntax")) {
			py::handle<> htraceback(py::allow_null(ptraceback));
			GV::pyUtils->pythonGlobal["traceback"] = htraceback;

			String code = "traceback_str = get_traceback(traceback)";
			py::exec(code.c_str(), GV::pyUtils->pythonGlobal);
			traceback = py::extract<std::string>(GV::pyUtils->pythonGlobal["traceback_str"]);
		}
	}catch (py::error_already_set) {
		traceback = "Error on call get_traceback\n";
	}

	std::cout << "Python Error (" + excType + "):\n"
			  << '\t' << excValue << '\n';

	if (traceback.size()) {
		std::cout << "Traceback:\n" << traceback;
	}

	std::cout << "Code:\n"
			  << code << "\n\n";
}


String PyUtils::exec(const String &fileName, size_t numLine, String code, bool retRes) {
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


	std::lock_guard<std::mutex> g(pyExecGuard);

	try {
		PyCodeObject *co = getCompileObject(execCode, fileName, numLine);
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
		errorProcessing(code);
	}catch (...) {
		std::cout << "Python Unknown Error\n";
		std::cout << "Code:\n" << code << '\n';
	}

	return res;
}
