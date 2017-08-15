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

void DisplayObject::updateGlobalPos() {
	int x, y;
	int parentXAnchor, parentYAnchor;
	int parentGlobalX, parentGlobalY, parentGlobalRotate;
	if (parent) {
		parentXAnchor = parent->xAnchor;
		parentYAnchor = parent->yAnchor;
		parentGlobalX = parent->getGlobalX();
		parentGlobalY = parent->getGlobalY();
		parentGlobalRotate = parent->getGlobalRotate();
	}else {
		parentXAnchor = parentYAnchor = 0;
		parentGlobalX = parentGlobalY = parentGlobalRotate = 0;
	}

	x = rect.x + xAnchor - parentXAnchor;
	y = rect.y + yAnchor - parentYAnchor;

	double sinA = Utils::getSin(parentGlobalRotate);
	double cosA = Utils::getCos(parentGlobalRotate);

	int rotX = std::round(x * cosA - y * sinA);
	int rotY = std::round(x * sinA + y * cosA);

	globalX = parentGlobalX + parentXAnchor + rotX - xAnchor;
	globalY = parentGlobalY + parentYAnchor + rotY - yAnchor;
	globalRotate = parentGlobalRotate + rotate;
}
void DisplayObject::updateGlobalAlpha() {
	if (parent) {
		globalAlpha = parent->getGlobalAlpha() * alpha;
	}else {
		globalAlpha = alpha;
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
	if (!enable || globalAlpha <= 0) return;

	if (texture) {
		SDL_Rect t = rect;
		t.x = globalX;
		t.y = globalY;

		Uint8 intAlpha = Utils::inBounds(int(globalAlpha * 255), 0, 255);
		if (SDL_SetTextureAlphaMod(texture, intAlpha)) {
			Utils::outMsg("SDL_SetTextureAlphaMod", SDL_GetError());
		}

		if (!globalRotate) {
			if (SDL_RenderCopy(GV::mainRenderer, texture, &crop, &t)) {
				Utils::outMsg("SDL_RenderCopy", SDL_GetError());
			}
		}else {
			SDL_Point center = { int(xAnchor), int(yAnchor) };
			if (SDL_RenderCopyEx(GV::mainRenderer, texture, &crop, &t, globalRotate, &center, SDL_FLIP_NONE)) {
				Utils::outMsg("SDL_RenderCopyEx", SDL_GetError());
			}
		}
	}
}
bool DisplayObject::useTexture(SDL_Texture *texture) {
	for (const DisplayObject *obj : objects) {
		if (obj->enable && obj->texture == texture) {
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
