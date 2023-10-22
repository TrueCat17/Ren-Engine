#include "hotspot.h"

#include "imagemap.h"

#include "utils/stage.h"
#include "utils/utils.h"


Hotspot::Hotspot(Node *node, Screen *screen):
    Child(node, nullptr, screen),
    btnRect(this)
{ }

void Hotspot::updatePos() {}
void Hotspot::updateRect(bool) {}

void Hotspot::checkEvents() {
	if (alpha <= 0 || !isModal() || globalSkipMouse) {
		btnRect.mouseOvered = false;
		btnRect.mouseLeftDown = false;
		btnRect.mouseRightDown = false;
	}

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

	surface = (btnRect.mouseOvered || selected) ? imageMap->hover : nullptr;
	btnRect.checkEvents();
}


bool Hotspot::transparentForMouse(int x, int y) const {
	if (!enable || globalAlpha <= 0 || globalSkipMouse) return true;

	const Imagemap *imageMap = static_cast<Imagemap*>(parent);
	SurfacePtr hover = imageMap->hover;
	if (!hover) return true;

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

	SDL_Rect rect = DisplayObject::buildIntRect(fx + getX(), fy + getY(), imageMap->getWidth(), imageMap->getHeight(), false);
	Uint32 color = Utils::getPixel(hover, rect, imageMap->crop);
	Uint8 alpha = color & 0xFF;
	return alpha == 0;
}

void Hotspot::draw() const {
	if (!enable || globalAlpha <= 0 || !surface) return;

	SDL_Rect from = { int(rect.x / scaleX), int(rect.y / scaleY), int(rect.w / scaleX), int(rect.h / scaleY) };
	SDL_Rect to = { int(getGlobalX()), int(getGlobalY()), int(rect.w), int(rect.h) };

	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));
	SDL_Rect clipIRect = DisplayObject::buildIntRect(clipRect.x, clipRect.y, clipRect.w, clipRect.h, false);
	SDL_Point center = { int(calcedXanchor), int(calcedYanchor) };

	pushToRender(surface, globalRotate, intAlpha, globalClipping, clipIRect, from, to, center);
}
