#include "screen_child.h"

#include <algorithm>

#include "gv.h"
#include "gui/screen/screen.h"

#include "parser/node.h"
#include "utils/utils.h"


std::vector<ScreenChild*> ScreenChild::screenObjects;

ScreenChild::ScreenChild(Node *node, ScreenChild *screenParent) {
	screenObjects.push_back(this);

	this->screenParent = screenParent;
	_isFakeContainer = screenParent && screenParent != this;

	this->node = node;
}
ScreenChild::~ScreenChild() {
	auto i = std::find(screenObjects.begin(), screenObjects.end(), this);
	if (i != screenObjects.end()) {
		screenObjects.erase(i);
	}
}
const String& ScreenChild::getType() const {
	return node->command;
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

	if (!screen) {
		const DisplayObject *t = this;
		while (t && !dynamic_cast<const Screen*>(t)) {
			t = t->parent;
		}
		screen = dynamic_cast<const Screen*>(t);
	}

	bool inHBox = false;
	bool inVBox = false;
	ScreenContainer *p = dynamic_cast<ScreenContainer*>(parent);
	if (p) {
		ScreenContainer *pScreenParent = dynamic_cast<ScreenContainer*>(p->screenParent);
		if (pScreenParent) {
			inHBox = pScreenParent->isHBox();
			inVBox = pScreenParent->isVBox();
		}
	}

	if (!inHBox) {
		String xAnchorStr = node->getProp("xanchor", "anchor", "[0]");
		xAnchor = xAnchorStr.toDouble();
		xAnchorIsDouble = xAnchorStr.contains('.') && xAnchor > 0 && xAnchor <= 1;
		if (xAnchorStr != "None" && !xAnchorStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps", "xanchor is not a number (" + xAnchorStr + ")");
		}

		String xPosStr = node->getProp("xpos", "pos", "[0]");
		xPos = xPosStr.toDouble();
		xPosIsDouble = xPosStr.contains('.') && xPos > 0 && xPos <= 1;
		if (xPosStr != "None" && !xPosStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps", "xpos is not a number (" + xPosStr + ")");
		}

		String xAlignStr = node->getProp("xalign", "align", "[0]");
		if (xAlignStr.isNumber()) {
			xAnchor = xPos = xAlignStr.toDouble();
			xAnchorIsDouble = xPosIsDouble = xAlignStr.contains('.') && xAnchor > 0 && xAnchor <= 1;
		}else if (xAlignStr != "None") {
			Utils::outMsg("ScreenChild::updateProps", "xalign is not a number (" + xAlignStr + ")");
		}
	}
	if (!inVBox) {
		String yAnchorStr = node->getProp("yanchor", "anchor", "[1]");
		yAnchor = yAnchorStr.toDouble();
		yAnchorIsDouble = yAnchorStr.contains('.') && yAnchor > 0 && yAnchor <= 1;
		if (yAnchorStr != "None" && !yAnchorStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps", "yanchor is not a number (" + yAnchorStr + ")");
		}

		String yPosStr = node->getProp("ypos", "pos", "[1]");
		yPos = yPosStr.toDouble();
		yPosIsDouble = yPosStr.contains('.') && yPos > 0 && yPos <= 1;
		if (yPosStr != "None" && !yPosStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps", "ypos is not a number (" + yPosStr + ")");
		}

		String yAlignStr = node->getProp("yalign", "align", "[1]");
		if (yAlignStr.isNumber()) {
			yAnchor = yPos = yAlignStr.toDouble();
			yAnchorIsDouble = yPosIsDouble = yAlignStr.contains('.') && yAnchor > 0 && yAnchor <= 1;
		}else if (yAlignStr && yAlignStr != "None") {
			Utils::outMsg("ScreenChild::updateProps", "yalign is not a number (" + yAlignStr + ")");
		}
	}

	String xSizeStr = node->getProp("xsize", "xysize", "[0]");
	xSize = xSizeStr.toDouble();
	xSizeIsDouble = xSizeStr.contains('.') && xSize > 0 && xSize <= 1;
	if (xSizeStr != "None" && !xSizeStr.isNumber()) {
		Utils::outMsg("ScreenChild::updateProps", "xsize is not a number (" + xSizeStr + ")");
	}

	String ySizeStr = node->getProp("ysize", "xysize", "[1]");
	ySize = ySizeStr.toDouble();
	ySizeIsDouble = ySizeStr.contains('.') && ySize > 0 && ySize <= 1;
	if (ySizeStr != "None" && !ySizeStr.isNumber()) {
		Utils::outMsg("ScreenChild::updateProps", "ysize is not a number (" + ySizeStr + ")");
	}
}

void ScreenChild::updateSize() {
	double w = xSize;
	if (xSizeIsDouble) w *= GV::width;
	double h = ySize;
	if (ySizeIsDouble) h *= GV::height;

	setSize(w, h);

	for (ScreenChild *screenChild : screenChildren) {
		if (screenChild->enable) {
			screenChild->updateSize();
		}
	}
}
void ScreenChild::updatePos() {
	bool inHBox = false;
	bool inVBox = false;
	ScreenContainer *p = dynamic_cast<ScreenContainer*>(parent);
	if (p) {
		ScreenContainer *pScreenParent = dynamic_cast<ScreenContainer*>(p->screenParent);
		if (pScreenParent) {
			inHBox = pScreenParent->isHBox();
			inVBox = pScreenParent->isVBox();
		}
	}

	if (!inHBox) {
		if (xPosIsDouble) xPos *= parent->getWidth();
		if (xAnchorIsDouble) xAnchor *= getWidth();
		double x = xPos - xAnchor;
		setX(x);
	}

	if (!inVBox) {
		if (yPosIsDouble) yPos *= parent->getHeight();
		if (yAnchorIsDouble) yAnchor *= getHeight();
		double y = yPos - yAnchor;
		setY(y);
	}


	for (ScreenChild *screenChild : screenChildren) {
		if (screenChild->enable) {
			screenChild->updatePos();
		}
	}
}


bool ScreenChild::isModal() const {
	bool screenIsModal = screen ? screen->screenIsModal() : false;
	bool hasModal = Screen::hasModal();
	return !hasModal || screenIsModal;//Нет модальных окон или есть, но мы находимся внутри такого окна
}
