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
	float parentGlobalRotate;
	float parentGlobalAlpha;
	if (parent) {
		parentXAnchor = parent->xAnchor;
		parentYAnchor = parent->yAnchor;
		parentGlobalX = parent->getGlobalX();
		parentGlobalY = parent->getGlobalY();
		parentGlobalRotate = parent->getGlobalRotate();
		parentGlobalAlpha = parent->getGlobalAlpha();
	}else {
		parentXAnchor = parentYAnchor = 0;
		parentGlobalX = parentGlobalY = 0;
		parentGlobalRotate = 0;
		parentGlobalAlpha = 1;
	}


	float x = float(rect.x + xAnchor - parentXAnchor);
	float y = float(rect.y + yAnchor - parentYAnchor);

	float sinA = Math::getSin(int(parentGlobalRotate));
	float cosA = Math::getCos(int(parentGlobalRotate));

	int rotX = int(x * cosA - y * sinA);
	int rotY = int(x * sinA + y * cosA);

	globalX = int(parentGlobalX + parentXAnchor + rotX - xAnchor);
	globalY = int(parentGlobalY + parentYAnchor + rotY - yAnchor);
	globalRotate = parentGlobalRotate + rotate;
	globalAlpha = parentGlobalAlpha * alpha;

	if (parent && parent->globalClipping) {
		globalClipping = true;
		if (clipping) {
			clipRect.x = std::max(parent->clipRect.x, globalX);
			clipRect.y = std::max(parent->clipRect.y, globalY);
			clipRect.w = std::min(parent->clipRect.x + parent->clipRect.w, globalX + getWidth()) - clipRect.x;
			clipRect.h = std::min(parent->clipRect.y + parent->clipRect.h, globalY + getHeight()) - clipRect.y;
		}else {
			clipRect = parent->clipRect;
		}
	}else {
		if (clipping) {
			globalClipping = true;
			clipRect = { globalX, globalY, getWidth(), getHeight() };
		}else {
			globalClipping = false;
		}
	}
}

bool DisplayObject::checkAlpha(int x, int y) const {
	if (!enable || globalAlpha <= 0 || !surface) return false;

	if (globalClipping) {
		if (x + globalX < clipRect.x ||
		    y + globalY < clipRect.y ||
		    x + globalX >= clipRect.x + clipRect.w ||
		    y + globalY >= clipRect.y + clipRect.h
		) return false;
	}

	if (x < 0 || y < 0 || x >= getWidth() || y >= getHeight()) return false;

	SDL_Rect rect = { x, y, getWidth(), getHeight() };
	Uint32 color = Utils::getPixel(surface, rect, crop);
	Uint8 alpha = color & 255;
	return alpha > 0;
}

void DisplayObject::draw() const {
	if (!enable || globalAlpha <= 0 || !surface) return;

	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));
	SDL_Rect dstRect = { globalX, globalY, rect.w, rect.h };
	SDL_Point center = { xAnchor, yAnchor };

	pushToRender(surface, globalRotate, intAlpha, globalClipping, clipRect, crop, dstRect, center);
}

DisplayObject::~DisplayObject() {
	for (size_t i = 0; i < objects.size(); ++i) {
		if (objects[i] == this) {
			objects.erase(objects.begin() + long(i));
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


void DisplayObject::pushToRender(const SurfacePtr &surface, float angle, Uint8 alpha, bool clip, const SDL_Rect &clipRect,
    const SDL_Rect &srcRect, const SDL_Rect &dstRect, const SDL_Point center)
{
	if (clip && !Math::floatsAreEq(angle, 0)) {
		Utils::outMsg("DisplayObject::pushToRender", "Clipped object can't rotate");
	}

	Renderer::toRender.push_back({
	    surface, angle, alpha, clip,
	    clipRect, srcRect, dstRect,
	    center
	});
}
