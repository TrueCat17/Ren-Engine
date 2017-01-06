#include "screen_python.h"

#include "parser/node.h"
#include "utils/utils.h"

ScreenPython::ScreenPython(Node *node): ScreenChild(node, nullptr) {

}

void ScreenPython::updateProps() {
	Utils::execPython(node->params);
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
