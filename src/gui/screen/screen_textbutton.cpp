#include "screen_textbutton.h"

#include "utils/utils.h"

ScreenTextButton::ScreenTextButton(Node* node): ScreenText(node) {
	auto onClick = [this](DisplayObject*) {
		const String action = this->node->getPropCode("action");
		if (action) {
			Utils::execPython("exec_funcs(" + action + ")");
		}
	};
	btnRect.init(this, onClick);
}

void ScreenTextButton::updateProps() {
	ScreenText::updateProps();

	background = node->getProp("background");
	hoverBackground = node->getProp("hover_background");

	if (!hoverBackground && background) {
		hoverBackground = Utils::execPython("im.MatrixColor('" + background + "', im.matrix.contrast(1.5))", true);
	}

	if (btnRect.mouseDown && isModal()) {
		btnRect.onClick();
	}
}

void ScreenTextButton::update() {
	const String &path = !btnRect.mouseOvered ? background : hoverBackground;
	texture = Utils::getTexture(path);

	ScreenText::update();
}
