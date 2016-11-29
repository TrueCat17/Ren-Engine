#include "screen_python.h"

#include "utils/utils.h"

ScreenPython::ScreenPython(Node *node): ScreenChild(node, nullptr) {

}

void ScreenPython::updateProps() {
	Utils::execPython(node->params);
}
void ScreenPython::update() {
	return;
}

void ScreenPython::draw() const {
	return;
}
