#include "screen_elif.h"

#include "media/py_utils.h"
#include "parser/node.h"

ScreenElif::ScreenElif(Node *node, ScreenContainer *screenParent, Screen *screen, ScreenContainer *prevContainer):
	ScreenContainer(node, screenParent, screen),
	condition("bool(" + node->params + ")")
{
	this->prevContainer = prevContainer;
}

void ScreenElif::calculateProps() {
	if (prevContainersSkipped()) {
		skipped = PyUtils::exec(getFileName(), getNumLine(), condition, true) != "True";
		if (!skipped) {
			ScreenContainer::calculateProps();
		}
	}else {
		skipped = true;
	}
}
