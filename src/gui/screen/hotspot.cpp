#include "hotspot.h"

#include "imagemap.h"

#include "utils/stage.h"
#include "utils/utils.h"


Hotspot::Hotspot(Node *node, Screen *screen):
    Child(node, nullptr, screen),
    btnRect(this)
{ }

void Hotspot::updatePos() {}
void Hotspot::updateSize() {}

void Hotspot::checkEvents() {
	const Imagemap *imageMap = static_cast<Imagemap*>(parent);

	const SurfacePtr &ground = imageMap->surface;
	float parentWidth  = imageMap->getWidth();
	float parentHeight = imageMap->getHeight();
	float scaleX = 1;
	float scaleY = 1;
	if (ground && parentWidth > 0 && parentHeight > 0) {
		scaleX = parentWidth  / float(ground->w);
		scaleY = parentHeight / float(ground->h);
	}else {
		scaleX = 1;
		scaleY = 1;
	}

	setX(xcrop * float(xcrop_is_float ? Stage::width  : 1) * scaleX);
	setY(ycrop * float(ycrop_is_float ? Stage::height : 1) * scaleY);

	setWidth( wcrop * float(wcrop_is_float ? Stage::width  : 1) * scaleX);
	setHeight(hcrop * float(hcrop_is_float ? Stage::height : 1) * scaleY);

	btnRect.checkEvents();
	surface = (btnRect.isHovered() || selected) ? imageMap->hover : nullptr;
}


bool Hotspot::transparentForMouse(int x, int y) const {
	if (!transparentForMouseChecks(x, y)) return true;

	const Imagemap *imageMap = static_cast<Imagemap*>(parent);
	const SurfacePtr &hover = imageMap->hover;
	if (!hover) return true;

	const SDL_Rect &srcRect = parent->crop;
	SDL_Rect dstRect = { 0, 0, int(parent->getWidth()), int(parent->getHeight()) };

	x = srcRect.x + (int(getX()) + x) * srcRect.w / dstRect.w;
	y = srcRect.y + (int(getY()) + y) * srcRect.h / dstRect.h;

	Uint32 color = Utils::getPixel(hover, x, y);
	Uint8 alpha = color & 0xFF;
	return alpha == 0;
}

void Hotspot::draw() const {
	if (!enable || globalAlpha <= 0 || !surface) return;

	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));

	float x = globalX;
	float y = globalY;
	float w = getWidth();
	float h = getHeight();
	if (globalClipping) {
		x = std::max(clipRect.x, x);
		y = std::max(clipRect.y, y);
		w = std::min(clipRect.x + clipRect.w, globalX + w) - x;
		h = std::min(clipRect.y + clipRect.h, globalY + h) - y;
	}
	SDL_Rect clipIRect = DisplayObject::buildIntRect(x, y, w, h, false);

	SDL_Rect dstIRect = DisplayObject::buildIntRect(
	    parent->getGlobalX(),
	    parent->getGlobalY(),
	    parent->getWidth(),
	    parent->getHeight(),
	    false
	);

	SDL_Point center = { int(parent->calcedXanchor), int(parent->calcedYanchor) };

	pushToRender(surface, globalRotate, intAlpha, true, clipIRect, parent->crop, dstIRect, center);
}
