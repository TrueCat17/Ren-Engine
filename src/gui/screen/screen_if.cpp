#include "screen_if.h"

#include "parser/node.h"
#include "utils/utils.h"

ScreenIf::ScreenIf(Node* node, ScreenChild *screenParent):
	ScreenContainer(node, screenParent),
	condition("bool(" + node->params + ")")
{
	prevContainer = nullptr;
}

bool ScreenIf::enabled() const {
	return !skipped && ScreenContainer::enabled();
}

void ScreenIf::updateProps() {
	skipped = Utils::execPython(condition, true) != "True";
	if (!skipped) {
		ScreenContainer::updateProps();
	}
}
