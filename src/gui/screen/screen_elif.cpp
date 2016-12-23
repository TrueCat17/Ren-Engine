#include "screen_elif.h"

#include "parser/node.h"
#include "utils/utils.h"

ScreenElif::ScreenElif(Node *node, ScreenChild *screenParent, ScreenContainer *prevContainer):
	ScreenContainer(node, screenParent),
	condition("bool(" + node->params + ")")
{
	this->prevContainer = prevContainer;
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
