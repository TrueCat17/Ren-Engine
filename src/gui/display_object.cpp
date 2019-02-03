#include "display_object.h"

#include "group.h"
#include "renderer.h"

#include "utils/math.h"
#include "utils/utils.h"


std::vector<DisplayObject*> DisplayObject::objects;


DisplayObject::DisplayObject() {
	objects.push_back(this);
}

void DisplayObject::updateGlobal() {
	int parentXAnchor, parentYAnchor;
	int parentGlobalX, parentGlobalY;
	int parentGlobalRotate;
	double parentGlobalAlpha;
	if (parent) {
		parentXAnchor = parent->xAnchor;
		parentYAnchor = parent->yAnchor;
		parentGlobalX = parent->getGlobalX();
		parentGlobalY = parent->getGlobalY();
		parentGlobalRotate = parent->getGlobalRotate();
		parentGlobalAlpha = parent->getGlobalAlpha();
	}else {
		parentXAnchor = parentYAnchor = 0;
		parentGlobalX = parentGlobalY = parentGlobalRotate = 0;
		parentGlobalAlpha = 1;
	}

	int x = rect.x + xAnchor - parentXAnchor;
	int y = rect.y + yAnchor - parentYAnchor;

	double sinA = Math::getSin(parentGlobalRotate);
	double cosA = Math::getCos(parentGlobalRotate);

	int rotX = int(x * cosA - y * sinA);
	int rotY = int(x * sinA + y * cosA);

	globalX = parentGlobalX + parentXAnchor + rotX - xAnchor;
	globalY = parentGlobalY + parentYAnchor + rotY - yAnchor;
	globalRotate = parentGlobalRotate + rotate;
	globalAlpha = parentGlobalAlpha * alpha;
}

void DisplayObject::setPos(int x, int y) {
	rect.x = x;
	rect.y = y;
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
	if (!enable || globalAlpha <= 0 || !surface) return;

	SDL_Rect t = {globalX, globalY, rect.w, rect.h};
	SDL_Point center = { xAnchor, yAnchor };
	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));

	pushToRender(surface, globalRotate, intAlpha, crop, t, center);
}

DisplayObject::~DisplayObject() {
	for (size_t i = 0; i < objects.size(); ++i) {
		if (objects[i] == this) {
			objects.erase(objects.begin() + int(i));
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

void DisplayObject::disableAll() {
	for (DisplayObject* obj : objects) {
		obj->enable = false;
	}
}

void DisplayObject::destroyAll() {
	GV::screens = nullptr;

	while (objects.size()) {
		DisplayObject *obj = objects[0];
		delete obj;
	}
}


void DisplayObject::pushToRender(const SurfacePtr &surface, int angle, Uint8 alpha,
	const SDL_Rect srcRect, const SDL_Rect dstRect, const SDL_Point center)
{
	if (surface && alpha) {
		Renderer::toRender.push_back({
			surface, angle, alpha,
			srcRect, dstRect,
			center
		});
	}
}
