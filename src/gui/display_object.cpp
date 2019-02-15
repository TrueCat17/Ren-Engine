#include "display_object.h"

#include "gv.h"
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

	if (parent && parent->globalClip) {
		globalClip = true;
		if (clip) {
			clipRect.x = std::max(parent->clipRect.x, globalX);
			clipRect.y = std::max(parent->clipRect.y, globalY);
			clipRect.w = std::min(parent->clipRect.x + parent->clipRect.w, globalX + getWidth()) - clipRect.x;
			clipRect.h = std::min(parent->clipRect.y + parent->clipRect.h, globalY + getHeight()) - clipRect.y;
		}else {
			clipRect = parent->clipRect;
		}
	}else {
		if (clip) {
			globalClip = true;
			clipRect = {globalX, globalY, getWidth(), getHeight()};
		}else {
			globalClip = false;
		}
	}
}

bool DisplayObject::checkAlpha(int x, int y) const {
	if (!enable || globalAlpha <= 0 || !surface) return false;

	if (globalClip) {
		if (x + globalX < clipRect.x ||
		    y + globalY < clipRect.y ||
		    x + globalX >= clipRect.x + clipRect.w ||
		    y + globalY >= clipRect.y + clipRect.h
		) return false;
	}

	if (x < 0 || y < 0 || x >= getWidth() || y >= getHeight()) return false;

	SDL_Rect rect = {x, y, getWidth(), getHeight()};
	Uint32 color = Utils::getPixel(surface, rect, crop);
	Uint8 alpha = color & 255;
	return alpha > 0;
}

void DisplayObject::draw() const {
	if (!enable || globalAlpha <= 0 || !surface) return;

	SDL_Rect t = {globalX, globalY, rect.w, rect.h};
	SDL_Point center = { xAnchor, yAnchor };
	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));

	pushToRender(surface, globalRotate, intAlpha, globalClip, clipRect, crop, t, center);
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

	while (!objects.empty()) {
		DisplayObject *obj = objects[0];
		delete obj;
	}
}


void DisplayObject::pushToRender(const SurfacePtr &surface, int angle, Uint8 alpha, bool clip, const SDL_Rect clipRect,
    const SDL_Rect srcRect, const SDL_Rect dstRect, const SDL_Point center)
{
	Renderer::toRender.push_back({
	    surface, angle, alpha, clip,
	    clipRect, srcRect, dstRect,
	    center
	});
}
