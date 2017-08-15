#include "screen_textbutton.h"

#include "media/py_utils.h"
#include "utils/utils.h"

ScreenTextButton::ScreenTextButton(Node* node): ScreenText(node) {
	auto onClick = [this](DisplayObject*) {
		String action = this->node->getPropCode("action").pyExpr;
		if (action) {
			PyUtils::exec("CPP_EMBED: screen_textbutton.cpp", __LINE__, "exec_funcs(" + action + ")");
		}
	};
	btnRect.init(this, onClick);

	setProp("ground", node->getPropCode("ground"));
	setProp("hover", node->getPropCode("hover"));
}

void ScreenTextButton::calculateProps() {
	ScreenText::calculateProps();

	if (btnRect.mouseDown) {
		if (isModal()) {
			btnRect.onClick();
		}
	}
}
void ScreenTextButton::updateTexture() {
	const String &newGround = propValues.at("ground");
	const String &newHover = propValues.at("hover");

	if (newHover) {
		hover = newHover;
	}else
	if (ground != newGround) {
		hover = PyUtils::exec("CPP_EMBED: screen_textbutton.cpp", __LINE__, "im.MatrixColor('" + newGround + "', im.matrix.contrast(1.5))", true);
	}
	ground = newGround;

	const String &path = !btnRect.mouseOvered ? ground : hover;
	texture = Utils::getTexture(path);
}

void ScreenTextButton::updateSize() {
	if (xSize <= 0) xSize = Utils::getTextureWidth(texture);
	if (ySize <= 0) ySize = Utils::getTextureHeight(texture);

	ScreenText::updateSize();
}
