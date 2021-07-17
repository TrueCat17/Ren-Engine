#include "gui.h"
#include "media/py_utils.h"

#define printTime 0
#if printTime
#include <iostream>
#include "utils/utils.h"
#endif

#include "gui/screen/screen.h"


void GUI::update() {
	if (!GV::inGame) return;

	++GV::numUpdate;

	Screen::updateLists();
	Screen::updateScreens();

	DisplayObject::disableAll();
	GV::screens->enable = true;

	PyUtils::exec("CPP_EMBED: gui.cpp", __LINE__, "globals().has_key('signals') and signals.send('enter_frame')");

	for (DisplayObject *child : GV::screens->children) {
		Screen *scr = static_cast<Screen*>(child);
#if printTime
		std::cout << "update <" << scr->getName() << ">:\n";
		const double a = Utils::getTimer();
#endif
		scr->calcProps();
#if printTime
		const double b = Utils::getTimer();
#endif
		scr->updateRect();
		scr->updateGlobal();
#if printTime
		const double c = Utils::getTimer();
		std::cout << (b-a) << '-' << (c-b) << '\n';
#endif
	}

	Screen::updateLists();
	Screen::updateScreens();

	PyUtils::exec("CPP_EMBED: gui.cpp", __LINE__, "globals().has_key('signals') and signals.send('exit_frame')");
}
