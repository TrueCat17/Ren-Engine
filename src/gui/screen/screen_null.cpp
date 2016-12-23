#include "screen_null.h"

#include "gv.h"

#include "parser/node.h"

ScreenNull::ScreenNull(Node *node): ScreenChild(node, this) {
	rect.w = 0;
	rect.h = 0;
}

int ScreenNull::getMaxX() const {
	return rect.w;
}
int ScreenNull::getMaxY() const {
	return rect.h;
}

void ScreenNull::updateProps() {
	enable = true;

	String widthStr = node->getProp("width");
	String heightStr = node->getProp("height");

	double w = widthStr.toDouble();
	double h = heightStr.toDouble();

	if (w >= 0 && w <= 1) w *= GV::width;
	if (h >= 0 && h <= 1) h *= GV::height;

	rect.w = w;
	rect.h = h;
}

void ScreenNull::update() {
	return;
}

void ScreenNull::draw() const {
	return;
}
