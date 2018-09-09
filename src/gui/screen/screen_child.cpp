#include "screen_child.h"

#include <algorithm>
#include <set>

#include "gv.h"

#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "parser/node.h"
#include "media/py_utils.h"
#include "utils/utils.h"


std::vector<ScreenChild*> ScreenChild::screenObjects;

ScreenChild::ScreenChild(Node *node, ScreenContainer *screenParent, Screen *screen):
	screen(screen),
	screenParent(screenParent),
	node(node)
{
	screenObjects.push_back(this);

	_isFakeContainer = screenParent && screenParent != this;

	const String &type = getType();

	static const std::set<String> cropable = {
		"image", "button", "textbutton", "imagemap"
	};
	static const std::set<String> nodrawable = {
		"if", "elif", "else",
		"for", "while",
		"continue", "break",
		"$", "python", "key"
	};

	_canCrop = cropable.find(type) != cropable.end();

	clearProps();
	if (nodrawable.find(type) == nodrawable.end()) {
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
	for (char &v : propWasChanged) {
		v = true;
	}

	propIndeces.clear();

	codeForCalcProps.clear();
	for (size_t i = 0; i < COUNT_PROPS; ++i) {
		const NodeProp &nodeProp = props[i];
		const String &propExpr = nodeProp.pyExpr;

		if (propExpr) {
			if (i == ScreenProp::X_ALIGN) usingXAlign = true;
			if (i == ScreenProp::Y_ALIGN) usingYAlign = true;

			if (PyUtils::isConstExpr(propExpr)) {
				propValues[i] = PyUtils::execRetObj(getFileName(), nodeProp.numLine, propExpr);
			}else {
				propIndeces.push_back(ScreenProp(i));

				const String &propName = screenPropNames[i];
				codeForCalcProps += "_SL_" + propName + " = " + propExpr + "\n";
			}
		}else if (nodeProp.styleName) {
			propValues[i] = Style::getProp(nodeProp.styleName, nodeProp.propName);
		}
	}

	if (codeForCalcProps) {
		co = PyUtils::getCompileObject(codeForCalcProps, getFileName(), getNumLine(), true);
		PyErr_Clear();
	}else {
		co = nullptr;
	}
}

void ScreenChild::calculateProps() {
	enable = true;

	if (codeForCalcProps) {
		bool calculated = false;
		static py::dict dict;
		if (co) {
			std::lock_guard<std::mutex> g(PyUtils::pyExecMutex);
			if (!PyEval_EvalCode(co, GV::pyUtils->pythonGlobal.ptr(), dict.ptr())) {
				PyUtils::errorProcessing(codeForCalcProps);
			}else {
				calculated = true;
			}
		}

		bool ok = false;
		if (calculated) {
			std::lock_guard<std::mutex> g(PyUtils::pyExecMutex);
			try {
				for (ScreenProp i : propIndeces) {
					const py::object &res = dict[screenPropObjects[i]];

					propWasChanged[i] = propWasChanged[i] || propValues[i] != res;
					propValues[i] = res;
				}
				ok = true;
			}catch (py::error_already_set&) {
				Utils::outMsg("EMBED_CPP: ScreenChild::calculateProps", "Ошибка при извлечении свойств из calc_object");
				PyUtils::errorProcessing("EMBED_CPP: ScreenChild::calculateProps\n" + codeForCalcProps);
			}
		}

		//some error (except)
		if (!ok) {
			for (ScreenProp i : propIndeces) {
				const NodeProp &nodeProp = props[i];
				const String &propExpr = nodeProp.pyExpr;

				py::object res = PyUtils::execRetObj(getFileName(), nodeProp.numLine, propExpr);
				propWasChanged[i] = propWasChanged[i] || propValues[i] != res;
				propValues[i] = res;
			}
		}
	}


	if (needUpdateChildren) {
		for (ScreenChild *screenChild : screenChildren) {
			screenChild->calculateProps();
		}
	}
	if (isFakeContainer()) return;


	bool textureChanged = propWasChanged[ScreenProp::IMAGE_PATH] ||
						  propWasChanged[ScreenProp::GROUND] || propWasChanged[ScreenProp::HOVER];
	updateTexture();


	if (!needUpdateFields) return;

	if (usingXAlign) {
		if (propWasChanged[ScreenProp::X_ALIGN]) {
			propWasChanged[ScreenProp::X_ALIGN] = false;

			py::object &xAlignObj = propValues[ScreenProp::X_ALIGN];
			bool isInt = PyUtils::isInt(xAlignObj);
			bool isFloat = !isInt && PyUtils::isFloat(xAlignObj);
			if (isInt || isFloat) {
				preXAnchor = xPos = PyUtils::getDouble(xAlignObj, isFloat);
				xAnchorIsDouble = xPosIsDouble = isFloat && preXAnchor > 0 && preXAnchor <= 1;
			}else {
				preXAnchor = xPos = 0;
				Utils::outMsg("ScreenChild::calculateProps",
							  "xalign is not a number (" + PyUtils::getStr(xAlignObj) + ")\n" + node->getPlace());
			}
		}
	}else {
		if (propWasChanged[ScreenProp::X_ANCHOR]) {
			propWasChanged[ScreenProp::X_ANCHOR] = false;

			py::object &xAnchorObj = propValues[ScreenProp::X_ANCHOR];
			bool isInt = PyUtils::isInt(xAnchorObj);
			bool isFloat = !isInt && PyUtils::isFloat(xAnchorObj);

			if (isInt || isFloat) {
				preXAnchor = PyUtils::getDouble(xAnchorObj, isFloat);
				xAnchorIsDouble = isFloat && preXAnchor > 0 && preXAnchor <= 1;
			}else {
				preXAnchor = 0;
				Utils::outMsg("ScreenChild::calculateProps",
							  "xanchor is not a number (" + PyUtils::getStr(xAnchorObj) + ")\n" + node->getPlace());
			}
		}

		if (!inHBox) {
			if (propWasChanged[ScreenProp::X_POS]) {
				propWasChanged[ScreenProp::X_POS] = false;

				py::object &xPosObj = propValues[ScreenProp::X_POS];
				bool isInt = PyUtils::isInt(xPosObj);
				bool isFloat = !isInt && PyUtils::isFloat(xPosObj);

				if (isInt || isFloat) {
					xPos = PyUtils::getDouble(xPosObj, isFloat);
					xPosIsDouble = isFloat && xPos > 0 && xPos <= 1;
				}else {
					xPos = 0;
					Utils::outMsg("ScreenChild::calculateProps",
								  "xpos is not a number (" + PyUtils::getStr(xPosObj) + ")\n" + node->getPlace());
				}
			}
		}
	}


	if (usingYAlign) {
		if (propWasChanged[ScreenProp::Y_ALIGN]) {
			propWasChanged[ScreenProp::Y_ALIGN] = false;

			py::object &yAlignObj = propValues[ScreenProp::Y_ALIGN];
			bool isInt = PyUtils::isInt(yAlignObj);
			bool isFloat = !isInt && PyUtils::isFloat(yAlignObj);

			if (isInt || isFloat) {
				preYAnchor = yPos = PyUtils::getDouble(yAlignObj, isFloat);
				yAnchorIsDouble = yPosIsDouble = isFloat && preYAnchor > 0 && preYAnchor <= 1;
			}else {
				preYAnchor = yPos = 0;
				Utils::outMsg("ScreenChild::calculateProps",
							  "yalign is not a number (" + PyUtils::getStr(yAlignObj) + ")\n" + node->getPlace());
			}
		}
	}else {
		if (propWasChanged[ScreenProp::Y_ANCHOR]) {
			propWasChanged[ScreenProp::Y_ANCHOR] = false;

			py::object &yAnchorObj = propValues[ScreenProp::Y_ANCHOR];
			bool isInt = PyUtils::isInt(yAnchorObj);
			bool isFloat = !isInt && PyUtils::isFloat(yAnchorObj);

			if (isInt || isFloat) {
				preYAnchor = PyUtils::getDouble(yAnchorObj, isFloat);
				yAnchorIsDouble = isFloat && preYAnchor > 0 && preYAnchor <= 1;
			}else {
				preYAnchor = 0;
				Utils::outMsg("ScreenChild::calculateProps",
							  "yanchor is not a number (" + PyUtils::getStr(yAnchorObj) + ")\n" + node->getPlace());
			}
		}

		if (!inVBox) {
			if (propWasChanged[ScreenProp::Y_POS]) {
				propWasChanged[ScreenProp::Y_POS] = false;

				py::object &yPosObj = propValues[ScreenProp::Y_POS];
				bool isInt = PyUtils::isInt(yPosObj);
				bool isFloat = !isInt && PyUtils::isFloat(yPosObj);

				if (isInt || isFloat) {
					yPos = PyUtils::getDouble(yPosObj, isFloat);
					yPosIsDouble = isFloat && yPos > 0 && yPos <= 1;
				}else {
					yPos = 0;
					Utils::outMsg("ScreenChild::calculateProps",
								  "ypos is not a number (" + PyUtils::getStr(yPosObj) + ")\n" + node->getPlace());
				}
			}
		}
	}

	bool xSizeChanged = false;
	if (propWasChanged[ScreenProp::X_SIZE]) {
		propWasChanged[ScreenProp::X_SIZE] = false;
		xSizeChanged = true;

		py::object &xSizeObj = propValues[ScreenProp::X_SIZE];
		bool isInt = PyUtils::isInt(xSizeObj);
		bool isFloat = !isInt && PyUtils::isFloat(xSizeObj);

		if (isInt || isFloat) {
			xSize = PyUtils::getDouble(xSizeObj, isFloat);
			xSizeIsDouble = isFloat && xSize > 0 && xSize <= 1;
		}else {
			xSize = 0;
			xSizeIsDouble = false;
			Utils::outMsg("ScreenChild::calculateProps",
						  "xsize is not a number (" + PyUtils::getStr(xSizeObj) + ")\n" + node->getPlace());
		}
	}

	bool ySizeChanged = false;
	if (propWasChanged[ScreenProp::Y_SIZE]) {
		propWasChanged[ScreenProp::Y_SIZE] = false;
		ySizeChanged = true;

		py::object &ySizeObj = propValues[ScreenProp::Y_SIZE];
		bool isInt = PyUtils::isInt(ySizeObj);
		bool isFloat = !isInt && PyUtils::isFloat(ySizeObj);

		if (isInt || isFloat) {
			ySize = PyUtils::getDouble(ySizeObj, isFloat);
			ySizeIsDouble = isFloat && ySize > 0 && ySize <= 1;
		}else {
			ySize = 0;
			ySizeIsDouble = false;
			Utils::outMsg("ScreenChild::calculateProps",
						  "ysize is not a number (" + PyUtils::getStr(ySizeObj) + ")\n" + node->getPlace());
		}
	}

	if (canCrop() && surface &&
		(xSizeChanged || ySizeChanged || textureChanged ||
		 propWasChanged[ScreenProp::CROP])
	) {
		propWasChanged[ScreenProp::CROP] = false;

		py::object &cropObj = propValues[ScreenProp::CROP];

		if ((PyUtils::isTuple(cropObj) || PyUtils::isList(cropObj)) && Py_SIZE(cropObj.ptr()) == 4) {
			int textureWidth = surface->w;
			int textureHeight = surface->h;

			bool isInt[4];
			bool isFloat[4];
			for (size_t i = 0; i < 4; ++i) {
				isInt[i] = PyUtils::isInt(cropObj[i]);
				isFloat[i] = !isInt[i] && PyUtils::isFloat(cropObj[i]);
				if (!isInt[i] && !isFloat[i]) {
					Utils::outMsg("ScreenChild::calculateProps", String() +
								  "В свойстве crop ожидался список из 4-х значений типа int или float\n"
								  "crop: <" + PyUtils::getStr(cropObj) + ">\n" +
								  node->getPlace());
					cropObj[i] = double(i >= 2);//0.0 for 0,1; 1.0 for 2,3; crop -> {0.0, 0.0, 1.0, 1.0}
				}
			}

			double x = PyUtils::getDouble(cropObj[0], isFloat[0]);
			double y = PyUtils::getDouble(cropObj[1], isFloat[1]);
			double w = PyUtils::getDouble(cropObj[2], isFloat[2]);
			double h = PyUtils::getDouble(cropObj[3], isFloat[3]);

			if (isFloat[0] && x > 0 && x <= 1) x *= textureWidth;
			if (isFloat[1] && y > 0 && y <= 1) y *= textureHeight;
			if (isFloat[2] && w > 0 && w <= 1) w *= textureWidth;
			if (isFloat[3] && h > 0 && h <= 1) h *= textureHeight;

			crop = {int(x), int(y), int(w), int(h)};
		}else {
			Utils::outMsg("ScreenChild::calculateProps", String() +
						  "В свойстве crop ожидался список из 4-х значений типа int или float\n"
						  "crop: <" + PyUtils::getStr(cropObj) + ">\n" +
						  node->getPlace());
		}
	}

	if (propWasChanged[ScreenProp::ROTATE]) {
		propWasChanged[ScreenProp::ROTATE] = false;

		py::object &rotateObj = propValues[ScreenProp::ROTATE];
		bool isInt = PyUtils::isInt(rotateObj);
		bool isFloat = !isInt && PyUtils::isFloat(rotateObj);

		if (isInt || isFloat) {
			rotate = PyUtils::getDouble(rotateObj, isFloat);
		}else {
			rotate = 0;
			Utils::outMsg("ScreenChild::calculateProps",
						  "rotate is not a number (" + PyUtils::getStr(rotateObj) + ")\n" + node->getPlace());
		}
	}

	if (propWasChanged[ScreenProp::ALPHA]) {
		propWasChanged[ScreenProp::ALPHA] = false;

		py::object &alphaObj = propValues[ScreenProp::ALPHA];
		bool isInt = PyUtils::isInt(alphaObj);
		bool isFloat = !isInt && PyUtils::isFloat(alphaObj);

		if (isInt || isFloat) {
			alpha = PyUtils::getDouble(alphaObj, isFloat);
		}else {
			alpha = 0;
			Utils::outMsg("ScreenChild::calculateProps",
						  "alpha is not a number (" + PyUtils::getStr(alphaObj) + ")\n" + node->getPlace());
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
