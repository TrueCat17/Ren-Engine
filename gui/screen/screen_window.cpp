#include "screen_window.h"

#include <algorithm>

std::vector<ScreenWindow*> ScreenWindow::created;
bool ScreenWindow::_hasModal = false;

void ScreenWindow::updateModality() {
	for (ScreenWindow *w : created) {
		w->updateModalProp();
	}

	_hasModal = false;
	for (ScreenWindow *w : created) {
		if (w->isModal()) {
			_hasModal = true;
			break;
		}
	}
}


ScreenWindow::ScreenWindow(Node *node): ScreenContainer(node, this) {
	created.push_back(this);
}
ScreenWindow::~ScreenWindow() {
	auto i = std::find(created.begin(), created.end(), this);
	if (i != created.end()) {
		created.erase(i);
	}
}

int ScreenWindow::getMinX() const {
	return DisplayObject::getMinX();
}
int ScreenWindow::getMinY() const {
	return DisplayObject::getMinY();
}
int ScreenWindow::getMaxX() const {
	return DisplayObject::getMaxX();
}
int ScreenWindow::getMaxY() const {
	return DisplayObject::getMaxY();
}

void ScreenWindow::updateModalProp() {
	_isModal = node->getProp("modal") == "True";
}
