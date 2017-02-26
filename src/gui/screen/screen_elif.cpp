#include "screen_elif.h"

#include "media/py_utils.h"
#include "parser/node.h"

ScreenElif::ScreenElif(Node *node, ScreenChild *screenParent, ScreenContainer *prevContainer):
	ScreenContainer(node, screenParent),
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
