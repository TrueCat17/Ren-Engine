#include "group.h"


void Group::updateGlobal() {
	DisplayObject::updateGlobal();

	for (DisplayObject *child : children) {
		if (child->enable) {
			child->updateGlobal();
		}
	}
}

size_t Group::getChildIndex(DisplayObject *child) const {
	for (size_t i = 0; i < children.size(); ++i) {
		if (children[i] == child) {
			return i;
		}
	}
	return size_t(-1);
}

void Group::addChildAt(DisplayObject *child, size_t index) {
	if (child->parent) {
		child->parent->removeChild(child);
	}

	auto to = std::min(children.begin() + long(index), children.end());
	children.insert(to, child);

	child->parent = this;
	child->updateGlobal();
}
void Group::removeChild(DisplayObject *child) {
	for (size_t i = 0; i < children.size(); ++i) {
		if (children[i] == child) {
			children.erase(children.begin() + long(i));
			break;
		}
	}
}
void Group::clearChildren() {
	while (!children.empty()) {
		DisplayObject *obj = children[0];
		delete obj;//remove from .parent=this (thereforce from children of parent)
	}
}

bool Group::transparentForMouse(int x, int y) const {
	if (!DisplayObject::transparentForMouse(x, y)) {
		return false;
	}

	for (const DisplayObject *child : children) {
		if (!child->transparentForMouse(x - int(child->getX()), y - int(child->getY()))) {
			return false;
		}
	}
	return true;
}

void Group::draw() const {
	if (!enable || globalAlpha <= 0) return;

	DisplayObject::draw();

	for (const DisplayObject *child : children) {
		child->draw();
	}
}

Group::~Group() {
	clearChildren();
}
