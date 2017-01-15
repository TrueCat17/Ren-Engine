#include "gui.h"

#include "gv.h"

#include "gui/screen/screen_child.h"
#include "gui/screen/screen.h"

#include "gui/screen/style.h"

#include "utils/utils.h"

void GUI::update() {
	if (GV::screens) {
		Style::disableAll();

		Screen::updateLists();
		Screen::updateModality();

		ScreenChild::disableAll();
		for (DisplayObject *child : GV::screens->children) {
			Screen *scr = dynamic_cast<Screen*>(child);
			if (scr) {
				try {
					//int a = Utils::getTimer();
					scr->updateProps();
					//int b = Utils::getTimer();
					scr->updateSize();
					//int c = Utils::getTimer();
					scr->updatePos();
					//int d = Utils::getTimer();

					//std::cout << (b-a) << '-' << (c-b) << '-' << (d-c) << '\n';
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
		Screen::updateModality();
	}
}
