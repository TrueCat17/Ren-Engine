#include "display_object.h"

#include "group.h"
#include "renderer.h"

#include "utils/math.h"
#include "utils/stage.h"
#include "utils/utils.h"


std::vector<DisplayObject*> DisplayObject::objects;


DisplayObject::DisplayObject() {
	objects.push_back(this);
}

void DisplayObject::updateGlobal() {
	float parentXAnchor, parentYAnchor;
	float parentGlobalX, parentGlobalY;
	float parentGlobalRotate;
	float parentGlobalAlpha;
	if (parent) {
		parentXAnchor = parent->calcedXanchor;
		parentYAnchor = parent->calcedYanchor;
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


	float x = rect.x + calcedXanchor - parentXAnchor;
	float y = rect.y + calcedYanchor - parentYAnchor;

	float sinA = Math::getSin(int(parentGlobalRotate));
	float cosA = Math::getCos(int(parentGlobalRotate));

	float rotX = x * cosA - y * sinA;
	float rotY = x * sinA + y * cosA;

	globalX = parentGlobalX + parentXAnchor + rotX - calcedXanchor;
	globalY = parentGlobalY + parentYAnchor + rotY - calcedYanchor;
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
		globalClipping = clipping;
		if (clipping) {
			if (!Math::floatsAreEq(std::fmod(globalRotate, float(360)), 0)) {
				Utils::outMsg("DisplayObject::updateGlobal", "Object with clipping can't be rotated");
			}
			clipRect = { globalX, globalY, getWidth(), getHeight() };
		}
	}

	if (parent && parent->globalSkipMouse) {
		globalSkipMouse = true;
	}else {
		globalSkipMouse = skip_mouse;
	}
}

bool DisplayObject::transparentForMouse(int x, int y) const {
	if (!enable || globalAlpha <= 0 || globalSkipMouse || !surface) return true;

	if (globalClipping) {
		float fx = float(x);
		float fy = float(y);

		if (fx + globalX < clipRect.x ||
		    fy + globalY < clipRect.y ||
		    fx + globalX >= clipRect.x + clipRect.w ||
		    fy + globalY >= clipRect.y + clipRect.h
		) return true;
	}

	if (x < 0 || y < 0 || x >= int(getWidth()) || y >= int(getHeight())) return true;

	SDL_Rect rect = { x, y, int(getWidth()), int(getHeight()) };
	Uint32 color = Utils::getPixel(surface, rect, crop);
	Uint8 alpha = color & 0xFF;
	return alpha == 0;
}

void DisplayObject::draw() const {
	if (!enable || globalAlpha <= 0 || !surface) return;

	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));
	SDL_Rect clipIRect = DisplayObject::buildIntRect(clipRect.x, clipRect.y, clipRect.w, clipRect.h, false);
	SDL_Rect dstIRect = DisplayObject::buildIntRect(globalX, globalY, rect.w, rect.h, false);
	SDL_Point center = { int(calcedXanchor), int(calcedYanchor) };

	pushToRender(surface, globalRotate, intAlpha, globalClipping, clipIRect, crop, dstIRect, center);
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
	Stage::screens = nullptr;

	while (!objects.empty()) {
		DisplayObject *obj = objects[0];
		delete obj;
	}
}


void DisplayObject::pushToRender(const SurfacePtr &surface, float angle, Uint8 alpha, bool clip, const SDL_Rect &clipRect,
    const SDL_Rect &srcRect, const SDL_Rect &dstRect, const SDL_Point center)
{
	Renderer::renderData.push_back({
	    surface, angle, alpha, clip,
	    clipRect, srcRect, dstRect,
	    center
	});
}


SDL_Rect DisplayObject::buildIntRect(float x, float y, float w, float h, bool exactSize) {
	SDL_Rect res;
	res.x = int(x);
	res.y = int(y);

	if (exactSize) {
		res.w = int(w);
		res.h = int(h);
	}else {
		res.w = int(x + w) - res.x;
		res.h = int(y + h) - res.y;
	}

	return res;
}
