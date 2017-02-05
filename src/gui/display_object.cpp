#include "display_object.h"

#include <algorithm>

#include "gv.h"

#include "group.h"
#include "utils/utils.h"


std::vector<DisplayObject*> DisplayObject::objects;


DisplayObject::DisplayObject() {
	rect.x = 0;
	rect.y = 0;
	rect.w = -1;
	rect.h = -1;

	objects.push_back(this);
}

void DisplayObject::updateGlobalX() {
	if (parent) {
		globalX = parent->getGlobalX() + rect.x;
	}else {
		globalX = rect.x;
	}
}
void DisplayObject::updateGlobalY() {
	if (parent) {
		globalY = parent->getGlobalY() + rect.y;
	}else {
		globalY = rect.y;
	}
}

void DisplayObject::setPos(int x, int y) {
	setX(x);
	setY(y);
}
void DisplayObject::setSize(int w, int h) {
	rect.w = w;
	rect.h = h;
}

bool DisplayObject::checkAlpha(int x, int y) const {
	if (texture && (x < rect.w && y < rect.h)) {
		SDL_Rect rect = {x, y, this->rect.w, this->rect.h};
		Uint32 color = Utils::getPixel(texture, rect, crop);
		Uint32 alpha = color & 0xFF;
		if (alpha > 0) {
			return true;
		}
	}
	return false;
}

void DisplayObject::draw() const {
	if (!enable) return;

	if (texture) {
		SDL_Rect t = rect;
		t.x = globalX;
		t.y = globalY;

		if (SDL_RenderCopy(GV::mainRenderer, texture, &crop, &t)) {
			Utils::outMsg("SDL_RenderCopy", SDL_GetError());
		}
	}
}
bool DisplayObject::useTexture(SDL_Texture *texture) {
	for (const DisplayObject *obj : objects) {
		if (obj->texture == texture) {
			return true;
		}
	}
	return false;
}

DisplayObject::~DisplayObject() {
	auto i = std::find(objects.begin(), objects.end(), this);
	if (i != objects.end()) {
		objects.erase(i);
	}

	removeFromParent();
}

void DisplayObject::removeFromParent() {
	if (parent) {
		parent->removeChild(this);
	}
}

SDL_Texture* DisplayObject::getTextureIfOne(size_t &count) const {
	count = 1;
	return texture;
}

void DisplayObject::destroyAll() {
	GV::screens = nullptr;

	while (objects.size()) {
		DisplayObject *obj = objects[0];
		delete obj;
	}
}
