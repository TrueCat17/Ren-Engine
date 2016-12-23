#include "screen_else.h"

ScreenElse::ScreenElse(Node *node, ScreenChild *screenParent, ScreenContainer *prevContainer):
	ScreenContainer(node, screenParent)
{
	this->prevContainer = prevContainer;
}

bool ScreenElse::enabled() const {
	return !skipped && ScreenContainer::enabled();
}

void ScreenElse::updateProps() {
	if (prevContainersSkipped()) {
		skipped = false;
		ScreenContainer::updateProps();
	}else {
		skipped = true;
	}
}
