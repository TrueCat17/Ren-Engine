#include "screen_text.h"

#include "gv.h"

#include "gui/text.h"

#include "media/py_utils.h"
#include "parser/node.h"

ScreenText::ScreenText(Node *node):
	ScreenChild(node, this),
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
}

void ScreenText::calculateProps() {
	ScreenChild::calculateProps();

	text = propValues.at(ScreenProp::TEXT);
	if (text) {
		font = propValues.at(ScreenProp::FONT);
		textHAlign = propValues.at(ScreenProp::TEXT_ALIGN);
		textVAlign = propValues.at(ScreenProp::TEXT_VALIGN);

		const String &colorStr = propValues.at(ScreenProp::COLOR);
		color = 0xFFFFFF;
		if (colorStr) {
			if (colorStr[0] == '#') {
				color = String(colorStr.substr(1)).toInt(16);
			}else {
				color = colorStr.toInt();
			}
		}

		size = propValues.at(ScreenProp::TEXT_SIZE).toInt();
		if (!size) size = 20;
	}
}

void ScreenText::updateSize() {
	if (xSizeIsDouble) xSize *= GV::width;
	if (ySizeIsDouble) ySize *= GV::height;

	bool needUpdate = false;
	if (text || prevText) {
		if (tf->getMaxWidth() != xSize) {
			tf->setMaxWidth(xSize);
			needUpdate = true;
		}
		if (tf->getMaxHeight() != ySize) {
			tf->setMaxHeight(ySize);
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
					 xSize != prevXSize || ySize != prevYSize)) {
			int w = xSize <= 0 ? tf->getWidth() : xSize;
			int h = ySize <= 0 ? tf->getHeight() : ySize;
			tf->setSize(w, h);
			setSize(w, h);

			tf->setAlign(textHAlign, textVAlign);
		}
	}else {
		setSize(xSize, ySize);
	}

	prevXSize = xSize;
	prevYSize = ySize;
}

void ScreenText::updateGlobalPos() {
	int prevGlobalRotate = getGlobalRotate();
	ScreenChild::updateGlobalPos();

	if (tf && tf->getText() && prevGlobalRotate != getGlobalRotate()) {
		tf->setAlign(tf->getHAlign(), tf->getVAlign());
	}
}
