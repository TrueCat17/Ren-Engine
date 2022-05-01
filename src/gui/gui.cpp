#include "gui.h"
#include <map>

#define printTime 0
#if printTime
#include <iostream>
#endif

#include "gui/screen/screen.h"
#include "media/py_utils.h"
#include "utils/stage.h"
#include "utils/utils.h"


static PyObject *screenTimes = nullptr;
PyObject* GUI::getScreenTimes() {
	if (!screenTimes) {
		return PyDict_New();
	}

	Py_INCREF(screenTimes);
	return screenTimes;
}


void GUI::update() {
	if (!GV::inGame) return;

	++GV::numUpdate;

	Screen::updateLists();
	Screen::updateScreens();

	DisplayObject::disableAll();
	Stage::screens->enable = true;

	PyUtils::exec("CPP_EMBED: gui.cpp", __LINE__, "globals().has_key('signals') and signals.send('enter_frame')");

	std::map<std::string, double> dict;
	for (DisplayObject *child : Stage::screens->children) {
		double st = Utils::getTimer();

		Screen *scr = static_cast<Screen*>(child);
#if printTime
		std::cout << "update <" << scr->getName() << ">:\n";
		const double time1 = Utils::getTimer();
#endif
		scr->calcProps();
#if printTime
		const double time2 = Utils::getTimer();
#endif
		scr->updateZoom();
#if printTime
		const double time3 = Utils::getTimer();
#endif
		scr->updateRect();
#if printTime
		const double time4 = Utils::getTimer();
#endif
		scr->updateGlobal();
#if printTime
		const double time5 = Utils::getTimer();
		auto round = [](double start, double end) {
			return std::trunc((end - start) * 1e5) / 1e2;
		};
		std::cout << round(time1, time2) << " - " <<
		             round(time2, time3) << " - " <<
		             round(time3, time4) << " - " <<
		             round(time4, time5) << ": " <<
		             round(time1, time5) << '\n';
#endif

		dict[scr->getName()] = Utils::getTimer() - st;
	}

	{
		std::lock_guard g(PyUtils::pyExecMutex);

		PyObject *pyDict = PyDict_New();
		for (auto [name, time] : dict) {
			PyDict_SetItemString(pyDict, name.c_str(), PyFloat_FromDouble(time));
		}

		Py_XDECREF(screenTimes);
		screenTimes = pyDict;
	}

	Screen::updateLists();
	Screen::updateScreens();

	PyUtils::exec("CPP_EMBED: gui.cpp", __LINE__, "globals().has_key('signals') and signals.send('exit_frame')");
}
