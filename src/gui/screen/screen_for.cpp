#include "screen_for.h"

ScreenFor::ScreenFor(Node *node, ScreenChild *screenParent): ScreenContainer(node, screenParent) {
	needUpdateChildren = false;

	String params = node->params;

	String in = " in ";
	size_t inPos = params.find(in);
	String beforeIn = params.substr(0, params.find_last_not_of(' ', inPos) + 1);
	String afterIn = params.substr(params.find_first_not_of(' ', inPos + in.size()));

	if (!beforeIn || !afterIn) {
		Utils::outMsg("ScreenFor::ScreenFor", "Неправильный синтаксис команды for:\n<" + params + ">");
		enable = false;
		return;
	}

	static int numCicle = 0;
	String iterName = "screen_iter_" + String(numCicle++);

	init = iterName + " = iter(" + afterIn + ")";
	onStep = beforeIn + " = " + iterName + ".next()";
}

void ScreenFor::updateProps() {
	skipped = true;
	size_t i = 0;

	Utils::execPython(init);
	while (true) {
		try {
			Utils::execPython(onStep);

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
