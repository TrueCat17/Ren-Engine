#include "textbutton.h"

#include "style.h"

#include "media/music.h"
#include "media/image_manipulator.h"
#include "media/py_utils.h"


TextButton::TextButton(Node* node, Screen *screen):
	Text(node, screen)
{
	auto onLeftClick = [this](DisplayObject*) {
		const Node *activateSound = this->node->getProp("activate_sound");
		if (activateSound) {
			Music::play("button_click " + activateSound->params,
						activateSound->getFileName(), activateSound->getNumLine());
		}else {
			const Node *style = this->node->getProp("style");
			const String &styleName = style ? style->params : this->node->command;
			const std::string sound = PyUtils::getStr(Style::getProp(styleName, "activate_sound"));
			if (sound != "None") {
				Music::play("button_click '" + sound + "'",
							this->getFileName(), this->getNumLine());
			}
		}

		const Node *action = this->node->getProp("action");
		if (action) {
			PyUtils::exec(action->getFileName(), action->getNumLine(),
						  "exec_funcs(" + action->params + ")");
		}else {
			const Node *style = this->node->getProp("style");
			const String &styleName = style ? style->params : this->node->command;
			PyUtils::exec(this->getFileName(), this->getNumLine(),
						  "exec_funcs(style." + styleName + ".action)");
		}
	};
	auto onRightClick = [this](DisplayObject*) {
		const Node* alternate = this->node->getProp("alternate");
		if (alternate) {
			PyUtils::exec(alternate->getFileName(), alternate->getNumLine(),
						  "exec_funcs(" + alternate->params + ")");
		}else {
			const Node *style = this->node->getProp("style");
			const String &styleName = style ? style->params : this->node->command;
			PyUtils::exec(this->getFileName(), this->getNumLine(),
						  "exec_funcs(style." + styleName + ".alternate)");
		}
	};
	btnRect.init(this, onLeftClick, onRightClick);
}

void TextButton::updateSize() {
	Text::updateSize();

	if (xsize <= 0) {
		xsizeIsTextureWidth = true;
		xsize = surface ? surface->w : 0;
	}else {
		xsizeIsTextureWidth = false;
	}

	if (ysize <= 0) {
		ysizeIsTextureHeight = true;
		ysize = surface ? surface->h : 0;
	}else {
		ysizeIsTextureHeight = false;
	}
}

void TextButton::updateTexture(bool skipeError) {
	if (skipeError && !ground) return;

	if (!surface || !hover || prevGround != ground || prevHover != hover || prevMouseOver != btnRect.mouseOvered) {
		if (prevGround != ground && !hover) {
			hover = PyUtils::exec("CPP_EMBED: screen_textbutton.cpp", __LINE__, "im.MatrixColor(r'" + ground + "', im.matrix.contrast(1.5))", true);
		}
		prevGround = ground;
		prevHover = hover;

		const String &path = !btnRect.mouseOvered ? ground : hover;
		surface = ImageManipulator::getImage(path);

		if (xsizeIsTextureWidth)  xsize = surface ? surface->w : 0;
		if (ysizeIsTextureHeight) ysize = surface ? surface->h : 0;
		updateSize();
	}
}
