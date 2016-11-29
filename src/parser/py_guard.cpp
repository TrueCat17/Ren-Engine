#include "py_guard.h"

#include <boost/python.hpp>
#include <Python.h>

#include "parser/music_channel.h"

#include "utils/game.h"
#include "utils/utils.h"

std::map<String, PyCodeObject*> PyGuard::compiledObjects;

PyCodeObject* PyGuard::getCompileObject(const String& code) {
	if (compiledObjects.find(code) != compiledObjects.end()) {
		return compiledObjects[code];
	}

	PyObject *t = Py_CompileString(code.c_str(), "<string>", Py_file_input);
	PyCodeObject *co = reinterpret_cast<PyCodeObject*>(t);

	compiledObjects[code] = co;
	return co;
}


PyGuard::PyGuard() {
	Py_Initialize();

	mainModule = py::import("__main__");
	pythonGlobal = mainModule.attr("__dict__");


	pythonGlobal["sleep"] = py::make_function(Utils::sleep);
	pythonGlobal["_out_msg"] = py::make_function(Utils::outMsg);

	pythonGlobal["register_channel"] = py::make_function(MusicChannel::make);
	pythonGlobal["volume"] = py::make_function(MusicChannel::setVolume);

	pythonGlobal["get_image"] = py::make_function(Utils::getImage);

	pythonGlobal["start_mod"] = py::make_function(Game::startMod);
	pythonGlobal["exit_from_game"] = py::make_function(Game::exitFromGame);

	pythonGlobal["set_fps"] = py::make_function(Game::setFps);
	pythonGlobal["get_fps"] = py::make_function(Game::getFps);

	pythonGlobal["get_stage_width"] = py::make_function(Game::getStageWidth);
	pythonGlobal["get_stage_height"] = py::make_function(Game::getStageHeight);
}

PyGuard::~PyGuard() {
	Py_Finalize();
}
