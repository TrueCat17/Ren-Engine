#include "btn_rect.h"

#include <iostream>

#include "gui/group.h"

#include "utils/mouse.h"


std::vector<BtnRect*> BtnRect::btnRects;

bool BtnRect::objInTop(DisplayObject *obj, int mouseX, int mouseY) {
	while (obj->parent && obj->parent->enable) {
		for (size_t i = obj->parent->getChildIndex(obj) + 1; i < obj->parent->children.size(); ++i) {
			DisplayObject *child = obj->parent->children[i];
			if (!child->enable) continue;

			int x = mouseX - child->getGlobalX() - child->xAnchor;
			int y = mouseY - child->getGlobalY() - child->yAnchor;

			double sinA = Utils::getSin(-child->getGlobalRotate());
			double cosA = Utils::getCos(-child->getGlobalRotate());

			int rotX = x * cosA - y * sinA + child->xAnchor;
			int rotY = x * sinA + y * cosA + child->yAnchor;

			if (child->checkAlpha(rotX, rotY)) {
				return false;
			}
		}
		obj = obj->parent;
	}
	return true;
}



BtnRect::BtnRect() {
	btnRects.push_back(this);
}

void BtnRect::init(DisplayObject *owner,
				   const std::function<void (DisplayObject*)> &onLeftClick,
				   const std::function<void (DisplayObject*)> &onRightClick,
				   bool buttonMode)
{
	this->owner = owner;
	_onLeftClick = onLeftClick;
	_onRightClick = onRightClick;
	this->buttonMode = buttonMode;
}


BtnRect::~BtnRect() {
	owner = nullptr;
	_onLeftClick = nullptr;
	_onRightClick = nullptr;

	for (size_t i = 0; i < btnRects.size(); ++i) {
		BtnRect *btnRect = btnRects[i];
		if (btnRect == this) {
			btnRects.erase(btnRects.begin() + i);
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

	for (int i = btnRects.size() - 1; i >= 0; --i) {
		BtnRect *btnRect = btnRects[i];
		DisplayObject *owner = btnRect->getOwner();
		if (!owner || !owner->enable) continue;

		int x = mouseX - owner->getGlobalX() - owner->xAnchor;
		int y = mouseY - owner->getGlobalY() - owner->yAnchor;

		double sinA = Utils::getSin(-owner->getGlobalRotate());
		double cosA = Utils::getCos(-owner->getGlobalRotate());

		int rotX = x * cosA - y * sinA + owner->xAnchor;
		int rotY = x * sinA + y * cosA + owner->yAnchor;

		if (rotX > 0 && rotX < owner->getWidth() &&
			rotY > 0 && rotY < owner->getHeight()
		) {
			if (!owner->checkAlpha(rotX, rotY)) continue;
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

	for (int i = btnRects.size() - 1; i >= 0; --i) {
		BtnRect *btnRect = btnRects[i];
		DisplayObject *owner = btnRect->getOwner();
		if (!owner || !owner->enable) continue;
		if (withKeyboard && !btnRect->buttonMode) continue;

		int x = mouseX - owner->getGlobalX() - owner->xAnchor;
		int y = mouseY - owner->getGlobalY() - owner->yAnchor;

		double sinA = Utils::getSin(-owner->getGlobalRotate());
		double cosA = Utils::getCos(-owner->getGlobalRotate());

		int rotX = x * cosA - y * sinA + owner->xAnchor;
		int rotY = x * sinA + y * cosA + owner->yAnchor;

		if (rotX > 0 && rotX < owner->getWidth() &&
			rotY > 0 && rotY < owner->getHeight()
		) {
			if (!owner->checkAlpha(rotX, rotY)) continue;
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

void BtnRect::onLeftClick() const {
	if (_onLeftClick != nullptr) {
		_onLeftClick(owner);
	}
}
void BtnRect::onRightClick() const {
	if (_onRightClick != nullptr) {
		_onRightClick(owner);
	}
}
