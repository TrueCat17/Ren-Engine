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
}

void ScreenText::calculateProps() {
	text = PyUtils::exec(textExec, true);
	if (text) {
		font = node->getProp("font");
		textHAlign = node->getProp("text_align");
		textVAlign = node->getProp("text_valign");

		String colorStr = node->getProp("color");
		color = 0xFFFFFF;
		if (colorStr) {
			if (colorStr[0] == '#') {
				color = String(colorStr.substr(1)).toInt(16);
			}else {
				color = colorStr.toInt();
			}
		}

		size = node->getProp("size").toInt();
		if (!size) size = 20;
		if (size % 2) size += 1;
	}

	ScreenChild::calculateProps();
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

		if (text && (needUpdate || tf->getHAlign() != textHAlign || tf->getVAlign() != textVAlign)) {
			tf->setAlign(textHAlign, textVAlign);
		}
	}

	int w = getWidth();
	int h = getHeight();
	if (w <= 0) w = tf->getWidth();
	if (h <= 0) h = tf->getHeight();
	setSize(w, h);
}
