#include "screen_child.h"

#include <algorithm>

#include "gv.h"
#include "gui/screen/screen.h"

#include "parser/node.h"
#include "utils/utils.h"


std::vector<ScreenChild*> ScreenChild::screenObjects;

ScreenChild::ScreenChild(Node *node, ScreenChild *screenParent) {
	screenObjects.push_back(this);

	this->node = node;

	this->screenParent = screenParent;
	_isFakeContainer = screenParent && screenParent != this;

	const String &type = getType();
	_canCrop = type == "image" || type == "button" || type == "textbutton" || type == "imagemap";
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

void ScreenChild::calculateProps() {
	enable = true;
	if (needUpdateChildren) {
		for (ScreenChild *screenChild : screenChildren) {
			screenChild->calculateProps();
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

	if (canCrop()) {
		String cropStr = node->getProp("crop");
		if (cropStr && cropStr != "None") {
			char start = cropStr.front();
			char end = cropStr.back();
			if ((start == '(' && end == ')') ||
				(start == '[' && end == ']'))
			{
				cropStr = cropStr.substr(1, cropStr.size() - 2);
				std::vector<String> cropVec = cropStr.split(", ");
				if (cropVec.size() == 4) {
					double x = cropVec[0].toDouble();
					double y = cropVec[1].toDouble();
					double w = cropVec[2].toDouble();
					double h = cropVec[3].toDouble();

					if ((x > 0 && x < 1) || cropVec[0] == "1.0") x *= Utils::getTextureWidth(texture);
					if ((y > 0 && y < 1) || cropVec[1] == "1.0") y *= Utils::getTextureHeight(texture);
					if ((w > 0 && w < 1) || cropVec[2] == "1.0") w *= Utils::getTextureWidth(texture);
					if ((h > 0 && h < 1) || cropVec[3] == "1.0") h *= Utils::getTextureHeight(texture);

					crop = {int(x), int(y), int(w), int(h)};
				}else {
					Utils::outMsg("ScreenChild::updateProps", String() +
								  "В свойстве crop ожидался список из 4-х значений, получено " + cropStr.size() + "\n"
								  "crop: <" + start + cropStr + end + ">");
				}
			}else {
				Utils::outMsg("ScreenChild::updateProps", String() +
							  "В свойстве crop ожидался список из 4-х значений\n"
							  "crop: <" + cropStr + ">");
			}
		}
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
