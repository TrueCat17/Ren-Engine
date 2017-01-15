#include "screen_while.h"

#include "media/py_utils.h"
#include "parser/node.h"
#include "utils/utils.h"

ScreenWhile::ScreenWhile(Node *node, ScreenChild *screenParent):
	ScreenContainer(node, screenParent),
	condition(node->params)
{
	needUpdateChildren = false;

	if (!condition) {
		Utils::outMsg("ScreenWhile::ScreenWhile", "Неправильный синтаксис команды while:\n<" + node->params + ">");
		enable = false;
	}
}

void ScreenWhile::updateProps() {
	skipped = true;
	size_t i = 0;

	while (PyUtils::exec(condition) == "True") {
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

	inited = true;
	ScreenContainer::updateProps();
}
