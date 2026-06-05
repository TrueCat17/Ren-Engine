#include "gui.h"

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
void GUI::clearScreenTimes() {
	Py_XDECREF(screenTimes);
	screenTimes = nullptr;
}


static void updateImpl(bool saving) {
	if (!GV::inGame) return;

	++GV::numUpdate;

	Screen::updateLists();
	Screen::updateScreens();

	DisplayObject::disableAll();
	Stage::screens->enable = true;


	PyObject *pyDict = PyDict_New();

	for (DisplayObject *child : Stage::screens->children) {
		Screen *scr = static_cast<Screen*>(child);
		if (saving && !scr->save) continue;

		double st = Utils::getTimer();

#define printTime 0

#if printTime
		printf("update <%s>:\n", scr->getName().c_str());
		const double time1 = Utils::getTimer();
#endif
		scr->calcProps();
		scr->updateTexture();
#if printTime
		const double time2 = Utils::getTimer();
#endif
		scr->updateZoom();
#if printTime
		const double time3 = Utils::getTimer();
#endif
		scr->updateSize();
		scr->updatePos();
#if printTime
		const double time4 = Utils::getTimer();
#endif
		scr->updateGlobal();
#if printTime
		const double time5 = Utils::getTimer();
		printf(
		    "%.2f - %.2f - %.2f - %.2f: %.2f\n",
		    (time2 - time1) * 1000,
		    (time3 - time2) * 1000,
		    (time4 - time3) * 1000,
		    (time5 - time4) * 1000,
		    (time5 - time1) * 1000
		);
#endif

		const std::string &name = scr->getName();
		double time = Utils::getTimer() - st;

		PyObject *pyTime = PyFloat_FromDouble(time);
		PyDict_SetItemString(pyDict, name.c_str(), pyTime);
		Py_DECREF(pyTime);
	}

	Py_XDECREF(screenTimes);
	screenTimes = pyDict;

	Screen::updateLists();
	Screen::updateScreens();
}

void GUI::update(bool saving) {
	PyUtils::callInPythonThread([&]() {
		updateImpl(saving);
	});
}
