#include "screen_while.h"

ScreenWhile::ScreenWhile(Node *node, ScreenChild *screenParent): ScreenContainer(node, screenParent) {
	needUpdateChildren = false;

	condition = node->params;
	if (!condition) {
		Utils::outMsg("ScreenWhile::ScreenWhile", "Неправильный синтаксис команды while:\n<" + node->params + ">");
		enable = false;
	}
}

void ScreenWhile::updateProps() {
	skipped = true;
	size_t i = 0;

	while (Utils::execPython(condition) == "True") {
		try {
			bool adding = i >= screenChildren.size();
			if (adding) {
				addChildrenFromNode();
			}
			for (size_t j = 0; j < countInitChildren; ++j) {
				ScreenChild *child = dynamic_cast<ScreenChild*>(screenChildren[i + j]);
				if (child) {
					child->updateProps();
				}
			}
		}catch (ContinueException) {
		}catch (BreakException) {
			skipped = false;
			break;
		}catch (StopException) {
			break;
		}

		i += countInitChildren;
	}

	ScreenContainer::updateProps();
}
