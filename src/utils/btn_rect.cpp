#include "btn_rect.h"

#include "gui/screen/style.h"

#include "media/audio_manager.h"
#include "media/py_utils.h"

#include "utils/math.h"
#include "utils/mouse.h"
#include "utils/utils.h"


static std::vector<BtnRect*> btnRects;

static bool objInTop(DisplayObject *obj, int mouseX, int mouseY) {
	while (obj->parent && obj->parent->enable) {
		for (size_t i = obj->index + 1; i < obj->parent->children.size(); ++i) {
			DisplayObject *child = obj->parent->children[i];
			if (!child->enable) continue;

			float x = float(mouseX) - child->getGlobalX() - child->calcedXanchor;
			float y = float(mouseY) - child->getGlobalY() - child->calcedYanchor;

			float sinA = Math::getSin(int(-child->getGlobalRotate()));
			float cosA = Math::getCos(int(-child->getGlobalRotate()));

			int rotX = int(x * cosA - y * sinA + child->calcedXanchor);
			int rotY = int(x * sinA + y * cosA + child->calcedYanchor);

			if (!child->transparentForMouse(rotX, rotY)) {
				return false;
			}
		}
		obj = obj->parent;
	}
	return true;
}



BtnRect::BtnRect(Child *owner):
    owner(owner)
{
	btnRects.push_back(this);
}

BtnRect::~BtnRect() {
	for (size_t i = 0; i < btnRects.size(); ++i) {
		BtnRect *btnRect = btnRects[i];
		if (btnRect == this) {
			btnRects.erase(btnRects.begin() + long(i));
			return;
		}
	}
}

void BtnRect::checkMouseCursor() {
	Mouse::setLocal(-1, -1);

	int mouseX = Mouse::getX();
	int mouseY = Mouse::getY();

	for (BtnRect *btnRect : btnRects) {
		btnRect->mouseOvered = false;
		btnRect->mouseLeftDown = false;
		btnRect->mouseRightDown = false;
	}

	for (size_t i = btnRects.size() - 1; i != size_t(-1); --i) {
		BtnRect *btnRect = btnRects[i];
		Child *owner = btnRect->owner;
		if (!owner || !owner->enable) continue;

		float x = float(mouseX) - owner->getGlobalX() - owner->calcedXanchor;
		float y = float(mouseY) - owner->getGlobalY() - owner->calcedYanchor;

		float sinA = Math::getSin(int(-owner->getGlobalRotate()));
		float cosA = Math::getCos(int(-owner->getGlobalRotate()));

		int rotX = int(x * cosA - y * sinA + owner->calcedXanchor);
		int rotY = int(x * sinA + y * cosA + owner->calcedYanchor);

		if (rotX >= 0 && rotX < int(owner->getWidth()) &&
		    rotY >= 0 && rotY < int(owner->getHeight())
		) {
			if (owner->transparentForMouse(rotX, rotY)) continue;
			if (!objInTop(owner, mouseX, mouseY)) continue;

			Mouse::setLocal(rotX, rotY);

			btnRect->mouseOvered = true;
			if (btnRect->buttonMode) {
				Mouse::setButtonMode();
			}else {
				Mouse::setUsualMode();
			}
			return;
		}
	}
	Mouse::setUsualMode();
}

bool BtnRect::checkMouseClick(bool left, bool withKeyboard) {
	if (left && !withKeyboard) {
		Mouse::setMouseDown(true);
	}

	int mouseX = Mouse::getX();
	int mouseY = Mouse::getY();

	for (size_t i = btnRects.size() - 1; i != size_t(-1); --i) {
		BtnRect *btnRect = btnRects[i];
		Child *owner = btnRect->owner;
		if (!owner || !owner->enable) continue;
		if (withKeyboard && !btnRect->buttonMode) continue;

		float x = float(mouseX) - owner->getGlobalX() - owner->calcedXanchor;
		float y = float(mouseY) - owner->getGlobalY() - owner->calcedYanchor;

		float sinA = Math::getSin(int(-owner->getGlobalRotate()));
		float cosA = Math::getCos(int(-owner->getGlobalRotate()));

		int rotX = int(x * cosA - y * sinA + owner->calcedXanchor);
		int rotY = int(x * sinA + y * cosA + owner->calcedYanchor);

		if (rotX >= 0 && rotX < int(owner->getWidth()) &&
		    rotY >= 0 && rotY < int(owner->getHeight())
		) {
			if (owner->transparentForMouse(rotX, rotY)) continue;
			if (!objInTop(owner, mouseX, mouseY)) continue;

			if (left) {
				btnRect->mouseLeftDown = true;
			}else {
				btnRect->mouseRightDown = true;
			}
			return true;
		}
	}
	return false;
}


