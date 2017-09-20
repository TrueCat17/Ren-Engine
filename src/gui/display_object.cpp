#include "display_object.h"

#include <algorithm>

#include "gv.h"

#include "group.h"


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

	int rotX = x * cosA - y * sinA;
	int rotY = x * sinA + y * cosA;

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
	if (texture && (x >= 0 && y >= 0 && x < rect.w && y < rect.h)) {
		SDL_Rect rect = {x, y, this->rect.w, this->rect.h};
		Uint32 color = Utils::getPixel(texture, rect, crop);
		Uint32 alpha = color & 0xFF;

		return alpha > 0;
	}
	return false;
}

void DisplayObject::draw() const {
	if (!enable || globalAlpha <= 0) return;

	if (texture) {
		SDL_Rect t = {globalX, globalY, rect.w, rect.h};
		SDL_Point center = { xAnchor, yAnchor };
		Uint8 intAlpha = Utils::inBounds(int(globalAlpha * 255), 0, 255);

		pushToRender(texture, intAlpha, &crop, &t, globalRotate, &center);
	}
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

void DisplayObject::destroyAll() {
	GV::screens = nullptr;

	while (objects.size()) {
		DisplayObject *obj = objects[0];
		delete obj;
	}
}

void DisplayObject::pushToRender(
	const TexturePtr &texture, Uint8 alpha,
	const SDL_Rect *srcRect, const SDL_Rect *dstRect,
	double angle, const SDL_Point *center)
{
	static const SDL_Rect emptyRect = {0, 0, 0, 0};
	static const SDL_Point emptyPoint = {0, 0};

	GV::toRender.push_back({
		texture, alpha,
		srcRect == nullptr, dstRect == nullptr, center == nullptr,

		SDL_Rect(srcRect ? *srcRect : emptyRect), SDL_Rect(dstRect ? *dstRect : emptyRect),
		angle, SDL_Point(center ? *center : emptyPoint)
	});
}
