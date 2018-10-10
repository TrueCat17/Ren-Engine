#include "display_object.h"

#include "group.h"
#include "renderer.h"

#include "utils/math.h"
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

	double sinA = Math::getSin(parentGlobalRotate);
	double cosA = Math::getCos(parentGlobalRotate);

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
	if (!enable || globalAlpha <= 0) return false;

	if (surface && (x >= 0 && y >= 0 && x < rect.w && y < rect.h)) {
		SDL_Rect rect = {x, y, this->rect.w, this->rect.h};
		Uint32 color = Utils::getPixel(surface, rect, crop);
		Uint8 alpha = color & 255;

		return alpha > 0;
	}
	return false;
}

void DisplayObject::draw() const {
	if (!enable || globalAlpha <= 0) return;

	if (surface) {
		SDL_Rect t = {globalX, globalY, rect.w, rect.h};
		SDL_Point center = { xAnchor, yAnchor };
		Uint8 intAlpha = Math::inBounds(int(globalAlpha * 255), 0, 255);

		pushToRender(surface, globalRotate, intAlpha, &crop, &t, &center);
	}
}

DisplayObject::~DisplayObject() {
	for (size_t i = 0; i < objects.size(); ++i) {
		if (objects[i] == this) {
			objects.erase(objects.begin() + i);
			break;
		}
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

void DisplayObject::pushToRender(const SurfacePtr &surface, float angle, Uint8 alpha,
	const SDL_Rect *srcRect, const SDL_Rect *dstRect, const SDL_Point *center)
{
	if (surface && alpha) {
		static const SDL_Rect emptyRect = {0, 0, 0, 0};
		static const SDL_Point emptyPoint = {0, 0};

		Renderer::toRender.push_back({
			surface, angle, alpha,
			srcRect == nullptr, dstRect == nullptr, center == nullptr,

			srcRect ? *srcRect : emptyRect, dstRect ? *dstRect : emptyRect,
			center ? *center : emptyPoint
		});
	}
}
