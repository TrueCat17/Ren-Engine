#include "btn_rect.h"

#include <iostream>

#include "utils/mouse.h"
#include "utils/utils.h"

std::vector<BtnRect*> BtnRect::btnRects;

BtnRect::BtnRect() {
	btnRects.push_back(this);
}

void BtnRect::init(DisplayObject *owner, const std::function<void (DisplayObject *)> &onClick, bool buttonMode) {
	this->owner = owner;
	this->_onClick = onClick;
	this->buttonMode = buttonMode;
}

void BtnRect::click() const {
	if (_onClick != nullptr) {
		_onClick(owner);
	}
}

BtnRect::~BtnRect() {
	owner = nullptr;
	_onClick = nullptr;

	for (size_t i = 0; i < btnRects.size(); ++i) {
		BtnRect *btnRect = btnRects[i];
		if (btnRect == this) {
			btnRects.erase(btnRects.begin() + i);
			return;
		}
	}
}

void BtnRect::checkMouseCursor() {
	int mouseX, mouseY;
	Mouse::getPos(mouseX, mouseY);

	for (BtnRect *btnRect : btnRects) {
		btnRect->mouseOvered = false;
		btnRect->mouseDown = false;
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

bool BtnRect::checkMouseClick() {
	int mouseX, mouseY;
	Mouse::getPos(mouseX, mouseY);

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

			btnRect->mouseDown = true;
			return true;
		}
	}
	return false;
}

void BtnRect::onClick() const {
	_onClick(owner);
}
