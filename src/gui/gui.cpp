#include "gui.h"

#include "gv.h"

#include "gui/text.h"
#include "gui/screen/screen.h"
#include "gui/screen/screen_child.h"

#include "gui/screen/style.h"

#include "utils/utils.h"

void GUI::update() {
	GV::updateGuard.lock();
	if (GV::inGame && GV::screens) {
		Style::updateAll();
		ScreenChild::disableAll();

		for (DisplayObject *child : GV::screens->children) {
			Screen *scr = dynamic_cast<Screen*>(child);
			if (scr) {
				try {
					scr->updateProps();
					scr->update();
				}catch (ContinueException) {
					Utils::outMsg("GUI::update", "continue вне цикла");
				}catch (BreakException) {
					Utils::outMsg("GUI::update", "break вне цикла");
				}catch (StopException) {
					Utils::outMsg("GUI::update", "Неожидаемое исключение StopException (конец итератора)");
				}
			}
		}
	}
	GV::updateGuard.unlock();
}
