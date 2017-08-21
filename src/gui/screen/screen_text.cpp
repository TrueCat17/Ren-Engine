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
	ScreenChild::updateSize();

	bool needUpdate = false;
	if (text || prevText) {
		if (tf->getMaxWidth() != getWidth()) {
			tf->setMaxWidth(getWidth());
			needUpdate = true;
		}
		if (tf->getMaxHeight() != getHeight()) {
			tf->setMaxHeight(getHeight());
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
			tf->setAlign(textHAlign, textVAlign);
		}
	}

	int w = getWidth();
	int h = getHeight();
	if (w <= 0) w = tf->getWidth();
	if (h <= 0) h = tf->getHeight();
	setSize(w, h);

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
