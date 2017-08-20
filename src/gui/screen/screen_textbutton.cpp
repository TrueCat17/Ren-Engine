#include "screen_textbutton.h"

#include "media/music.h"
#include "media/py_utils.h"


ScreenTextButton::ScreenTextButton(Node* node):
	ScreenText(node)
{
	auto onLeftClick = [this](DisplayObject*) {
		const NodeProp activateSound = this->node->getPropCode("activate_sound");
		const String &sound = activateSound.pyExpr;
		if (sound) {
			Music::play("button_click " + sound, this->getFileName(), activateSound.numLine);
		}

		const NodeProp action = this->node->getPropCode("action");
		const String &actionExpr = action.pyExpr;
		if (actionExpr) {
			PyUtils::exec(this->getFileName(), action.numLine, "exec_funcs(" + actionExpr + ")");
		}
	};
	auto onRightClick = [this](DisplayObject*) {
		const NodeProp alternate = this->node->getPropCode("alternate");
		const String &alternateExpr = alternate.pyExpr;
		if (alternateExpr) {
			PyUtils::exec(this->getFileName(), alternate.numLine, "exec_funcs(" + alternateExpr + ")");
		}
	};
	btnRect.init(this, onLeftClick, onRightClick);

	setProp("ground", node->getPropCode("ground"));
	setProp("hover", node->getPropCode("hover"));
	setProp("mouse", node->getPropCode("mouse"));
}

void ScreenTextButton::calculateProps() {
	ScreenText::calculateProps();

	const String &mouse = propValues.at("mouse");
	btnRect.buttonMode = mouse == "True";

	if (btnRect.mouseOvered) {
		if (!prevMouseOver) {
			const NodeProp hoverSound = node->getPropCode("hover_sound");
			const String &sound = hoverSound.pyExpr;
			if (sound) {
				Music::play("button_hover " + sound, getFileName(), hoverSound.numLine);
			}

			const NodeProp hovered = node->getPropCode("hovered");
			const String &hoveredExpr = hovered.pyExpr;
			if (hoveredExpr) {
				PyUtils::exec(getFileName(), hovered.numLine, "exec_funcs(" + hoveredExpr + ")");
			}
		}
	}else {
		if (prevMouseOver) {
			const NodeProp unhovered = node->getPropCode("unhovered");
			const String &unhoveredExpr = unhovered.pyExpr;
			if (unhoveredExpr) {
				PyUtils::exec(getFileName(), unhovered.numLine, "exec_funcs(" + unhoveredExpr + ")");
			}
		}
	}
	prevMouseOver = btnRect.mouseOvered;

	if (btnRect.mouseLeftDown && isModal()) {
		btnRect.onLeftClick();
	}
	if (btnRect.mouseRightDown && isModal()) {
		btnRect.onRightClick();
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
