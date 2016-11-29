#include "screen_text.h"

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

		String colorStr = node->getProp("color");
		color = 0xFFFFFF;
		if (colorStr) {
			if (colorStr[0] == '#') {
				color = String(colorStr.substr(1)).toInt(16);
			}else {
				color = colorStr.toInt();
			}
		}

		String textAlign = node->getProp("text_align");
		tf->setAlign(textAlign);

		size = node->getProp("size").toInt();
		if (!size) size = 20;
	}

	ScreenContainer::updateProps();
}

double ScreenText::getDefaultWidth() const { return 0.2; }
double ScreenText::getDefaultHeight() const { return 0.1; }

void ScreenText::update() {
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
	}

	ScreenContainer::update();
}
