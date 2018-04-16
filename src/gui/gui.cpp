#include "gui.h"

#define printTime 0
#if printTime
#include <iostream>
#endif

#include "gv.h"

#include "gui/screen/screen_child.h"
#include "gui/screen/screen.h"

#include "gui/screen/style.h"

#include "utils/utils.h"


void GUI::update() {
	if (GV::screens) {
		Screen::updateLists();
		Screen::updateScreens();

		ScreenChild::disableAll();
		for (DisplayObject *child : GV::screens->children) {
			Screen *scr = dynamic_cast<Screen*>(child);
			if (scr) {
				try {
#if printTime
					std::cout << "update <" << scr->getName() << ">:\n";
					const int a = Utils::getTimer();
#endif
					scr->calculateProps();
#if printTime
					const int b = Utils::getTimer();
#endif
					scr->updateSize();
					scr->updatePos();
					scr->updateGlobalPos();
					scr->updateGlobalAlpha();
#if printTime
					const int c = Utils::getTimer();
					std::cout << (b-a) << '-' << (c-b) << '\n';
#endif
				}catch (ContinueException) {
					Utils::outMsg("GUI::update", "continue вне цикла");
				}catch (BreakException) {
					Utils::outMsg("GUI::update", "break вне цикла");
				}catch (StopException) {
					Utils::outMsg("GUI::update", "Неожидаемое исключение StopException (конец итератора)");
				}
			}
		}

		Screen::updateLists();
		Screen::updateScreens();
	}
}
