#include "gui.h"

#define printTime 0
#if printTime
#include <iostream>
#endif

#include "gv.h"

#include "gui/screen/child.h"
#include "gui/screen/screen.h"

#include "gui/screen/style.h"

#include "utils/utils.h"


void GUI::update() {
	++GV::numUpdate;

	if (GV::screens) {
		Screen::updateLists();
		Screen::updateScreens();

		DisplayObject::disableAll();
		for (DisplayObject *child : GV::screens->children) {
			Screen *scr = static_cast<Screen*>(child);
			try {
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
			}catch (StopException&) {
				Utils::outMsg("gui::update", "Неожидаемое исключение StopException (конец итератора)");
			}
		}

		Screen::updateLists();
		Screen::updateScreens();
	}
}
