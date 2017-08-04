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

	setProp("xsize", node->getPropCode("xsize", "xysize", "[0]"));
	setProp("ysize", node->getPropCode("ysize", "xysize", "[1]"));

	setProp("crop", node->getPropCode("crop"));
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
}

void ScreenChild::setProp(const String &propName, const NodeProp &nodeProp,
						  bool priority) {
	if (nodeProp.pyExpr) {
		(priority ? propCodesPriority : propCodes)[propName] = nodeProp.pyExpr;
		propNumLines[propName] = nodeProp.numLine;
	}else {
		propInStyle[propName] = nodeProp;
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



	for (auto i : propInStyle) {
		const String &propName = i.first;
		const NodeProp &nodeProp = i.second;

		propValues[propName] = Style::getProp(nodeProp.styleName, nodeProp.propName);
	}


	auto updateProps = [this](std::map<String, String> propCodes) {
		{
			std::lock_guard<std::mutex> g(PyUtils::pyExecGuard);
			GV::pyUtils->pythonGlobal["calc_object"] = py::object();//... = None
		}

		String code =
				"calc_object = {\n";
		for (auto i : propCodes) {
			const String &propName = i.first;
			const String &propExpr = i.second;

			code += "    '" + propName + "': " + propExpr + ",\n";
		}
		if (propCodes.size()) {
			code.erase(code.size() - 2);
		}
		code +=     "\n"
					"}\n";
		if (propCodes.size()) {
			PyUtils::exec("EMBED_CPP: ScreenChild.cpp", __LINE__, code);
		}


		bool ok = false;
		{
			std::lock_guard<std::mutex> g(PyUtils::pyExecGuard);

			String errorDesc = "Ошибка при извлечении calc_object";
			try {
				py::object obj = GV::pyUtils->pythonGlobal["calc_object"];

				//ok (try)
				if (!obj.is_none()) {
					errorDesc = "Ошибка при извлечении свойств из calc_object";
					for (auto i : propCodes) {
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
			for (auto i : propCodes) {
				const String &propName = i.first;
				const String &propExpr = i.second;
				const String &fileName = node->getFileName();
				size_t numLine = propNumLines[propName];

				propValues[propName] = PyUtils::exec(fileName, numLine, propExpr, true);
			}
		}
	};
	updateProps(propCodesPriority);
	afterPriorityUpdate();
	updateProps(propCodes);


	if (!needUpdateFields) return;


	if (!inHBox) {
		String xAnchorStr = propValues["xanchor"];
		xAnchor = xAnchorStr.toDouble();
		xAnchorIsDouble = xAnchorStr.contains('.') && xAnchor > 0 && xAnchor <= 1;
		if (xAnchorStr != "None" && !xAnchorStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps",
						  "xanchor is not a number (" + xAnchorStr + ")\n" + node->getPlace());
		}

		String xPosStr = propValues["xpos"];
		xPos = xPosStr.toDouble();
		xPosIsDouble = xPosStr.contains('.') && xPos > 0 && xPos <= 1;
		if (xPosStr != "None" && !xPosStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps",
						  "xpos is not a number (" + xPosStr + ")\n" + node->getPlace());
		}

		String xAlignStr = propValues["xalign"];
		if (xAlignStr.isNumber()) {
			xAnchor = xPos = xAlignStr.toDouble();
			xAnchorIsDouble = xPosIsDouble = xAlignStr.contains('.') && xAnchor > 0 && xAnchor <= 1;
		}else if (xAlignStr != "None") {
			Utils::outMsg("ScreenChild::updateProps",
						  "xalign is not a number (" + xAlignStr + ")\n" + node->getPlace());
		}
	}
	if (!inVBox) {
		String yAnchorStr = propValues["yanchor"];
		yAnchor = yAnchorStr.toDouble();
		yAnchorIsDouble = yAnchorStr.contains('.') && yAnchor > 0 && yAnchor <= 1;
		if (yAnchorStr != "None" && !yAnchorStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps",
						  "yanchor is not a number (" + yAnchorStr + ")\n" + node->getPlace());
		}

		String yPosStr = propValues["ypos"];
		yPos = yPosStr.toDouble();
		yPosIsDouble = yPosStr.contains('.') && yPos > 0 && yPos <= 1;
		if (yPosStr != "None" && !yPosStr.isNumber()) {
			Utils::outMsg("ScreenChild::updateProps",
						  "ypos is not a number (" + yPosStr + ")\n" + node->getPlace());
		}

		String yAlignStr = propValues["yalign"];
		if (yAlignStr.isNumber()) {
			yAnchor = yPos = yAlignStr.toDouble();
			yAnchorIsDouble = yPosIsDouble = yAlignStr.contains('.') && yAnchor > 0 && yAnchor <= 1;
		}else if (yAlignStr && yAlignStr != "None") {
			Utils::outMsg("ScreenChild::updateProps",
						  "yalign is not a number (" + yAlignStr + ")\n" + node->getPlace());
		}
	}



	String xSizeStr = propValues["xsize"];
	xSize = xSizeStr.toDouble();
	xSizeIsDouble = xSizeStr.contains('.') && xSize > 0 && xSize <= 1;
	if (xSizeStr != "None" && !xSizeStr.isNumber()) {
		Utils::outMsg("ScreenChild::updateProps",
					  "xsize is not a number (" + xSizeStr + ")\n" + node->getPlace());
	}

	String ySizeStr = propValues["ysize"];
	ySize = ySizeStr.toDouble();
	ySizeIsDouble = ySizeStr.contains('.') && ySize > 0 && ySize <= 1;
	if (ySizeStr != "None" && !ySizeStr.isNumber()) {
		Utils::outMsg("ScreenChild::updateProps",
					  "ysize is not a number (" + ySizeStr + ")\n" + node->getPlace());
	}

	if (canCrop() && texture) {
		String cropStr = propValues["crop"];
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
								  "crop: <" + start + cropStr + end + ">\n" +
								   node->getPlace());
				}
			}else {
				Utils::outMsg("ScreenChild::updateProps", String() +
							  "В свойстве crop ожидался список из 4-х значений\n"
							  "crop: <" + cropStr + ">\n" +
							  node->getPlace());
			}
		}
	}

	String alphaStr = propValues["alpha"];
	alpha = alphaStr.toDouble();
	if (alphaStr != "None" && !alphaStr.isNumber()) {
		Utils::outMsg("ScreenChild::updateProps",
					  "alpha is not a number (" + alphaStr + ")\n" + node->getPlace());
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
