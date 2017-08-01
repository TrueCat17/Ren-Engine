#include "screen_textbutton.h"

#include "media/py_utils.h"
#include "utils/utils.h"

ScreenTextButton::ScreenTextButton(Node* node): ScreenText(node) {
	auto onClick = [this](DisplayObject*) {
		String action = this->node->getPropCode("action");
		if (action) {
			PyUtils::exec("CPP_EMBED: screen_textbutton.cpp", __LINE__, "exec_funcs(" + action + ")");
		}
	};
	btnRect.init(this, onClick);
}

void ScreenTextButton::calculateProps() {
	String newGround = node->getProp("ground");
	String newHover = node->getProp("hover");

	if (newHover) {
		hover = newHover;
	}else
	if (ground != newGround) {
		hover = PyUtils::exec("CPP_EMBED: screen_textbutton.cpp", __LINE__, "im.MatrixColor('" + newGround + "', im.matrix.contrast(1.5))", true);
	}
	ground = newGround;

	String path = !btnRect.mouseOvered ? ground : hover;
	texture = Utils::getTexture(path);

	ScreenText::calculateProps();

	if (btnRect.mouseDown) {
		if (isModal()) {
			btnRect.onClick();
		}
	}
}

void ScreenTextButton::updateSize() {
	if (xSize <= 0) xSize = Utils::getTextureWidth(texture);
	if (ySize <= 0) ySize = Utils::getTextureHeight(texture);

	ScreenText::updateSize();
}
