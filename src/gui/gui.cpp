#include "gui.h"

#include <iostream>


#include "gv.h"

#include "gui/screen/screen_child.h"
#include "gui/screen/screen.h"

#include "gui/screen/style.h"

#include "utils/utils.h"


void GUI::update() {
	if (GV::screens) {
		Style::disableAll();

		Screen::updateLists();
		Screen::updateScreens();

		ScreenChild::disableAll();
		for (DisplayObject *child : GV::screens->children) {
			Screen *scr = dynamic_cast<Screen*>(child);
			if (scr) {
				try {
//					std::cout << scr->name << ":\n";
//					int a = Utils::getTimer();
					scr->calculateProps();
//					int b = Utils::getTimer();
					scr->updateSize();
//					int c = Utils::getTimer();
					scr->updatePos();
//					int d = Utils::getTimer();
					scr->updateGlobalPos();
					scr->updateGlobalAlpha();
//					int e = Utils::getTimer();

//					std::cout << (b-a) << '-' << (c-b) << '-' << (d-c) << '-' << (e-d) << '\n';
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