void BtnRect::onHovered() const {
	const Node *node = owner->node;
	const Style *style = owner->style;

	const Node *hoverSound = node->getProp("hover_sound");
	if (hoverSound) {
		AudioManager::play("button_hover " + hoverSound->params,
		                   hoverSound->getFileName(), hoverSound->getNumLine());
	}else {
		PyObject *hoverSoundObj = StyleManager::getProp(style, "hover_sound");

		if (PyUnicode_CheckExact(hoverSoundObj)) {
			std::string sound = PyUtils::objToStr(hoverSoundObj);
			AudioManager::play("button_hover '" + sound + "'", node->getFileName(), node->getNumLine());
		}else if (hoverSoundObj != Py_None) {
			std::string type = hoverSoundObj->ob_type->tp_name;
			Utils::outMsg("BtnRect::onHovered",
			              "In style." + style->name + ".hover_sound expected type str, got " + type);
		}
	}

	const Node *hovered = node->getProp("hovered");
	if (hovered) {
		PyUtils::exec(hovered->getFileName(), hovered->getNumLine(),
		              "exec_funcs(" + hovered->params + ")");
	}else {
		StyleManager::execAction(node->getFileName(), node->getNumLine(), style, "hovered");
	}
}

void BtnRect::onUnhovered() const {
	const Node *node = owner->node;
	const Style *style = owner->style;

	const Node *unhovered = node->getProp("unhovered");
	if (unhovered) {
		PyUtils::exec(unhovered->getFileName(), unhovered->getNumLine(),
		              "exec_funcs(" + unhovered->params + ")");
	}else {
		StyleManager::execAction(node->getFileName(), node->getNumLine(), style, "unhovered");
	}
}

void BtnRect::onLeftClick() const {
	const Node *node = owner->node;
	const Style *style = owner->style;

	const Node *activateSound = node->getProp("activate_sound");
	if (activateSound) {
		AudioManager::play("button_click " + activateSound->params,
		                   activateSound->getFileName(), activateSound->getNumLine());
	}else {
		PyObject *activateSoundObj = StyleManager::getProp(style, "activate_sound");

		if (PyUnicode_CheckExact(activateSoundObj)) {
			std::string sound = PyUtils::objToStr(activateSoundObj);
			AudioManager::play("button_click '" + sound + "'", node->getFileName(), node->getNumLine());
		}else if (activateSoundObj != Py_None) {
			std::string type = activateSoundObj->ob_type->tp_name;
			Utils::outMsg("BtnRect::onLeftClick",
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

void BtnRect::onRightClick() const {
	const Node *node = owner->node;
	const Style *style = owner->style;

	const Node* alternate = node->getProp("alternate");
	if (alternate) {
		PyUtils::exec(alternate->getFileName(), alternate->getNumLine(),
		              "exec_funcs(" + alternate->params + ")");
	}else {
		StyleManager::execAction(node->getFileName(), node->getNumLine(), style, "alternate");
	}
}

void BtnRect::checkEvents() {
	if (owner->alpha <= 0 || !owner->isModal() || owner->globalSkipMouse) {
		mouseOvered = false;
		mouseLeftDown = false;
		mouseRightDown = false;
	}

	if (mouseOvered) {
		if (!prevMouseOvered) {
			onHovered();
		}
	}else {
		if (prevMouseOvered) {
			onUnhovered();
		}
	}
	prevMouseOvered = mouseOvered;

	if (mouseLeftDown) {
		onLeftClick();
	}
	if (mouseRightDown) {
		onRightClick();
	}
}
