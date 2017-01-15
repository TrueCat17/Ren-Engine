#include "screen_textbutton.h"

#include "media/py_utils.h"
#include "utils/utils.h"

ScreenTextButton::ScreenTextButton(Node* node): ScreenText(node) {
	auto onClick = [this](DisplayObject*) {
		String action = this->node->getPropCode("action");
		if (action) {
			PyUtils::exec("exec_funcs(" + action + ")");
		}
	};
	btnRect.init(this, onClick);
}

void ScreenTextButton::updateProps() {
	ScreenText::updateProps();

	String newBackground = node->getProp("background");
	String newHoverBackground = node->getProp("hover_background");

	if (newHoverBackground) {
		hoverBackground = newHoverBackground;
	}else
	if (background != newBackground) {
		hoverBackground = PyUtils::exec("im.MatrixColor('" + newBackground + "', im.matrix.contrast(1.5))", true);
	}
	background = newBackground;

	if (btnRect.mouseDown && isModal()) {
		btnRect.onClick();
	}
}

void ScreenTextButton::updateSize() {
	String path = !btnRect.mouseOvered ? background : hoverBackground;
	texture = Utils::getTexture(path);

	if (xSize <= 0) xSize = Utils::getTextureWidth(texture);
	if (ySize <= 0) ySize = Utils::getTextureHeight(texture);

	ScreenText::updateSize();
}
