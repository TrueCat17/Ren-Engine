#include "screen_child.h"

#include <algorithm>

#include "utils/utils.h"


std::vector<ScreenChild*> ScreenChild::screenObjects;

ScreenChild::ScreenChild(Node *node, ScreenChild *screenParent) {
	screenObjects.push_back(this);

	this->screenParent = screenParent;
	if (screenParent && screenParent != this) {
		screenParent->addChild(this);
	}

	this->node = node;
}
ScreenChild::~ScreenChild() {
	auto i = std::find(screenObjects.begin(), screenObjects.end(), this);
	if (i != screenObjects.end()) {
		screenObjects.erase(i);
	}
}

void ScreenChild::disableAll() {
	for (ScreenChild* obj : screenObjects) {
		obj->enable = false;
	}
}

void ScreenChild::updateProps() {
	enable = true;
	if (needUpdateChildren) {
		for (ScreenChild *screenChild : screenChildren) {
			screenChild->updateProps();
		}
	}
	if (!node) return;


	String xAnchorStr;
	String yAnchorStr;
	String anchorStr = node->getProp("anchor");
	if (anchorStr) {
		xAnchorStr = Utils::execPython(anchorStr + "[0]", true);
		yAnchorStr = Utils::execPython(anchorStr + "[1]", true);
	}else {
		xAnchorStr = node->getProp("xanchor");
		yAnchorStr = node->getProp("yanchor");
	}
	xAnchor = xAnchorStr.toDouble();
	yAnchor = yAnchorStr.toDouble();

	String xPosStr;
	String yPosStr;
	String posStr = node->getProp("pos");
	if (posStr) {
		xPosStr = Utils::execPython(posStr + "[0]", true);
		yPosStr = Utils::execPython(posStr + "[1]", true);
	}else {
		xPosStr = node->getProp("xpos");
		yPosStr = node->getProp("ypos");
	}
	xPos = xPosStr.toDouble();
	yPos = yPosStr.toDouble();


	String xAlignStr;
	String yAlignStr;
	String alignStr = node->getProp("align");
	if (alignStr) {
		xAlignStr = Utils::execPython(alignStr + "[0]", true);
		yAlignStr = Utils::execPython(alignStr + "[1]", true);
	}else {
		xAlignStr = node->getProp("xalign");
		yAlignStr = node->getProp("yalign");
	}
	if (xAlignStr) xAnchor = xPos = xAlignStr.toDouble();
	if (yAlignStr) yAnchor = yPos = yAlignStr.toDouble();


	String xSizeStr;
	String ySizeStr;
	String sizeStr = node->getProp("xysize");
	if (sizeStr) {
		xSizeStr = Utils::execPython(sizeStr + "[0]", true);
		ySizeStr = Utils::execPython(sizeStr + "[1]", true);
	}else {
		xSizeStr = node->getProp("xsize");
		ySizeStr = node->getProp("ysize");
	}
	xSize = xSizeStr ? xSizeStr.toDouble() : getDefaultWidth();
	ySize = ySizeStr ? ySizeStr.toDouble() : getDefaultHeight();
}

double ScreenChild::getDefaultWidth() const { return 0.25; }
double ScreenChild::getDefaultHeight() const { return 0.25; }

void ScreenChild::update() {
	double w = xSize;
	if (w > 0 && w <= 1) w *= GV::width;
	double h = ySize;
	if (h > 0 && h <= 1) h *= GV::height;

	setSize(w, h);


	for (ScreenChild *screenChild : screenChildren) {
		screenChild->update();
	}
	if (!node) {
		setPos(0, 0);
		return;
	}

	if (xPos >= -1 && xPos <= 1) xPos *= parent->getWidth();
	if (xAnchor >= -1 && xAnchor <= 1) xAnchor *= getWidth();
	double x = xPos - xAnchor;

	if (yPos >= -1 && yPos <= 1) yPos *= parent->getHeight();
	if (yAnchor >= -1 && yAnchor <= 1) yAnchor *= getHeight();
	double y = yPos - yAnchor;

	setPos(x, y);
}
