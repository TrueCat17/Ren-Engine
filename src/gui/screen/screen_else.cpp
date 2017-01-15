#include "screen_else.h"

ScreenElse::ScreenElse(Node *node, ScreenChild *screenParent, ScreenContainer *prevContainer):
	ScreenContainer(node, screenParent)
{
	this->prevContainer = prevContainer;
}

void ScreenElse::updateProps() {
	if (prevContainersSkipped()) {
		skipped = false;
		ScreenContainer::updateProps();
	}else {
		skipped = true;
	}
}
