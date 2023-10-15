#include "textbutton.h"

#include "style.h"

#include "media/audio_manager.h"
#include "media/image_manipulator.h"
#include "media/py_utils.h"

#include "utils/file_system.h"
#include "utils/utils.h"



static void onLeftClick(DisplayObject* owner) {
	const TextButton *button = static_cast<TextButton*>(owner);
	const Node *node = button->node;
	const Style *style = button->style;

	const Node *activateSound = node->getProp("activate_sound");
	if (activateSound) {
		AudioManager::play("button_click " + activateSound->params,
		                   activateSound->getFileName(), activateSound->getNumLine());
	}else {
		PyObject *activateSoundObj = StyleManager::getProp(style, "activate_sound");

		if (PyUnicode_CheckExact(activateSoundObj)) {
			std::string sound = PyUnicode_AsUTF8(activateSoundObj);
			AudioManager::play("button_click '" + sound + "'", node->getFileName(), node->getNumLine());
		}else if (activateSoundObj != Py_None) {
			std::string type = activateSoundObj->ob_type->tp_name;
			Utils::outMsg("TextButton::onLeftClick",
			              "In style." + style->name + ".activate_sound expected type str, got " + type);
		}
	}

	const Node *action = node->getProp("action");
	if (action) {
		PyUtils::exec(action->getFileName(), action->getNumLine(),
		              "exec_funcs(" + action->params + ")");
	}else {
		StyleManager::execAction(node->getFileName(), node->getNumLine(), style, "action");
	}
}

static void onRightClick(DisplayObject* owner) {
	const TextButton *button = static_cast<TextButton*>(owner);
	const Node *node = button->node;
	const Style *style = button->style;

	const Node* alternate = node->getProp("alternate");
	if (alternate) {
		PyUtils::exec(alternate->getFileName(), alternate->getNumLine(),
		              "exec_funcs(" + alternate->params + ")");
	}else {
		StyleManager::execAction(node->getFileName(), node->getNumLine(), style, "alternate");
	}
}



TextButton::TextButton(Node* node, Screen *screen):
    Text(node, screen),
    hoverIsModifiedGround(node->getProp("ground") && !node->getProp("hover"))
{
	btnRect.init(this, onLeftClick, onRightClick);
}

void TextButton::updateRect(bool) {
	Child::updateRect(false);
	Text::updateRect();

	if (xsize < 0) { xsize = 0; }
	if (ysize < 0) { ysize = 0; }
}

void TextButton::updateTexture(bool skipError) {
	if (skipError && ground.empty()) return;
	if (surface && prevGround == ground && prevHover == hover && prevMouseOver == btnRect.mouseOvered) return;

	if (hoverIsModifiedGround && prevGround != ground) {
		hover = PyUtils::exec("CPP_EMBED: textbutton.cpp", __LINE__,
		                      "im.MatrixColor(r'" + ground + "', im.matrix.brightness(0.1))", true);
	}
	prevGround = ground;
	prevHover = hover;

	const std::string &path = !btnRect.mouseOvered ? ground : hover;
	if (skipError && !FileSystem::exists(path)) return;

	surface = ImageManipulator::getImage(path, false);
	updateRect();
}

void TextButton::checkEvents() {
	if (alpha <= 0) {
		btnRect.mouseOvered = false;
		btnRect.mouseLeftDown = false;
		btnRect.mouseRightDown = false;
	}

	curParamsIsHover = btnRect.mouseOvered || ground == hover;

	if (btnRect.mouseOvered) {
		if (!prevMouseOver) {
			const Node *hoverSound = node->getProp("hover_sound");
			if (hoverSound) {
				AudioManager::play("button_hover " + hoverSound->params, hoverSound->getFileName(), hoverSound->getNumLine());
			}else {
				PyObject *hoverSoundObj = StyleManager::getProp(style, "hover_sound");

				if (PyUnicode_CheckExact(hoverSoundObj)) {
					std::string sound = PyUnicode_AsUTF8(hoverSoundObj);
					AudioManager::play("button_hover '" + sound + "'", getFileName(), getNumLine());
				}else if (hoverSoundObj != Py_None) {
					std::string type = hoverSoundObj->ob_type->tp_name;
					Utils::outMsg("TextButton::hovered",
					              "In style." + style->name + ".hover_sound expected type str, got " + type);
				}
			}

			const Node *hovered = node->getProp("hovered");
			if (hovered) {
				PyUtils::exec(hovered->getFileName(), hovered->getNumLine(), "exec_funcs(" + hovered->params + ")");
			}else {
				StyleManager::execAction(getFileName(), getNumLine(), style, "hovered");
			}
		}
	}else {
		if (prevMouseOver) {
			const Node *unhovered = node->getProp("unhovered");
			if (unhovered) {
				PyUtils::exec(unhovered->getFileName(), unhovered->getNumLine(), "exec_funcs(" + unhovered->params + ")");
			}else {
				StyleManager::execAction(getFileName(), getNumLine(), style, "unhovered");
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
