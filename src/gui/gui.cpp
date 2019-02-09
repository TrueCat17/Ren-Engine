#include "gui.h"

#define printTime 0
#if printTime
#include <iostream>
#endif

#include "gv.h"

#include "gui/screen/screen.h"

#include "utils/utils.h"


void GUI::update() {
	if (!GV::screens) return;

	++GV::numUpdate;

	Screen::updateLists();
	Screen::updateScreens();

	DisplayObject::disableAll();
	GV::screens->enable = true;

	for (DisplayObject *child : GV::screens->children) {
		Screen *scr = static_cast<Screen*>(child);
#if printTime
		std::cout << "update <" << scr->getName() << ">:\n";
		const int a = Utils::getTimer();
#endif
		scr->calcProps();
#if printTime
		const int b = Utils::getTimer();
#endif
		scr->updateRect();
		scr->updateGlobal();
#if printTime
		const int c = Utils::getTimer();
		std::cout << (b-a) << '-' << (c-b) << '\n';
#endif
	}

	Screen::updateLists();
	Screen::updateScreens();
}
