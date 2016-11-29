#include "btn_rect.h"

#include "utils/mouse.h"

std::vector<BtnRect*> BtnRect::btnRects;

BtnRect::BtnRect() {
	btnRects.push_back(this);
}

void BtnRect::init(DisplayObject *owner, std::function<void (DisplayObject *)> onClick, bool buttonMode) {
	this->owner = owner;
	this->onClick = onClick;
	this->buttonMode = buttonMode;
}

void BtnRect::click() const {
	if (onClick != nullptr) {
		onClick(owner);
	}
}

BtnRect::~BtnRect() {
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
	}

	for (int i = btnRects.size() - 1; i >= 0; --i) {
		BtnRect *btnRect = btnRects[i];
		DisplayObject *owner = btnRect->getOwner();
		if (!owner) continue;

		if (mouseX > owner->getGlobalX() && mouseX < owner->getGlobalX() + owner->getWidth() &&
			mouseY > owner->getGlobalY() && mouseY < owner->getGlobalY() + owner->getHeight()
		) {
			if (!owner->checkAlpha(mouseX - owner->getGlobalX(), mouseY - owner->getGlobalY())) continue;

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

void BtnRect::checkMouseClick() {
	int mouseX, mouseY;
	Mouse::getPos(mouseX, mouseY);

	for (int i = btnRects.size() - 1; i >= 0; --i) {
		BtnRect *btnRect = btnRects[i];
		DisplayObject *owner = btnRect->getOwner();
		if (!owner) continue;

		if (mouseX > owner->getGlobalX() && mouseX < owner->getGlobalX() + owner->getWidth() &&
			mouseY > owner->getGlobalY() && mouseY < owner->getGlobalY() + owner->getHeight()
		) {
			if (!owner->checkAlpha(mouseX - owner->getGlobalX(), mouseY - owner->getGlobalY())) continue;

			btnRect->click();
			return;
		}
	}
}
