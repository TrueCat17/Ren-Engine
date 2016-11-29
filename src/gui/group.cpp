#include "group.h"

#include "utils/utils.h"


Group::Group() {

}

void Group::updateGlobalX() {
	DisplayObject::updateGlobalX();

	for (size_t i = 0; i < children.size(); ++i) {
		DisplayObject *child = children[i];
		child->updateGlobalX();
	}
}
void Group::updateGlobalY() {
	DisplayObject::updateGlobalY();

	for (size_t i = 0; i < children.size(); ++i) {
		DisplayObject *child = children[i];
		child->updateGlobalY();
	}
}

int Group::getMinX() const {
	int res = 0;
	for (size_t i = 0; i < children.size(); ++i) {
		const DisplayObject *child = children[i];
		int childMinX = child->getMinX();
		if (childMinX < res) {
			res = childMinX;
		}
	}
	return res;
}
int Group::getMinY() const {
	int res = 0;
	for (size_t i = 0; i < children.size(); ++i) {
		const DisplayObject *child = children[i];
		int childMinY = child->getMinY();
		if (childMinY < res) {
			res = childMinY;
		}
	}
	return res;
}

int Group::getMaxX() const {
	int res = std::max(0, rect.w);
	if (!texture) {
		if (children.empty()) return res;

		res = std::max(res, children[0]->getX() + children[0]->getMaxX());
	}

	for (size_t i = 0; i < children.size(); ++i) {
		const DisplayObject *child = children[i];
		int childMaxX = child->getX() + child->getMaxX();
		if (childMaxX > res) {
			res = childMaxX;
		}
	}
	return res;
}
int Group::getMaxY() const {
	int res = std::max(0, rect.h);
	if (!texture) {
		if (children.empty()) return res;

		res = std::max(res, children[0]->getY() + children[0]->getMaxY());
	}

	for (size_t i = 0; i < children.size(); ++i) {
		const DisplayObject *child = children[i];
		int childMaxY = child->getY() + child->getMaxY();
		if (childMaxY > res) {
			res = childMaxY;
		}
	}
	return res;
}

DisplayObject* Group::getChildAt(size_t index) const {
	return children[index];
}
size_t Group::getChildIndex(DisplayObject *child) const {
	for (size_t i = 0; i < children.size(); ++i) {
		if (children[i] == child){
			return i;
		}
	}
	return -1;
}

void Group::addChild(DisplayObject *child) {
	addChildAt(child, children.size());
}
void Group::addChildAt(DisplayObject *child, size_t index) {
	if (child == this) {
		Utils::outMsg("Group::addChild(At)", "Добавление объекта в самого себя");
		return;
	}

	if (child->parent == this) {
		if (children[children.size() - 1] != child) {
			for (size_t i = 0; i < children.size(); ++i) {
				if (children[i] == child) {
					children.erase(children.begin() + i);
					break;
				}
			}
			children.push_back(child);
		}
		return;
	}

	if (child->parent) {
		child->parent->removeChild(child);
	}

	child->parent = this;
	child->updateGlobalX();
	child->updateGlobalY();

	if (children.size() < index) {
		children.insert(children.begin() + index, child);
	}else {
		children.push_back(child);
	}
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
	DisplayObject *child = children[index];

	if (child->parent != this) {
		Utils::outMsg("Group::removeChildAt", "Удаление объекта не из своего родителя");
		return;
	}

	children.erase(children.begin() + index);
}
void Group::clearChildren() {
	while (children.size()) {
		DisplayObject *obj = children[0];
		delete obj;//Удаляется так же и из текущего объекта как из родителя (а значит и из children)
	}
	children.clear();
}

bool Group::checkAlpha(int x, int y) const {
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
	if (!enabled()) return;

	DisplayObject::draw();

	for (size_t i = 0; i < children.size(); ++i) {
		const DisplayObject *child = children[i];
		child->draw();
	}
}

SDL_Texture* Group::getTextureIfOne(size_t &count) const {
	SDL_Texture *res = nullptr;
	for (DisplayObject* child : children) {
		size_t t = 0;
		SDL_Texture *childTexture = child->getTextureIfOne(t);

		if ((t != 0 && t != 1) || (childTexture && res)) {
			count = size_t(-1);
			return nullptr;
		}
		res = childTexture;
	}
	if (res && texture) {
		count = size_t(-1);
		return nullptr;
	}

	if (res) {
		count = 1;
	}else{
		res = texture;
		count = res ? 1 : 0;
	}
	return res;
}

Group::~Group() {
	if (parent) {
		parent->removeChild(this);
	}
	clearChildren();
}
