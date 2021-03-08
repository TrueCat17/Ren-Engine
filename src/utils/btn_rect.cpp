#include "btn_rect.h"

#include "gui/group.h"

#include "utils/math.h"
#include "utils/mouse.h"


std::vector<BtnRect*> BtnRect::btnRects;

bool BtnRect::objInTop(DisplayObject *obj, int mouseX, int mouseY) {
	while (obj->parent && obj->parent->enable) {
		for (size_t i = obj->parent->getChildIndex(obj) + 1; i < obj->parent->children.size(); ++i) {
			DisplayObject *child = obj->parent->children[i];
			if (!child->enable) continue;

			float x = float(mouseX - child->getGlobalX() - child->xAnchor);
			float y = float(mouseY - child->getGlobalY() - child->yAnchor);

			float sinA = Math::getSin(int(-child->getGlobalRotate()));
			float cosA = Math::getCos(int(-child->getGlobalRotate()));

			int rotX = int(x * cosA - y * sinA + float(child->xAnchor));
			int rotY = int(x * sinA + y * cosA + float(child->yAnchor));

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
		DisplayObject *owner = btnRect->getOwner();
		if (!owner || !owner->enable) continue;

		float x = float(mouseX - owner->getGlobalX() - owner->xAnchor);
		float y = float(mouseY - owner->getGlobalY() - owner->yAnchor);

		float sinA = Math::getSin(int(-owner->getGlobalRotate()));
		float cosA = Math::getCos(int(-owner->getGlobalRotate()));

		int rotX = int(x * cosA - y * sinA + float(owner->xAnchor));
		int rotY = int(x * sinA + y * cosA + float(owner->yAnchor));

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

	for (size_t i = btnRects.size() - 1; i != size_t(-1); --i) {
		BtnRect *btnRect = btnRects[i];
		DisplayObject *owner = btnRect->getOwner();
		if (!owner || !owner->enable) continue;
		if (withKeyboard && !btnRect->buttonMode) continue;

		float x = float(mouseX - owner->getGlobalX() - owner->xAnchor);
		float y = float(mouseY - owner->getGlobalY() - owner->yAnchor);

		float sinA = Math::getSin(int(-owner->getGlobalRotate()));
		float cosA = Math::getCos(int(-owner->getGlobalRotate()));

		int rotX = int(x * cosA - y * sinA + float(owner->xAnchor));
		int rotY = int(x * sinA + y * cosA + float(owner->yAnchor));

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
