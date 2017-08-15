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

	setProp("text", NodeProp::initPyExpr(textExec ? textExec : "''", getNumLine()));
	setProp("font", node->getPropCode("font"));
	setProp("text_align", node->getPropCode("text_align"));
	setProp("text_valign", node->getPropCode("text_valign"));
	setProp("color", node->getPropCode("color"));
	setProp("text_size", node->getPropCode("text_size"));
}

void ScreenText::calculateProps() {
	ScreenChild::calculateProps();

	text = propValues.at("text");
	if (text) {
		font = propValues.at("font");
		textHAlign = propValues.at("text_align");
		textVAlign = propValues.at("text_valign");

		const String &colorStr = propValues.at("color");
		color = 0xFFFFFF;
		if (colorStr) {
			if (colorStr[0] == '#') {
				color = String(colorStr.substr(1)).toInt(16);
			}else {
				color = colorStr.toInt();
			}
		}

		size = propValues.at("text_size").toInt();
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
