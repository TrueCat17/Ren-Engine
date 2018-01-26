#include "screen_python.h"

#include "gv.h"
#include "media/py_utils.h"
#include "parser/node.h"

ScreenPython::ScreenPython(Node *node, bool isBlock):
	ScreenChild(node, nullptr, nullptr)
{
	co = PyUtils::getCompileObject(node->params, getFileName(), getNumLine() + isBlock, true);
	if (!co) {
		PyUtils::errorProcessing(node->params);
	}
}

void ScreenPython::calculateProps() {
	if (co) {
		std::lock_guard<std::mutex> g(PyUtils::pyExecMutex);
		if (!PyEval_EvalCode(co, GV::pyUtils->pythonGlobal.ptr(), nullptr)) {
			PyUtils::errorProcessing(node->params);
		}
	}
}
