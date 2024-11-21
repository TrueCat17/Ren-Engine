#include "display_object.h"

#include "group.h"
#include "renderer.h"

#include "utils/math.h"
#include "utils/stage.h"
#include "utils/utils.h"


std::vector<DisplayObject*> DisplayObject::objects;


DisplayObject::DisplayObject():
    globalClipping(false),
    clearing(false),
    enable(false),
    clipping(false),
    skip_mouse(false),
    globalSkipMouse(false)
{
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


	float x = getX() + calcedXanchor - parentXAnchor;
	float y = getY() + calcedYanchor - parentYAnchor;

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

	float width = getWidth();
	float height = getHeight();
	if (width <= 0 || height <= 0) return;

	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));
	SDL_Point center = { int(calcedXanchor), int(calcedYanchor) };
	SDL_Rect clipIRect = DisplayObject::buildIntRect(clipRect.x, clipRect.y, clipRect.w, clipRect.h, false);

	//simple way
	if (Math::floatsAreEq(corner_sizes_left, 0) &&
	    Math::floatsAreEq(corner_sizes_top, 0) &&
	    Math::floatsAreEq(corner_sizes_right, 0) &&
	    Math::floatsAreEq(corner_sizes_bottom, 0)
	) {
		SDL_Rect dstIRect = DisplayObject::buildIntRect(globalX, globalY, width, height, false);
		pushToRender(surface, globalRotate, intAlpha, globalClipping, clipIRect, crop, dstIRect, center);
		return;
	}


	//zoom with const corner-sizes:
	// 1 2 3      1 2 2 3
	// 4 5 6  ->  4 5 5 6
	// 7 8 9      4 5 5 6
	//            7 8 8 9

	float cropW = float(crop.w);
	float cropH = float(crop.h);

#define makeSide(side, rel, defaultSize) \
	float side = std::round( \
	    corner_sizes_##side < 0 ? \
	        defaultSize : \
	        corner_sizes_##side * (corner_sizes_##side##_is_float ? rel : 1) \
	)
	makeSide(left, cropW, std::floor(std::min(cropW, cropH) / 3));
	makeSide(top, cropH, left);
	makeSide(right, cropW, left);
	makeSide(bottom, cropH, top);
#undef makeSide

	float zoomX = (left + right) / (width * float(0.8));
	float zoomY = (top + bottom) / (height * float(0.8));
	float zoom = std::ceil(std::max(std::max(zoomX, zoomY), float(1)));

	std::pair<int, float> ws[3] = {
	    { left, left / zoom },
	    { cropW - (left + right), (width  * zoom - (left + right)) / zoom },
	    { right, right / zoom }
	};
	std::pair<int, float> hs[3] = {
	    { top, top / zoom },
	    { cropH - (top + bottom), (height * zoom - (top + bottom)) / zoom },
	    { bottom, bottom / zoom }
	};


	int srcY = crop.y;
	float dstY = globalY;
	for (auto [srcH, dstH] : hs) {
		int srcX = crop.x;
		float dstX = globalX;

		for (auto [srcW, dstW] : ws) {
			SDL_Rect dstIRect = DisplayObject::buildIntRect(dstX, dstY, dstW, dstH, false);
			SDL_Rect partCrop = { srcX, srcY, srcW, srcH };
			pushToRender(surface, globalRotate, intAlpha, globalClipping, clipIRect, partCrop, dstIRect, center);

			srcX += srcW;
			dstX += dstW;
		}

		srcY += srcH;
		dstY += dstH;
	}
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
	res.x = int(std::round(x));
	res.y = int(std::round(y));

	if (exactSize) {
		res.w = int(w);
		res.h = int(h);
	}else {
		res.w = int(std::round(x + w)) - res.x;
		res.h = int(std::round(y + h)) - res.y;
	}

	return res;
}
