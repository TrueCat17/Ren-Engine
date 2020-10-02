#include "textbutton.h"

#include "style.h"

#include "media/music.h"
#include "media/image_manipulator.h"
#include "media/py_utils.h"

#include "utils/string.h"
#include "utils/utils.h"


TextButton::TextButton(Node* node, Screen *screen):
	Text(node, screen)
{
	for (Node *child : node->children) {
		if (child->command == "ground") {
			groundIsStd = false;
		}else
		if (child->command == "hover") {
			hoverIsStd = false;
		}
	}

	auto onLeftClick = [this](DisplayObject*) {
		const Node *activateSound = this->node->getProp("activate_sound");
		if (activateSound) {
			Music::play("button_click " + activateSound->params,
						activateSound->getFileName(), activateSound->getNumLine());
		}else {
			const Node *style = this->node->getProp("style");
			const std::string &styleName = style ? style->params : this->node->command;
			PyObject *activateSoundObj = Style::getProp(styleName, "activate_sound");

			if (PyString_CheckExact(activateSoundObj)) {
				const char *sound = PyString_AS_STRING(activateSoundObj);
				Music::play("button_click '" + std::string(sound) + "'",
							this->getFileName(), this->getNumLine());
			}else if (activateSoundObj != Py_None) {
				std::string type = activateSoundObj->ob_type->tp_name;
				Utils::outMsg("TextButton::onLeftClick",
							  "In style." + styleName + ".activate_sound expected type str, got " + type);
			}
		}

		const Node *action = this->node->getProp("action");
		if (action) {
			PyUtils::exec(action->getFileName(), action->getNumLine(),
						  "exec_funcs(" + action->params + ")");
		}else {
			const Node *style = this->node->getProp("style");
			const std::string &styleName = style ? style->params : this->node->command;
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
			const std::string &styleName = style ? style->params : this->node->command;
			PyUtils::exec(this->getFileName(), this->getNumLine(),
						  "exec_funcs(style." + styleName + ".alternate)");
		}
	};
	btnRect.init(this, onLeftClick, onRightClick);
}

void TextButton::updateRect(bool) {
	Child::updateRect(false);
	Text::updateRect();

	xsize = std::max(xsize, 0.0);
	ysize = std::max(ysize, 0.0);
}

void TextButton::updateTexture(bool skipError) {
	if (skipError && ground.empty()) return;

	if (!surface || hover.empty() || prevGround != ground || prevHover != hover || prevMouseOver != btnRect.mouseOvered) {
		if (prevGround != ground && (hoverIsStd || hover.empty())) {
			std::string toConvert;

			if (!groundIsStd) {
				toConvert = ground;
			}else {
				const Node *style = node->getProp("style");
				const std::string &styleName = style ? style->params : node->command;

				PyObject *hoverObj = Style::getProp(styleName, "hover");
				if (PyString_CheckExact(hoverObj)) {
					hover = PyString_AS_STRING(hoverObj);
					if (hover.empty()) {
						PyObject *groundObj = Style::getProp(styleName, "ground");
						if (PyString_CheckExact(groundObj)) {
							toConvert = PyString_AS_STRING(groundObj);
						}else {
							std::string type = groundObj->ob_type->tp_name;
							Utils::outMsg("TextButton::hover",
							              "In style." + styleName + ".ground expected type str, got " + type);
						}
						toConvert = ground;
					}
				}else {
					std::string type = hoverObj->ob_type->tp_name;
					Utils::outMsg("TextButton::hover",
					              "In style." + styleName + ".hover expected type str, got " + type);
				}
			}

			if (!toConvert.empty()) {
				hover = PyUtils::exec("CPP_EMBED: textbutton.cpp", __LINE__, "im.MatrixColor(r'" + toConvert + "', im.matrix.brightness(0.1))", true);
			}
		}
		prevGround = ground;
		prevHover = hover;

		const std::string &path = !btnRect.mouseOvered ? ground : hover;
		surface = ImageManipulator::getImage(path, false);

		updateRect();
	}
}

void TextButton::checkEvents() {
	const std::string *styleName = nullptr;
	if (btnRect.mouseOvered != prevMouseOver) {
		const Node *style = node->getProp("style");
		styleName = style ? &style->params : &node->command;
	}

	if (btnRect.mouseOvered) {
		if (!prevMouseOver) {
			const Node *hoverSound = node->getProp("hover_sound");
			if (hoverSound) {
				Music::play("button_hover " + hoverSound->params, hoverSound->getFileName(), hoverSound->getNumLine());
			}else {
				PyObject *hoverSoundObj = Style::getProp(*styleName, "hover_sound");

				if (PyString_CheckExact(hoverSoundObj)) {
					const char *sound = PyString_AS_STRING(hoverSoundObj);
					Music::play("button_hover '" + std::string(sound) + "'",
								getFileName(), getNumLine());
				}else if (hoverSoundObj != Py_None) {
					std::string type = hoverSoundObj->ob_type->tp_name;
					Utils::outMsg("TextButton::hovered",
								  "In style." + *styleName + ".hover_sound expected type str, got " + type);
				}
			}

			const Node *hovered = node->getProp("hovered");
			if (hovered) {
				PyUtils::exec(hovered->getFileName(), hovered->getNumLine(), "exec_funcs(" + hovered->params + ")");
			}else {
				PyUtils::exec(getFileName(), getNumLine(),
							  "exec_funcs(style." + *styleName + ".hovered)");
			}
		}
	}else {
		if (prevMouseOver) {
			const Node *unhovered = node->getProp("unhovered");
			if (unhovered) {
				PyUtils::exec(unhovered->getFileName(), unhovered->getNumLine(), "exec_funcs(" + unhovered->params + ")");
			}else {
				PyUtils::exec(getFileName(), getNumLine(),
							  "exec_funcs(style." + *styleName + ".unhovered)");
			}
		}
	}
	prevMouseOver = btnRect.mouseOvered;

	if (isModal()) {
		if (btnRect.mouseLeftDown) {
			btnRect.onLeftClick();
		}
		if (btnRect.mouseRightDown) {
			btnRect.onRightClick();
		}
	}
}
