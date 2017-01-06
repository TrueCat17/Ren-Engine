#include "gui.h"

#include "gv.h"

#include "gui/screen/screen_child.h"
#include "gui/screen/screen.h"
#include "gui/screen/screen_window.h"

#include "gui/screen/style.h"

#include "utils/utils.h"

void GUI::update() {
	if (GV::screens) {
		GV::initGuard.lock();
		Style::updateAll();
		GV::initGuard.unlock();

		Screen::updateLists();
		ScreenWindow::updateModality();

		ScreenChild::disableAll();

		for (DisplayObject *child : GV::screens->children) {
			Screen *scr = dynamic_cast<Screen*>(child);
			if (scr) {
				try {
					scr->updateProps();
					scr->updateSize();
					scr->updatePos();
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
		ScreenWindow::updateModality();
	}
}
