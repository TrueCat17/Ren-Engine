#include "screen_while.h"

#include "media/py_utils.h"
#include "parser/node.h"


ScreenWhile::ScreenWhile(Node *node, ScreenChild *screenParent, Screen *screen):
	ScreenContainer(node, screenParent, screen),
	condition(node->params)
{
	needUpdateChildren = false;

	if (!condition) {
		Utils::outMsg("ScreenWhile::ScreenWhile", "Неправильный синтаксис команды while:\n<" + node->params + ">");
		enable = false;
	}
}

void ScreenWhile::calculateProps() {
	skipped = true;
	size_t i = 0;

	while (PyUtils::exec(getFileName(), getNumLine(), condition, true) == "True") {
		try {
			bool adding = i >= screenChildren.size();
			if (adding) {
				if (screenChildren.empty()) {
					inited = false;
				}
				addChildrenFromNode();
				inited = true;
			}
			for (size_t j = 0; j < countInitChildren; ++j) {
				ScreenChild *child = dynamic_cast<ScreenChild*>(screenChildren[i + j]);
				if (child) {
					child->calculateProps();
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

	inited = true;
	ScreenContainer::calculateProps();
}
