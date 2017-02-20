#include "screen_if.h"

#include "media/py_utils.h"
#include "parser/node.h"

ScreenIf::ScreenIf(Node* node, ScreenChild *screenParent):
	ScreenContainer(node, screenParent),
	condition("bool(" + node->params + ")")
{
	prevContainer = nullptr;
}

void ScreenIf::calculateProps() {
	skipped = PyUtils::exec(condition, true) != "True";
	if (!skipped) {
		ScreenContainer::calculateProps();
	}
}
