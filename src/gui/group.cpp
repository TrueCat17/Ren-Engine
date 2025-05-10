#include "group.h"


void Group::updateGlobal() {
	DisplayObject::updateGlobal();

	for (DisplayObject *child : children) {
		if (child->enable) {
			child->updateGlobal();
		}
	}
}

void Group::addChildAt(DisplayObject *child, uint32_t index) {
	child->removeFromParent();

	index = std::min(index, uint32_t(children.size()));
	children.insert(children.begin() + long(index), child);

	child->index = index;
	for (size_t i = index + 1; i < children.size(); ++i) {
		++children[i]->index;
	}

	child->parent = this;
	child->updateGlobal();
}
void Group::removeChild(DisplayObject *child) {
	if (clearing) return;

	size_t i = 0;
	for (; i < children.size(); ++i) {
		if (children[i] == child) {
			children.erase(children.begin() + long(i));
			break;
		}
	}
	for (; i < children.size(); ++i) {
		--children[i]->index;
	}
}
void Group::clearChildren() {
	clearing = true;
	for (DisplayObject *obj : children) {
		delete obj;
	}
	children.clear();
	clearing = false;
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
