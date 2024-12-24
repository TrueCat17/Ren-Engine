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
	const Imagemap* imageMap = static_cast<Imagemap*>(parent);

	SurfacePtr ground = imageMap->surface;
	float parentWidth = imageMap->getWidth();
	float parentHeight = imageMap->getHeight();
	if (ground && parentWidth > 0 && parentHeight > 0) {
		scaleX = parentWidth / float(ground->w);
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
	surface = (btnRect.mouseOvered || selected) ? imageMap->hover : nullptr;
}


bool Hotspot::transparentForMouse(int x, int y) const {
	if (!enable || globalAlpha <= 0 || globalSkipMouse) return true;

	const Imagemap *imageMap = static_cast<Imagemap*>(parent);
	SurfacePtr ground = imageMap->surface;
	SurfacePtr hover = imageMap->hover;
	if (!ground || !hover) return true;

	float fx = float(x);
	float fy = float(y);

	if (globalClipping) {
		if (fx + globalX < clipRect.x ||
		    fy + globalY < clipRect.y ||
		    fx + globalX >= clipRect.x + clipRect.w ||
		    fy + globalY >= clipRect.y + clipRect.h
		) return true;
	}

	if (x < 0 || y < 0 || x >= int(getWidth()) || y >= int(getHeight())) return true;

	//sizes in prev frame (when called checkEvents)
	float parentWidth = float(ground->w) * scaleX;
	float parentHeight = float(ground->h) * scaleY;

	SDL_Rect rect = DisplayObject::buildIntRect(fx + getX(), fy + getY(), parentWidth, parentHeight, false);
	Uint32 color = Utils::getPixel(hover, rect, imageMap->crop);
	Uint8 alpha = color & 0xFF;
	return alpha == 0;
}

void Hotspot::draw() const {
	if (!enable || globalAlpha <= 0 || !surface) return;

	SDL_Rect from = buildIntRect(getX() / scaleX, getY() / scaleY, getWidth() / scaleX, getHeight() / scaleY, false);
	SDL_Rect to = buildIntRect(getGlobalX(), getGlobalY(), getWidth(), getHeight(), false);

	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));
	SDL_Rect clipIRect = DisplayObject::buildIntRect(clipRect.x, clipRect.y, clipRect.w, clipRect.h, false);
	SDL_Point center = { int(calcedXanchor), int(calcedYanchor) };

	pushToRender(surface, globalRotate, intAlpha, globalClipping, clipIRect, from, to, center);
}
