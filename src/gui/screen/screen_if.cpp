#include "screen_if.h"

#include "utils/utils.h"

ScreenIf::ScreenIf(Node* node, ScreenChild *screenParent):
	ScreenContainer(node, screenParent)
{
	prevContainer = nullptr;
	condition = "bool(" + node->params + ")";
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
