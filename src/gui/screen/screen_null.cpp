#include "screen_null.h"

ScreenNull::ScreenNull(Node *node, ScreenChild *screenParent): ScreenChild(node, screenParent) {
	rect.w = rect.h = 0;
}

int ScreenNull::getMaxX() const {
	return rect.w;
}
int ScreenNull::getMaxY() const {
	return rect.h;
}

void ScreenNull::updateProps() {
	String widthStr = node->getProp("width");
	String heightStr = node->getProp("height");

	rect.w = widthStr ? widthStr.toInt() : 0;
	rect.h = heightStr ? heightStr.toInt() : 0;
}

void ScreenNull::update() {
	return;
}

void ScreenNull::draw() const {
	return;
}
