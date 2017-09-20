#include "screen_child.h"

#include <algorithm>

#include "gv.h"

#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "parser/node.h"
#include "media/py_utils.h"


std::vector<ScreenChild*> ScreenChild::screenObjects;

std::vector<String> ScreenChild::propNames;
void ScreenChild::setPropNames() {
	propNames.clear();
	for (size_t i = 0; i < COUNT_PROPS; ++i) {
		propNames.push_back(String(i));
	}
}

ScreenChild::ScreenChild(Node *node, ScreenChild *screenParent) {
	screenObjects.push_back(this);

	this->node = node;

	this->screenParent = screenParent;
	_isFakeContainer = screenParent && screenParent != this;

	const String &type = getType();
	_canCrop = type == "image" || type == "button" || type == "textbutton" || type == "imagemap";

	bool drawable = type != "if" && type != "elif" && type != "else" &&
					type != "for" && type != "while" &&
					type != "continue" && type != "break" &&
					type != "$" && type != "python" && type != "key";

	clearProps();

	if (drawable) {
		setProp(ScreenProp::X_ANCHOR, node->getPropCode("xanchor", "anchor", "[0]"));
		setProp(ScreenProp::Y_ANCHOR, node->getPropCode("yanchor", "anchor", "[1]"));
		setProp(ScreenProp::X_POS, node->getPropCode("xpos", "pos", "[0]"));
		setProp(ScreenProp::Y_POS, node->getPropCode("ypos", "pos", "[1]"));
		setProp(ScreenProp::X_ALIGN, node->getPropCode("xalign", "align", "[0]"));
		setProp(ScreenProp::Y_ALIGN, node->getPropCode("yalign", "align", "[1]"));

		setProp(ScreenProp::X_SIZE, node->getPropCode("xsize", "size", "[0]"));
		setProp(ScreenProp::Y_SIZE, node->getPropCode("ysize", "size", "[1]"));

		setProp(ScreenProp::CROP, node->getPropCode("crop"));
		setProp(ScreenProp::ROTATE, node->getPropCode("rotate"));
		setProp(ScreenProp::ALPHA, node->getPropCode("alpha"));
	}

	preparationToUpdateCalcProps();
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

void ScreenChild::clearProps() {
	props.clear();
	props.resize(COUNT_PROPS);

	propValues.clear();
	propValues.resize(COUNT_PROPS);

	propWasChanged.clear();
	propWasChanged.resize(COUNT_PROPS);
}
void ScreenChild::setProp(const ScreenProp prop, const NodeProp &nodeProp) {
	props[prop] = nodeProp;
}
void ScreenChild::preparationToUpdateCalcProps() {
	codeForCalcProps = "calc_object = {\n";

	for (char &v : propWasChanged) {
		v = true;
	}

	bool empty = true;
	propIndeces.clear();

	for (size_t i = 0; i < COUNT_PROPS; ++i) {
		const NodeProp &nodeProp = props.at(i);
		const String &propExpr = nodeProp.pyExpr;

		if (propExpr) {
			if (i == ScreenProp::X_ALIGN) usingXAlign = true;
			if (i == ScreenProp::Y_ALIGN) usingYAlign = true;

			if (PyUtils::isConstExpr(propExpr)) {
				propValues[i] = PyUtils::exec(getFileName(), nodeProp.numLine, propExpr, true);
			}else {
				empty = false;
				propIndeces.push_back(ScreenProp(i));

				const String &propName = propNames.at(i);
				codeForCalcProps += "    '" + propName + "': " + propExpr + ",\n";
			}
		}else if (nodeProp.styleName) {
			propValues[i] = Style::getProp(nodeProp.styleName, nodeProp.propName);
		}
	}

	if (!empty) {
		codeForCalcProps.erase(codeForCalcProps.size() - 2);
		codeForCalcProps += "\n"
							"}\n";
	}else {
		codeForCalcProps.clear();
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


	if (isFakeContainer()) return;

	if (codeForCalcProps) {
		{
			std::lock_guard<std::mutex> g(PyUtils::pyExecMutex);
			GV::pyUtils->pythonGlobal["calc_object"] = py::object();//... = None
		}
		PyUtils::exec("EMBED_CPP: ScreenChild.cpp", __LINE__, codeForCalcProps);

		bool ok = false;
		{
			std::lock_guard<std::mutex> g(PyUtils::pyExecMutex);

			const char *errorDesc = "Ошибка при извлечении calc_object";
			try {
				py::object obj = GV::pyUtils->pythonGlobal["calc_object"];

				//ok (try)
				if (!obj.is_none()) {
					errorDesc = "Ошибка при извлечении свойств из calc_object";
					for (ScreenProp i : propIndeces) {
						const String &propName = propNames.at(i);
						py::object res = obj[propName.c_str()];

						const String t = String(py::extract<const std::string>(py::str(res)));
						propWasChanged[i] = propWasChanged[i] || propValues.at(i) != t;
						propValues[i] = t;
					}
					ok = true;
				}
			}catch (py::error_already_set) {
				Utils::outMsg("EMBED_CPP: ScreenChild::calculateProps", errorDesc);
				PyUtils::errorProcessing("EMBED_CPP: ScreenChild::calculateProps\n" + codeForCalcProps);
			}
		}

		//some error (except)
		if (!ok) {
			for (ScreenProp i : propIndeces) {
				const NodeProp &nodeProp = props.at(i);
				const String &propExpr = nodeProp.pyExpr;

				const String t = PyUtils::exec(getFileName(), nodeProp.numLine, propExpr, true);
				propWasChanged[i] = propWasChanged[i] || propValues.at(i) != t;
				propValues[i] = t;
			}
		}
	}

	bool textureChanged = propWasChanged[ScreenProp::IMAGE_PATH] ||
						  propWasChanged[ScreenProp::GROUND] || propWasChanged[ScreenProp::HOVER];
	updateTexture();


	if (!needUpdateFields) return;


	static const String None = "None";

	if (usingXAlign) {
		if (propWasChanged[ScreenProp::X_ALIGN]) {
			propWasChanged[ScreenProp::X_ALIGN] = false;

			const String &xAlignStr = propValues.at(ScreenProp::X_ALIGN);
			if (xAlignStr.isNumber()) {
				preXAnchor = xPos = xAlignStr.toDouble();
				xAnchorIsDouble = xPosIsDouble = preXAnchor > 0 && preXAnchor <= 1 && xAlignStr.contains('.');
			}else if (xAlignStr != None) {
				Utils::outMsg("ScreenChild::updateProps",
							  "xalign is not a number (" + xAlignStr + ")\n" + node->getPlace());
			}
		}
	}else {
		if (propWasChanged[ScreenProp::X_ANCHOR]) {
			propWasChanged[ScreenProp::X_ANCHOR] = false;

			const String &xAnchorStr = propValues.at(ScreenProp::X_ANCHOR);
			preXAnchor = xAnchorStr.toDouble();
			xAnchorIsDouble = preXAnchor > 0 && preXAnchor <= 1 && xAnchorStr.contains('.');
			if (xAnchorStr != None && !xAnchorStr.isNumber()) {
				Utils::outMsg("ScreenChild::updateProps",
							  "xanchor is not a number (" + xAnchorStr + ")\n" + node->getPlace());
			}
		}

		if (!inHBox) {
			if (propWasChanged[ScreenProp::X_POS]) {
				propWasChanged[ScreenProp::X_POS] = false;

				const String &xPosStr = propValues.at(ScreenProp::X_POS);
				xPos = xPosStr.toDouble();
				xPosIsDouble = xPos > 0 && xPos <= 1 && xPosStr.contains('.');
				if (xPosStr != None && !xPosStr.isNumber()) {
					Utils::outMsg("ScreenChild::updateProps",
								  "xpos is not a number (" + xPosStr + ")\n" + node->getPlace());
				}
			}
		}
	}


	if (usingYAlign) {
		if (propWasChanged[ScreenProp::Y_ALIGN]) {
			propWasChanged[ScreenProp::Y_ALIGN] = false;

			const String &yAlignStr = propValues.at(ScreenProp::Y_ALIGN);
			if (yAlignStr.isNumber()) {
				preYAnchor = yPos = yAlignStr.toDouble();
				yAnchorIsDouble = yPosIsDouble = preYAnchor > 0 && preYAnchor <= 1 && yAlignStr.contains('.');
			}else if (yAlignStr && yAlignStr != None) {
				Utils::outMsg("ScreenChild::updateProps",
							  "yalign is not a number (" + yAlignStr + ")\n" + node->getPlace());
			}
		}
	}else {
		if (propWasChanged[ScreenProp::Y_ANCHOR]) {
			propWasChanged[ScreenProp::Y_ANCHOR] = false;

			const String &yAnchorStr = propValues.at(ScreenProp::Y_ANCHOR);
			preYAnchor = yAnchorStr.toDouble();
			yAnchorIsDouble = preYAnchor > 0 && preYAnchor <= 1 && yAnchorStr.contains('.');
			if (yAnchorStr != None && !yAnchorStr.isNumber()) {
				Utils::outMsg("ScreenChild::updateProps",
							  "yanchor is not a number (" + yAnchorStr + ")\n" + node->getPlace());
			}
		}

		if (!inVBox) {
			if (propWasChanged[ScreenProp::Y_POS]) {
				propWasChanged[ScreenProp::Y_POS] = false;

				const String &yPosStr = propValues.at(ScreenProp::Y_POS);
				yPos = yPosStr.toDouble();
				yPosIsDouble = yPos > 0 && yPos <= 1 && yPosStr.contains('.');
				if (yPosStr != None && !yPosStr.isNumber()) {
					Utils::outMsg("ScreenChild::updateProps",
								  "ypos is not a number (" + yPosStr + ")\n" + node->getPlace());
				}
			}
		}
	}

	bool xSizeChanged = false;
	if (propWasChanged[ScreenProp::X_SIZE]) {
		propWasChanged[ScreenProp::X_SIZE] = false;
		xSizeChanged = true;

		const String &xSizeStr = propValues.at(ScreenProp::X_SIZE);
		xSize = xSizeStr.toDouble();
		xSizeIsDouble = xSize > 0 && xSize <= 1 && xSizeStr.contains('.');
		if (xSizeStr != None && !xSizeStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps",
						  "xsize is not a number (" + xSizeStr + ")\n" + node->getPlace());
		}
	}

	bool ySizeChanged = false;
	if (propWasChanged[ScreenProp::Y_SIZE]) {
		propWasChanged[ScreenProp::Y_SIZE] = false;
		ySizeChanged = true;

		const String &ySizeStr = propValues.at(ScreenProp::Y_SIZE);
		ySize = ySizeStr.toDouble();
		ySizeIsDouble = ySize > 0 && ySize <= 1 && ySizeStr.contains('.');
		if (ySizeStr != None && !ySizeStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps",
						  "ysize is not a number (" + ySizeStr + ")\n" + node->getPlace());
		}
	}

	if (canCrop() && texture &&
		(xSizeChanged || ySizeChanged || textureChanged ||
		 propWasChanged[ScreenProp::CROP])
	) {
		propWasChanged[ScreenProp::CROP] = false;

		String cropStr = propValues.at(ScreenProp::CROP);
		if (cropStr && cropStr != None) {
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
									  "В свойстве crop ожидался список из 4-х значений, получено " + cropVec.size() + "\n" +
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

	if (propWasChanged[ScreenProp::ROTATE]) {
		propWasChanged[ScreenProp::ROTATE] = false;

		const String &rotateStr = propValues.at(ScreenProp::ROTATE);
		rotate = rotateStr.toDouble();
		if (rotateStr != None && !rotateStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps",
						  "rotate is not a number (" + rotateStr + ")\n" + node->getPlace());
		}
	}

	if (propWasChanged[ScreenProp::ALPHA]) {
		propWasChanged[ScreenProp::ALPHA] = false;

		const String &alphaStr = propValues.at(ScreenProp::ALPHA);
		alpha = alphaStr.toDouble();
		if (alphaStr != None && !alphaStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps",
						  "alpha is not a number (" + alphaStr + ")\n" + node->getPlace());
		}
	}
}

void ScreenChild::updateSize() {
	setSize(xSize * (xSizeIsDouble ? GV::width : 1), ySize * (ySizeIsDouble ? GV::height : 1));

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

	xAnchor = preXAnchor * (xAnchorIsDouble ? getWidth() : 1);
	yAnchor = preYAnchor * (yAnchorIsDouble ? getHeight() : 1);

	if (!inHBox) {
		int endXPos = xPos * (xPosIsDouble ? parent->getWidth() : 1);
		int x = endXPos - xAnchor;
		setX(x);
	}

	if (!inVBox) {
		int endYPos = yPos * (yPosIsDouble ? parent->getHeight() : 1);
		int y = endYPos - yAnchor;
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
