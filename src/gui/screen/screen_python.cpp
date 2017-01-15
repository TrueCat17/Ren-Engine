#include "screen_python.h"

#include "media/py_utils.h"
#include "parser/node.h"

ScreenPython::ScreenPython(Node *node): ScreenChild(node, nullptr) {

}

void ScreenPython::updateProps() {
	PyUtils::exec(node->params);
}
void ScreenPython::updateSize() {
	return;
}
void ScreenPython::updatePos() {
	return;
}

void ScreenPython::draw() const {
	return;
}
