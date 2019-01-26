#include "group.h"

#include "utils/utils.h"


void Group::updateGlobal() {
	DisplayObject::updateGlobal();

	for (size_t i = 0; i < children.size(); ++i) {
		DisplayObject *child = children[i];
		if (child->enable) {
			child->updateGlobal();
		}
	}
}

size_t Group::getChildIndex(DisplayObject *child) const {
	for (size_t i = 0; i < children.size(); ++i) {
		if (children[i] == child){
			return i;
		}
	}
	return size_t(-1);
}

void Group::addChildAt(DisplayObject *child, size_t index) {
	if (child == this) {
		Utils::outMsg("Group::addChild(At)", "Добавление объекта в самого себя");
		return;
	}

	if (child->parent) {
		child->parent->removeChild(child);
	}

	auto to = std::min(children.begin() + int(index), children.end());
	children.insert(to, child);

	child->parent = this;
	child->updateGlobal();
}
void Group::removeChild(DisplayObject *child) {
	if (child->parent != this) {
		Utils::outMsg("Group::removeChild", "Удаление объекта не из своего родителя");
		return;
	}

	for (size_t i = 0; i < children.size(); ++i) {
		if (children[i] == child) {
			removeChildAt(i);
			break;
		}
	}
}
void Group::removeChildAt(size_t index) {
	children.erase(children.begin() + int(index));
}
void Group::clearChildren() {
	while (children.size()) {
		DisplayObject *obj = children[0];
		delete obj;//Удаляется так же и из текущего объекта как из родителя (а значит и из children)
	}
}

bool Group::checkAlpha(int x, int y) const {
	if (!enable || globalAlpha <= 0) return false;

	if (DisplayObject::checkAlpha(x, y)) {
		return true;
	}

	for (const DisplayObject *child : children) {
		if (child->checkAlpha(x - child->getX(), y - child->getY())) {
			return true;
		}
	}
	return false;
}

void Group::draw() const {
	if (!enable || globalAlpha <= 0) return;

	DisplayObject::draw();

	for (size_t i = 0; i < children.size(); ++i) {
		const DisplayObject *child = children[i];
		child->draw();
	}
}

Group::~Group() {
	clearChildren();
}
