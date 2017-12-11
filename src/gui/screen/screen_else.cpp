#include "screen_else.h"

ScreenElse::ScreenElse(Node *node, ScreenChild *screenParent, Screen *screen, ScreenContainer *prevContainer):
	ScreenContainer(node, screenParent, screen)
{
	this->prevContainer = prevContainer;
}

void ScreenElse::calculateProps() {
	if (prevContainersSkipped()) {
		skipped = false;
		ScreenContainer::calculateProps();
	}else {
		skipped = true;
	}
}
