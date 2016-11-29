#include "screen_elif.h"

#include "utils/utils.h"

ScreenElif::ScreenElif(Node *node, ScreenChild *screenParent, ScreenContainer *prevContainer):
	ScreenContainer(node, screenParent)
{
	this->prevContainer = prevContainer;
	condition = "bool(" + node->params + ")";
}

bool ScreenElif::enabled() const {
	return !skipped && ScreenContainer::enabled();
}

void ScreenElif::updateProps() {
	if (prevContainersSkipped()) {
		skipped = Utils::execPython(condition, true) != "True";
		if (!skipped) {
			ScreenContainer::updateProps();
		}
	}else {
		skipped = true;
	}
}
