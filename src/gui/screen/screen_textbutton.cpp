#include "screen_textbutton.h"

#include "media/music.h"
#include "media/py_utils.h"


ScreenTextButton::ScreenTextButton(Node* node):
	ScreenText(node)
{
	auto onLeftClick = [this](DisplayObject*) {
		const NodeProp activateSound = this->node->getPropCode("activate_sound");
		if (activateSound.pyExpr) {
			Music::play("button_click " + activateSound.pyExpr,
						this->getFileName(), activateSound.numLine);
		}else if (activateSound.styleName) {
			const String sound = PyUtils::exec(this->getFileName(), this->getNumLine(),
								  "style." + activateSound.styleName + ".activete_sound",
								  true);
			if (sound != "None") {
				Music::play("button_click '" + sound + "'",
							this->getFileName(), this->getNumLine());
			}
		}

		const NodeProp action = this->node->getPropCode("action");
		if (action.pyExpr) {
			PyUtils::exec(this->getFileName(), action.numLine,
						  "exec_funcs(" + action.pyExpr + ")");
		}else if (action.styleName) {
			PyUtils::exec(this->getFileName(), this->getNumLine(),
						  "exec_funcs(style." + action.styleName + "." + action.propName + ")");
		}
	};
	auto onRightClick = [this](DisplayObject*) {
		const NodeProp alternate = this->node->getPropCode("alternate");
		if (alternate.pyExpr) {
			PyUtils::exec(this->getFileName(), alternate.numLine,
						  "exec_funcs(" + alternate.pyExpr + ")");
		}else if (alternate.styleName) {
			PyUtils::exec(this->getFileName(), this->getNumLine(),
						  "exec_funcs(style." + alternate.styleName + ".alternate)");
		}
	};
	btnRect.init(this, onLeftClick, onRightClick);

	setProp(ScreenProp::GROUND, node->getPropCode("ground"));
	setProp(ScreenProp::HOVER, node->getPropCode("hover"));
	setProp(ScreenProp::MOUSE, node->getPropCode("mouse"));

	preparationToUpdateCalcProps();
}

void ScreenTextButton::calculateProps() {
	ScreenText::calculateProps();

	if (propWasChanged[ScreenProp::MOUSE]) {
		propWasChanged[ScreenProp::MOUSE] = false;

		const String &mouse = propValues[ScreenProp::MOUSE];
		btnRect.buttonMode = mouse == "True";
	}

	if (btnRect.mouseOvered) {
		if (!prevMouseOver) {
			const NodeProp hoverSound = node->getPropCode("hover_sound");
			if (hoverSound.pyExpr) {
				Music::play("button_hover " + hoverSound.pyExpr, getFileName(), hoverSound.numLine);
			}else if (hoverSound.styleName) {
				const String sound = PyUtils::exec(this->getFileName(), this->getNumLine(),
									  "style." + hoverSound.styleName + ".hover_sound",
									  true);
				if (sound != "None") {
					Music::play("button_hover '" + sound + "'",
								this->getFileName(), this->getNumLine());
				}
			}

			const NodeProp hovered = node->getPropCode("hovered");
			if (hovered.pyExpr) {
				PyUtils::exec(getFileName(), hovered.numLine, "exec_funcs(" + hovered.pyExpr + ")");
			}else if (hovered.styleName) {
				PyUtils::exec(this->getFileName(), this->getNumLine(),
							  "exec_funcs(style." + hovered.styleName + ".hovered)");
			}
		}
	}else {
		if (prevMouseOver) {
			const NodeProp unhovered = node->getPropCode("unhovered");
			if (unhovered.pyExpr) {
				PyUtils::exec(getFileName(), unhovered.numLine, "exec_funcs(" + unhovered.pyExpr + ")");
			}else if (unhovered.styleName) {
				PyUtils::exec(this->getFileName(), this->getNumLine(),
							  "exec_funcs(style." + unhovered.styleName + ".unhovered)");
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
	if (propWasChanged[ScreenProp::GROUND] || propWasChanged[ScreenProp::HOVER] || prevMouseOver != btnRect.mouseOvered) {
		propWasChanged[ScreenProp::GROUND] = false;
		propWasChanged[ScreenProp::HOVER] = false;

		const String &newGround = propValues[ScreenProp::GROUND];
		const String &newHover = propValues[ScreenProp::HOVER];

		if (newHover) {
			hover = newHover;
		}else
			if (ground != newGround) {
				hover = PyUtils::exec("CPP_EMBED: screen_textbutton.cpp", __LINE__, "im.MatrixColor(r'" + newGround + "', im.matrix.contrast(1.5))", true);
			}
		ground = newGround;

		const String &path = !btnRect.mouseOvered ? ground : hover;
		texture = Utils::getTexture(path);

		if (xSizeIsTextureWidth)  xSize = Utils::getTextureWidth(texture);
		if (ySizeIsTextureHeight) ySize = Utils::getTextureHeight(texture);
		ScreenText::updateSize();
	}
}

void ScreenTextButton::updateSize() {
	if (xSize <= 0) {
		xSizeIsTextureWidth = true;
		xSize = Utils::getTextureWidth(texture);
	}else {
		xSizeIsTextureWidth = false;
	}

	if (ySize <= 0) {
		ySizeIsTextureHeight = true;
		ySize = Utils::getTextureHeight(texture);
	}else {
		ySizeIsTextureHeight = false;
	}

	ScreenText::updateSize();
}
