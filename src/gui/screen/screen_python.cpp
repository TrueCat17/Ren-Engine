#include "screen_python.h"

#include "media/py_utils.h"
#include "parser/node.h"

ScreenPython::ScreenPython(Node *node, bool isBlock):
	ScreenChild(node, nullptr, nullptr),
	isBlock(isBlock)
{ }

void ScreenPython::calculateProps() {
	PyUtils::exec(getFileName(), getNumLine() + isBlock, node->params);
}
