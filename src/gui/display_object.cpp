#include "display_object.h"

#include "renderer.h"

#include "group.h"

#include "utils/algo.h"
#include "utils/math.h"
#include "utils/message.h"
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
				Message::outMsg("DisplayObject::updateGlobal", "Object with clipping can't be rotated");
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


static bool withoutCornerSizes(const DisplayObject *obj) {
	if (!Math::floatsAreEq(obj->corner_sizes_left,   0)) return false;
	if (!Math::floatsAreEq(obj->corner_sizes_top,    0)) return false;
	if (!Math::floatsAreEq(obj->corner_sizes_right,  0)) return false;
	if (!Math::floatsAreEq(obj->corner_sizes_bottom, 0)) return false;
	return true;
}

template<typename T>
static void callForEachPart(const DisplayObject *obj, const SDL_Rect &srcRect, const SDL_Rect &dstRect, T func) {
	float width  = float(dstRect.w);
	float height = float(dstRect.h);

	float cropW = float(srcRect.w);
	float cropH = float(srcRect.h);

#define makeSide(side, rel, defaultSize) \
	float side = std::round( \
	    obj->corner_sizes_##side < 0 ? \
	        defaultSize : \
	        obj->corner_sizes_##side * (obj->corner_sizes_##side##_is_float ? rel : 1) \
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

	int srcRectXW = srcRect.x + srcRect.w;
	int srcRectYH = srcRect.y + srcRect.h;

	int srcY = srcRect.y;
	float dstY = float(dstRect.y);
	for (auto [srcH, dstH] : hs) {
		if (dstH <= 0) continue;

		int srcX = srcRect.x;
		float dstX = float(dstRect.x);

		for (auto [srcW, dstW] : ws) {
			if (dstW <= 0) continue;

			SDL_Rect partSrcRect;
			partSrcRect.x = Math::inBounds(srcX, 0, srcRectXW - 1);
			partSrcRect.y = Math::inBounds(srcY, 0, srcRectYH - 1);
			partSrcRect.w = Math::inBounds(srcW, 0, srcRectXW - partSrcRect.x);
			partSrcRect.h = Math::inBounds(srcH, 0, srcRectYH - partSrcRect.y);

			SDL_Rect partDstRect = DisplayObject::buildIntRect(dstX, dstY, dstW, dstH, false);

			func(partSrcRect, partDstRect);

			srcX += srcW;
			dstX += dstW;
		}

		srcY += srcH;
		dstY += dstH;
	}
}

void DisplayObject::draw() const {
	if (!enable || globalAlpha <= 0 || !surface) return;
	if (getWidth() <= 0 || getHeight() <= 0) return;

	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));
	SDL_Rect clipIRect = DisplayObject::buildIntRect(clipRect.x, clipRect.y, clipRect.w, clipRect.h, false);
	SDL_Rect dstIRect = DisplayObject::buildIntRect(globalX, globalY, getWidth(), getHeight(), false);
	SDL_Point center = { int(calcedXanchor), int(calcedYanchor) };

	//simple way
	if (withoutCornerSizes(this)) {
		pushToRender(surface, globalRotate, intAlpha, globalClipping, clipIRect, crop, dstIRect, center);
		return;
	}

	//zoom with const corner-sizes:
	// 1 2 3      1 2 2 3
	// 4 5 6  ->  4 5 5 6
	// 7 8 9      4 5 5 6
	//            7 8 8 9

	auto drawPart = [&](const SDL_Rect &srcRect, const SDL_Rect &dstRect) {
		pushToRender(surface, globalRotate, intAlpha, globalClipping, clipIRect, srcRect, dstRect, center);
	};
	callForEachPart(this, crop, dstIRect, drawPart);
}


bool DisplayObject::transparentForMouse(int x, int y) const {
	if (!transparentForMouseChecks(x, y) || !surface) return true;

	const SDL_Rect &srcRect = crop;
	SDL_Rect dstRect = DisplayObject::buildIntRect(globalX, globalY, getWidth(), getHeight(), false);

	if (withoutCornerSizes(this)) {
		x = srcRect.x + x * srcRect.w / dstRect.w;
		y = srcRect.y + y * srcRect.h / dstRect.h;

		Uint32 color = Utils::getPixel(surface, x, y);
		Uint8 alpha = color & 0xFF;
		return alpha == 0;
	}

	int globalX = int(getGlobalX()) + x;
	int globalY = int(getGlobalY()) + y;

	bool calced = false;
	bool res = true;
	auto checkPart = [&](const SDL_Rect &partSrcRect, const SDL_Rect &partDstRect) {
		if (calced) return;
		if (!Algo::pointInRect(globalX, globalY, partDstRect)) return;

		//calc only for need (one) part:

		int xForDst = globalX - partDstRect.x;
		int yForDst = globalY - partDstRect.y;

		int xForSrc = partSrcRect.x + xForDst * partSrcRect.w / partDstRect.w;
		int yForSrc = partSrcRect.y + yForDst * partSrcRect.h / partDstRect.h;

		Uint32 color = Utils::getPixel(surface, xForSrc, yForSrc);
		Uint8 alpha = color & 0xFF;

		res = (alpha == 0);
		calced = true;
	};

	callForEachPart(this, srcRect, dstRect, checkPart);
	return res;
}

bool DisplayObject::transparentForMouseChecks(int x, int y) const {
	if (!enable || globalAlpha <= 0 || globalSkipMouse) return false;
	if (globalClipping && !Algo::pointInRect(globalX + float(x), globalY + float(y), clipRect)) return false;
	if (!Algo::pointInRect(x, y, { 0, 0, int(getWidth()), int(getHeight()) })) return false;

	return true;
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
