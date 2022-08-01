#include "hotspot.h"

#include "imagemap.h"
#include "style.h"

#include "media/music.h"
#include "media/py_utils.h"

#include "utils/stage.h"
#include "utils/utils.h"



static void onLeftClick(DisplayObject* owner) {
	const Hotspot *hotspot = static_cast<Hotspot*>(owner);
	const Node *node = hotspot->node;
	const StyleStruct *style = hotspot->style;

	const Node *activateSound = node->getProp("activate_sound");
	if (activateSound) {
		Music::play("button_click " + activateSound->params,
		            activateSound->getFileName(), activateSound->getNumLine());
	}else {
		PyObject *activateSoundObj = Style::getProp(style, "activate_sound");

		if (PyString_CheckExact(activateSoundObj)) {
			const char *sound = PyString_AS_STRING(activateSoundObj);
			Music::play("button_click '" + std::string(sound) + "'",
			            node->getFileName(), node->getNumLine());
		}else if (activateSoundObj != Py_None) {
			std::string type = activateSoundObj->ob_type->tp_name;
			Utils::outMsg("Hotspot::onLeftClick",
			              "In style." + style->name + ".activate_sound expected type str, got " + type);
		}
	}

	const Node *action = node->getProp("action");
	if (action) {
		PyUtils::exec(action->getFileName(), action->getNumLine(),
		              "exec_funcs(" + action->params + ")");
	}else {
		Style::execAction(node->getFileName(), node->getNumLine(), style, "action");
	}
};

static void onRightClick(DisplayObject* owner) {
	const Hotspot *hotspot = static_cast<Hotspot*>(owner);
	const Node *node = hotspot->node;
	const StyleStruct *style = hotspot->style;

	const Node* alternate = node->getProp("alternate");
	if (alternate) {
		PyUtils::exec(alternate->getFileName(), alternate->getNumLine(),
		              "exec_funcs(" + alternate->params + ")");
	}else {
		Style::execAction(node->getFileName(), node->getNumLine(), style, "alternate");
	}
};

Hotspot::Hotspot(Node *node, Screen *screen):
	Child(node, nullptr, screen)
{
	btnRect.init(this, onLeftClick, onRightClick);
}

void Hotspot::updatePos() {}
void Hotspot::updateRect(bool) {}

void Hotspot::checkEvents() {
	const Imagemap* imageMap = static_cast<Imagemap*>(parent);

	SurfacePtr ground = imageMap->surface;
	scaleX = 1;
	scaleY = 1;
	float parentWidth = imageMap->getWidth();
	float parentHeight = imageMap->getHeight();
	if (ground && parentWidth > 0 && parentHeight > 0) {
		scaleX = parentWidth / float(ground->w);
		scaleY = parentHeight / float(ground->h);
	}

	setX(xcrop * float(xcropIsFloat ? Stage::width  : 1) * scaleX);
	setY(ycrop * float(ycropIsFloat ? Stage::height : 1) * scaleY);

	setWidth( wcrop * float(wcropIsFloat ? Stage::width  : 1) * scaleX);
	setHeight(hcrop * float(hcropIsFloat ? Stage::height : 1) * scaleY);


	if (btnRect.mouseOvered) {
		if (!prevMouseOver) {
			const Node *hoverSound = node->getProp("hover_sound");
			if (hoverSound) {
				Music::play("button_hover " + hoverSound->params,
							hoverSound->getFileName(), hoverSound->getNumLine());
			}else {
				PyObject *hoverSoundObj = Style::getProp(style, "hover_sound");

				if (PyString_CheckExact(hoverSoundObj)) {
					const char *sound = PyString_AS_STRING(hoverSoundObj);
					Music::play("button_hover '" + std::string(sound) + "'",
								getFileName(), getNumLine());
				}else if (hoverSoundObj != Py_None) {
					std::string type = hoverSoundObj->ob_type->tp_name;
					Utils::outMsg("Hotspot::hovered",
					              "In style." + style->name + ".hover_sound expected type str, got " + type);
				}
			}

			const Node *hovered = node->getProp("hovered");
			if (hovered) {
				PyUtils::exec(hovered->getFileName(), hovered->getNumLine(),
				              "exec_funcs(" + hovered->params + ")");
			}else {
				Style::execAction(node->getFileName(), node->getNumLine(), style, "hovered");
			}
		}

		surface = imageMap->hover;
	}else {
		if (prevMouseOver) {
			const Node *unhovered = node->getProp("unhovered");
			if (unhovered) {
				PyUtils::exec(unhovered->getFileName(), unhovered->getNumLine(),
				              "exec_funcs(" + unhovered->params + ")");
			}else {
				Style::execAction(node->getFileName(), node->getNumLine(), style, "unhovered");
			}
		}

		surface = nullptr;
	}
	prevMouseOver = btnRect.mouseOvered;

	if (isModal()) {
		if (btnRect.mouseLeftDown) {
			btnRect.onLeftClick();
		}
		if (btnRect.mouseRightDown) {
			btnRect.onRightClick();
		}
	}
}


bool Hotspot::checkAlpha(int x, int y) const {
	if (!enable || globalAlpha <= 0) return false;

	const Imagemap *imageMap = static_cast<Imagemap*>(parent);
	SurfacePtr hover = imageMap->hover;
	if (!hover) return false;

	float fx = float(x);
	float fy = float(y);

	if (globalClipping) {
		if (fx + globalX < clipRect.x ||
		    fy + globalY < clipRect.y ||
		    fx + globalX >= clipRect.x + clipRect.w ||
		    fy + globalY >= clipRect.y + clipRect.h
		) return false;
	}

	if (x < 0 || y < 0 || x >= int(getWidth()) || y >= int(getHeight())) return false;

	SDL_Rect rect = DisplayObject::buildIntRect(fx + getX(), fy + getY(), imageMap->getWidth(), imageMap->getHeight(), false);
	Uint32 color = Utils::getPixel(hover, rect, imageMap->crop);
	Uint8 alpha = color & 0xFF;
	return alpha > 0;
}

void Hotspot::draw() const {
	if (!enable || globalAlpha <= 0 || !surface) return;

	SDL_Rect from = { int(rect.x / scaleX), int(rect.y / scaleY), int(rect.w / scaleX), int(rect.h / scaleY) };
	SDL_Rect to = { int(getGlobalX()), int(getGlobalY()), int(rect.w), int(rect.h) };

	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));
	SDL_Rect clipIRect = DisplayObject::buildIntRect(clipRect.x, clipRect.y, clipRect.w, clipRect.h, false);
	SDL_Point center = { int(xAnchor), int(yAnchor) };

	pushToRender(surface, globalRotate, intAlpha, globalClipping, clipIRect, from, to, center);
}
