#include "gui.h"

#include <iostream>


#include "gv.h"

#include "gui/screen/screen_child.h"
#include "gui/screen/screen.h"

#include "gui/screen/style.h"


void GUI::update() {
	if (GV::screens) {
		Screen::updateLists();
		Screen::updateScreens();

		ScreenChild::disableAll();
		for (DisplayObject *child : GV::screens->children) {
			Screen *scr = dynamic_cast<Screen*>(child);
			if (scr) {
				try {
//					std::cout << "update <" << scr->name << ">:\n";
//					int a = Utils::getTimer();
					scr->calculateProps();
//					int b = Utils::getTimer();
					scr->updateSize();
					scr->updatePos();
					scr->updateGlobalPos();
					scr->updateGlobalAlpha();
//					int c = Utils::getTimer();

//					std::cout << (b-a) << '-' << (c-b) << '\n';
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
