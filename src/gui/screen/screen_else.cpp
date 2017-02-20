#include "screen_else.h"

ScreenElse::ScreenElse(Node *node, ScreenChild *screenParent, ScreenContainer *prevContainer):
	ScreenContainer(node, screenParent)
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
