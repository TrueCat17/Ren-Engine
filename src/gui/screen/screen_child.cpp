#include "screen_child.h"

#include <iostream>
#include <algorithm>


#include "gv.h"

#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "parser/node.h"
#include "media/py_utils.h"
#include "utils/utils.h"


std::vector<ScreenChild*> ScreenChild::screenObjects;

ScreenChild::ScreenChild(Node *node, ScreenChild *screenParent) {
	screenObjects.push_back(this);

	this->node = node;

	this->screenParent = screenParent;
	_isFakeContainer = screenParent && screenParent != this;

	const String &type = getType();
	_canCrop = type == "image" || type == "button" || type == "textbutton" || type == "imagemap";


	setProp("xanchor", node->getPropCode("xanchor", "anchor", "[0]"));
	setProp("yanchor", node->getPropCode("yanchor", "anchor", "[1]"));
	setProp("xpos", node->getPropCode("xpos", "pos", "[0]"));
	setProp("ypos", node->getPropCode("ypos", "pos", "[1]"));
	setProp("xalign", node->getPropCode("xalign", "align", "[0]"));
	setProp("yalign", node->getPropCode("yalign", "align", "[1]"));

	setProp("xsize", node->getPropCode("xsize", "size", "[0]"));
	setProp("ysize", node->getPropCode("ysize", "size", "[1]"));

	setProp("crop", node->getPropCode("crop"));
	setProp("rotate", node->getPropCode("rotate"));
	setProp("alpha", node->getPropCode("alpha"));
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

void ScreenChild::removeAllProps() {
	propCodes.clear();
	propNumLines.clear();
	propInStyle.clear();
	propValues.clear();
}

void ScreenChild::setProp(const String &propName, const NodeProp &nodeProp) {
	if (nodeProp.pyExpr) {
		propCodes[propName] = nodeProp.pyExpr;
		propNumLines[propName] = nodeProp.numLine;
	}else {
		propInStyle[propName] = nodeProp;
	}
	propValues[propName] = "";
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


	if (isFakeContainer()) return;



	for (auto &i : propInStyle) {
		const String &propName = i.first;
		const NodeProp &nodeProp = i.second;

		propValues[propName] = Style::getProp(nodeProp.styleName, nodeProp.propName);
	}



	if (propCodes.empty()) return;

	{
		std::lock_guard<std::mutex> g(PyUtils::pyExecGuard);
		GV::pyUtils->pythonGlobal["calc_object"] = py::object();//... = None
	}

	String code =
				"calc_object = {\n";
	for (auto &i : propCodes) {
		const String &propName = i.first;
		const String &propExpr = i.second;

		code += "    '" + propName + "': " + propExpr + ",\n";
	}
	code.erase(code.size() - 2);
	code +=     "\n"
				"}\n";
	PyUtils::exec("EMBED_CPP: ScreenChild.cpp", __LINE__, code);

	bool ok = false;
	{
		std::lock_guard<std::mutex> g(PyUtils::pyExecGuard);

		String errorDesc = "Ошибка при извлечении calc_object";
		try {
			py::object obj = GV::pyUtils->pythonGlobal["calc_object"];

			//ok (try)
			if (!obj.is_none()) {
				errorDesc = "Ошибка при извлечении свойств из calc_object";
				for (auto &i : propCodes) {
					const String &propName = i.first;

					py::object res = obj[propName.c_str()];
					propValues[propName] = String(py::extract<const std::string>(py::str(res)));
				}
				ok = true;
			}
		}catch (py::error_already_set) {
			Utils::outMsg("EMBED_CPP: ScreenChild::calculateProps", errorDesc);
			PyUtils::errorProcessing("EMBED_CPP: ScreenChild::calculateProps");
		}
	}

	//some error (except)
	if (!ok) {
		for (auto &i : propCodes) {
			const String &propName = i.first;
			const String &propExpr = i.second;
			const String &fileName = node->getFileName();
			size_t numLine = propNumLines.at(propName);

			propValues[propName] = PyUtils::exec(fileName, numLine, propExpr, true);
		}
	}



	updateTexture();


	if (!needUpdateFields) return;


	if (!inHBox) {
		const String &xAnchorStr = propValues.at("xanchor");
		xAnchor = xAnchorStr.toDouble();
		xAnchorIsDouble = xAnchor > 0 && xAnchor <= 1 && xAnchorStr.contains('.');
		if (xAnchorStr != "None" && !xAnchorStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps",
						  "xanchor is not a number (" + xAnchorStr + ")\n" + node->getPlace());
		}

		const String &xPosStr = propValues.at("xpos");
		xPos = xPosStr.toDouble();
		xPosIsDouble = xPos > 0 && xPos <= 1 && xPosStr.contains('.');
		if (xPosStr != "None" && !xPosStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps",
						  "xpos is not a number (" + xPosStr + ")\n" + node->getPlace());
		}

		const String &xAlignStr = propValues.at("xalign");
		if (xAlignStr.isNumber()) {
			xAnchor = xPos = xAlignStr.toDouble();
			xAnchorIsDouble = xPosIsDouble = xAnchor > 0 && xAnchor <= 1 && xAlignStr.contains('.');
		}else if (xAlignStr != "None") {
			Utils::outMsg("ScreenChild::updateProps",
						  "xalign is not a number (" + xAlignStr + ")\n" + node->getPlace());
		}
	}

	if (!inVBox) {
		const String &yAnchorStr = propValues.at("yanchor");
		yAnchor = yAnchorStr.toDouble();
		yAnchorIsDouble = yAnchor > 0 && yAnchor <= 1 && yAnchorStr.contains('.');
		if (yAnchorStr != "None" && !yAnchorStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps",
						  "yanchor is not a number (" + yAnchorStr + ")\n" + node->getPlace());
		}

		const String &yPosStr = propValues.at("ypos");
		yPos = yPosStr.toDouble();
		yPosIsDouble = yPos > 0 && yPos <= 1 && yPosStr.contains('.');
		if (yPosStr != "None" && !yPosStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps",
						  "ypos is not a number (" + yPosStr + ")\n" + node->getPlace());
		}

		const String &yAlignStr = propValues.at("yalign");
		if (yAlignStr.isNumber()) {
			yAnchor = yPos = yAlignStr.toDouble();
			yAnchorIsDouble = yPosIsDouble = yAnchor > 0 && yAnchor <= 1 && yAlignStr.contains('.');
		}else if (yAlignStr && yAlignStr != "None") {
			Utils::outMsg("ScreenChild::updateProps",
						  "yalign is not a number (" + yAlignStr + ")\n" + node->getPlace());
		}
	}



	const String &xSizeStr = propValues.at("xsize");
	xSize = xSizeStr.toDouble();
	xSizeIsDouble = xSize > 0 && xSize <= 1 && xSizeStr.contains('.');
	if (xSizeStr != "None" && !xSizeStr.isNumber()) {
		Utils::outMsg("ScreenChild::updateProps",
					  "xsize is not a number (" + xSizeStr + ")\n" + node->getPlace());
	}

	const String &ySizeStr = propValues.at("ysize");
	ySize = ySizeStr.toDouble();
	ySizeIsDouble = ySize > 0 && ySize <= 1 && ySizeStr.contains('.');
	if (ySizeStr != "None" && !ySizeStr.isNumber()) {
		Utils::outMsg("ScreenChild::updateProps",
					  "ysize is not a number (" + ySizeStr + ")\n" + node->getPlace());
	}

	if (canCrop() && texture) {
		String cropStr = propValues.at("crop");
		if (cropStr && cropStr != "None") {
			char start = cropStr.front();
			char end = cropStr.back();
			if ((start == '(' && end == ')') ||
				(start == '[' && end == ']'))
			{
				int textureWidth = Utils::getTextureWidth(texture);
				int textureHeight = Utils::getTextureHeight(texture);

				if (cropStr == "(0.0, 0.0, 1.0, 1.0)") {
					crop = {0, 0, textureWidth, textureHeight};
				}else {
					cropStr = cropStr.substr(1, cropStr.size() - 2);
					std::vector<String> cropVec = cropStr.split(", ");
					if (cropVec.size() == 4) {
						double x = cropVec[0].toDouble();
						double y = cropVec[1].toDouble();
						double w = cropVec[2].toDouble();
						double h = cropVec[3].toDouble();

						if ((x > 0 && x < 1) || cropVec[0] == "1.0") x *= textureWidth;
						if ((y > 0 && y < 1) || cropVec[1] == "1.0") y *= textureHeight;
						if ((w > 0 && w < 1) || cropVec[2] == "1.0") w *= textureWidth;
						if ((h > 0 && h < 1) || cropVec[3] == "1.0") h *= textureHeight;

						crop = {int(x), int(y), int(w), int(h)};
					}else {
						Utils::outMsg("ScreenChild::updateProps", String() +
									  "В свойстве crop ожидался список из 4-х значений, получено " + cropVec.size() + "\n"
																													  "crop: <" + start + cropStr + end + ">\n" +
									  node->getPlace());
					}
				}
			}else {
				Utils::outMsg("ScreenChild::updateProps", String() +
							  "В свойстве crop ожидался список из 4-х значений\n"
							  "crop: <" + cropStr + ">\n" +
							  node->getPlace());
			}
		}
	}

	const String &rotateStr = propValues.at("rotate");
	rotate = rotateStr.toDouble();
	if (rotateStr != "None" && !rotateStr.isNumber()) {
		Utils::outMsg("ScreenChild::updateProps",
					  "rotate is not a number (" + rotateStr + ")\n" + node->getPlace());
	}

	const String &alphaStr = propValues.at("alpha");
	alpha = alphaStr.toDouble();
	if (alphaStr != "None" && !alphaStr.isNumber()) {
		Utils::outMsg("ScreenChild::updateProps",
					  "alpha is not a number (" + alphaStr + ")\n" + node->getPlace());
	}
}

void ScreenChild::updateSize() {
	if (xSizeIsDouble) xSize *= GV::width;
	if (ySizeIsDouble) ySize *= GV::height;

	setSize(xSize, ySize);

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
		int x = xPos - xAnchor;
		setX(x);
	}

	if (!inVBox) {
		if (yPosIsDouble) yPos *= parent->getHeight();
		if (yAnchorIsDouble) yAnchor *= getHeight();
		int y = yPos - yAnchor;
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
