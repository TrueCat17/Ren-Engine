#include "screen_text.h"

#include "gv.h"

#include "gui/text.h"

#include "media/py_utils.h"
#include "parser/node.h"
#include "utils/utils.h"

ScreenText::ScreenText(Node *node, Screen *screen):
	ScreenChild(node, this, screen),
	textExec(node->getFirstParam())
{
	tf = new Text();
	tf->wordWrap = true;
	addChild(tf);

	setProp(ScreenProp::TEXT, NodeProp::initPyExpr(textExec ? textExec : "''", getNumLine()));
	setProp(ScreenProp::FONT, node->getPropCode("font"));
	setProp(ScreenProp::TEXT_SIZE, node->getPropCode("text_size"));
	setProp(ScreenProp::TEXT_ALIGN, node->getPropCode("text_align"));
	setProp(ScreenProp::TEXT_VALIGN, node->getPropCode("text_valign"));
	setProp(ScreenProp::COLOR, node->getPropCode("color"));

	preparationToUpdateCalcProps();
}

void ScreenText::calculateProps() {
	ScreenChild::calculateProps();

	if (propWasChanged[ScreenProp::TEXT]) {
		propWasChanged[ScreenProp::TEXT] = false;
		text = PyUtils::getStr(propValues[ScreenProp::TEXT]);
	}

	if (text) {
		if (propWasChanged[ScreenProp::FONT]) {
			propWasChanged[ScreenProp::FONT] = false;
			font = PyUtils::getStr(propValues[ScreenProp::FONT]);
		}
		if (propWasChanged[ScreenProp::TEXT_ALIGN]) {
			propWasChanged[ScreenProp::TEXT_ALIGN] = false;
			textHAlign = PyUtils::getStr(propValues[ScreenProp::TEXT_ALIGN]);
		}
		if (propWasChanged[ScreenProp::TEXT_VALIGN]) {
			propWasChanged[ScreenProp::TEXT_VALIGN] = false;
			textVAlign = PyUtils::getStr(propValues[ScreenProp::TEXT_VALIGN]);
		}

		if (propWasChanged[ScreenProp::COLOR]) {
			propWasChanged[ScreenProp::COLOR] = false;

			const String &colorStr = PyUtils::getStr(propValues[ScreenProp::COLOR]);
			color = 0xFFFFFF;
			if (colorStr) {
				if (colorStr[0] == '#') {
					color = String(colorStr.substr(1)).toInt(16);
				}else {
					color = colorStr.toInt();
				}
			}
		}

		if (propWasChanged[ScreenProp::TEXT_SIZE]) {
			propWasChanged[ScreenProp::TEXT_SIZE] = false;

			py::object &sizeObj = propValues[ScreenProp::TEXT_SIZE];
			bool isInt = PyUtils::isInt(sizeObj);
			bool isFloat = !isInt && PyUtils::isFloat(sizeObj);

			if (isInt || isFloat) {
				size = PyUtils::getDouble(sizeObj, isFloat);
				if (!size) size = 20;
			}else {
				size = 20;
				Utils::outMsg("ScreenText::calculateProps",
							  "text_size is not a number (" + PyUtils::getStr(sizeObj) + ")");
			}
		}
	}
}

void ScreenText::updateSize() {
	int width = xSize * (xSizeIsDouble ? GV::width : 1);
	int height = ySize * (ySizeIsDouble ? GV::height : 1);

	bool needUpdate = false;
	if (text || prevText) {
		if (tf->getMaxWidth() != width) {
			tf->setMaxWidth(width);
			needUpdate = true;
		}
		if (tf->getMaxHeight() != height) {
			tf->setMaxHeight(height);
			needUpdate = true;
		}

		if (tf->getFontName() != font || tf->getFontSize() != size) {
			tf->setFont(font, size);
			needUpdate = true;
		}
		if (prevText != text || color != prevColor) {
			prevText = text;
			prevColor = color;
			needUpdate = true;
		}

		if (needUpdate) {
			tf->setText(text, color);
		}

		if (text && (needUpdate || tf->getHAlign() != textHAlign || tf->getVAlign() != textVAlign ||
					 width != prevWidth || height != prevHeight)) {
			int w = width  <= 0 ? tf->getWidth()  : width;
			int h = height <= 0 ? tf->getHeight() : height;
			tf->setSize(w, h);
			setSize(w, h);

			tf->setAlign(textHAlign, textVAlign);
		}
	}else {
		setSize(width, height);
	}

	prevWidth = width;
	prevHeight = height;
}

void ScreenText::updateGlobalPos() {
	int prevGlobalRotate = getGlobalRotate();
	ScreenChild::updateGlobalPos();

	if (tf && tf->getText() && prevGlobalRotate != getGlobalRotate()) {
		tf->setAlign(tf->getHAlign(), tf->getVAlign());
	}
}
