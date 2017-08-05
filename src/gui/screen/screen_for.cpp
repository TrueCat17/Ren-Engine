#include "screen_for.h"

#include "gv.h"
#include "media/py_utils.h"
#include "parser/node.h"
#include "utils/utils.h"

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

	propName = beforeIn;
	iterName = "screen_iter_" + String(GV::numScreenFor++);

	init = iterName + " = iter(" + afterIn + ")";

	for (char c : propName) {
		if ((c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') ||
			 c == '_') continue;

		onStep = propName + " = " + iterName + ".next()";
		break;
	}
}
ScreenFor::~ScreenFor() {
	if (used) {
		std::lock_guard<std::mutex> g(PyUtils::pyExecGuard);
		try {
			py::dict global = py::extract<py::dict>(GV::pyUtils->pythonGlobal);
			if (global.has_key(iterName.c_str())) {
				py::api::delitem(global, iterName.c_str());
			}
		}catch (py::error_already_set) {
			PyUtils::errorProcessing("del " + iterName);
		}
	}
}

void ScreenFor::calculateProps() {
	skipped = true;

	used = true;
	PyUtils::exec(getFileName(), getNumLine(), init);

	py::object nextMethod;
	if (!onStep) {
		std::lock_guard<std::mutex> g(PyUtils::pyExecGuard);
		try {
			py::object iter = GV::pyUtils->pythonGlobal[iterName.c_str()];
			nextMethod = iter.attr("next");
		}catch (py::error_already_set) {
			Utils::outMsg("EMBED_CPP: ScreenChild::calculateProps", "Ошибка при извлечении " + iterName);
			PyUtils::errorProcessing(iterName);
			return;
		}
	}

	size_t i = 0;
	while (true) {
		try {
			if (!onStep) {
				std::lock_guard<std::mutex> g(PyUtils::pyExecGuard);
				try {
					GV::pyUtils->pythonGlobal[propName.c_str()] = nextMethod();
				}catch (py::error_already_set) {
					PyUtils::errorProcessing(propName + " = " + iterName + ".next()");
				}
			}else {
				PyUtils::exec(getFileName(), getNumLine(), onStep);
			}

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
