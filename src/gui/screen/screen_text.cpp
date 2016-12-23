#include "screen_text.h"

#include "gv.h"

#include "gui/text.h"

#include "parser/node.h"
#include "utils/utils.h"

ScreenText::ScreenText(Node *node): ScreenContainer(node, this) {
	textExec = node->getFirstParam();

	tf = new Text(font, size);
	tf->wordWrap = true;
	addChild(tf);
}

void ScreenText::updateProps() {
	text = Utils::execPython(textExec, true);
	if (text) {
		font = node->getProp("font");
		if (!font) {
			font = Text::defaultFontName;
		}
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

	ScreenContainer::updateProps();
}

void ScreenText::update() {
	double w = xSize;
	if (w > 0 && w <= 1) w *= GV::width;
	double h = ySize;
	if (h > 0 && h <= 1) h *= GV::height;

	setSize(w, h);


	bool needUpdate = false;

	if (tf->getMaxWidth() != rect.w) {
		tf->setMaxWidth(rect.w);
		needUpdate = true;
	}
	if (tf->getMaxHeight() != rect.h) {
		tf->setMaxHeight(rect.h);
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
	}else
	if (text && (tf->getHAlign() != textHAlign || tf->getVAlign() != textVAlign)) {
		tf->setAlign(textHAlign, textVAlign);
	}

	ScreenContainer::update();
}
