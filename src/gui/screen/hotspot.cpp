#include "hotspot.h"

#include "imagemap.h"
#include "style.h"

#include "gv.h"

#include "media/music.h"
#include "media/py_utils.h"

#include "parser/node.h"

#include "utils/math.h"
#include "utils/utils.h"


Hotspot::Hotspot(Node *node, Screen *screen):
	Child(node, nullptr, screen)
{
	auto onLeftClick = [this](DisplayObject*) {
		const Node *activateSound = this->node->getProp("activate_sound");
		if (activateSound) {
			Music::play("button_click " + activateSound->params,
						activateSound->getFileName(), activateSound->getNumLine());
		}else {
			const Node *style = this->node->getProp("style");
			const String &styleName = style ? style->params : this->node->command;
			const std::string sound = PyUtils::getStr(Style::getProp(styleName, "activate_sound"));
			if (sound != "None") {
				Music::play("button_click '" + sound + "'",
							this->getFileName(), this->getNumLine());
			}
		}

		const Node *action = this->node->getProp("action");
		if (action) {
			PyUtils::exec(action->getFileName(), action->getNumLine(),
						  "exec_funcs(" + action->params + ")");
		}else {
			const Node *style = this->node->getProp("style");
			const String &styleName = style ? style->params : this->node->command;
			PyUtils::exec(this->getFileName(), this->getNumLine(),
						  "exec_funcs(style." + styleName + ".action)");
		}
	};
	auto onRightClick = [this](DisplayObject*) {
		const Node* alternate = this->node->getProp("alternate");
		if (alternate) {
			PyUtils::exec(alternate->getFileName(), alternate->getNumLine(),
						  "exec_funcs(" + alternate->params + ")");
		}else {
			const Node *style = this->node->getProp("style");
			const String &styleName = style ? style->params : this->node->command;
			PyUtils::exec(this->getFileName(), this->getNumLine(),
						  "exec_funcs(style." + styleName + ".alternate)");
		}
	};
	btnRect.init(this, onLeftClick, onRightClick);
}

void Hotspot::updatePos() {}
void Hotspot::updateSize() {}

void Hotspot::checkEvents() {
	const Imagemap* imageMap = static_cast<Imagemap*>(parent);

	SurfacePtr ground = imageMap->surface;
	scaleX = 1;
	scaleY = 1;
	int parentWidth = imageMap->getWidth();
	int parentHeight = imageMap->getHeight();
	if (ground && parentWidth && parentHeight) {
		scaleX = double(parentWidth) / ground->w;
		scaleY = double(parentHeight) / ground->h;
	}

	setPos(
		int(xcrop * (xcropIsDouble ? GV::width : 1) * scaleX),
		int(ycrop * (ycropIsDouble ? GV::height : 1) * scaleY)
	);
	setSize(
		int(wcrop * (wcropIsDouble ? GV::width : 1) * scaleX),
		int(hcrop * (hcropIsDouble ? GV::height : 1) * scaleY)
	);


	const String *styleName = nullptr;
	if (btnRect.mouseOvered != prevMouseOver) {
		const Node *style = node->getProp("style");
		styleName = style ? &style->params : &node->command;
	}

	if (btnRect.mouseOvered) {
		if (!prevMouseOver) {
			const Node *hoverSound = node->getProp("hover_sound");
			if (hoverSound) {
				Music::play("button_hover " + hoverSound->params, hoverSound->getFileName(), hoverSound->getNumLine());
			}else {
				const std::string sound = PyUtils::getStr(Style::getProp(*styleName, "hover_sound"));
				if (sound != "None") {
					Music::play("button_hover '" + sound + "'",
								getFileName(), getNumLine());
				}
			}

			const Node *hovered = node->getProp("hovered");
			if (hovered) {
				PyUtils::exec(hovered->getFileName(), hovered->getNumLine(), "exec_funcs(" + hovered->params + ")");
			}else {
				PyUtils::exec(getFileName(), getNumLine(),
							  "exec_funcs(style." + *styleName + ".hovered)");
			}
		}

		surface = imageMap->hover;
	}else {
		if (prevMouseOver) {
			const Node *unhovered = node->getProp("unhovered");
			if (unhovered) {
				PyUtils::exec(unhovered->getFileName(), unhovered->getNumLine(), "exec_funcs(" + unhovered->params + ")");
			}else {
				PyUtils::exec(getFileName(), getNumLine(),
							  "exec_funcs(style." + *styleName + ".unhovered)");
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

	x = getX() + x;
	y = getY() + y;
	if (x < getX() || x >= getX() + getWidth() || y < getY() || y >= getY() + getHeight()) return false;

	SDL_Rect rect = {x, y, imageMap->getDrawRect().w, imageMap->getDrawRect().h};

	SurfacePtr hover = imageMap->hover;
	Uint32 hoverPixel = Utils::getPixel(hover, rect, imageMap->getCropRect());

	Uint8 alpha = hoverPixel & 0xFF;
	return alpha > 0;
}

void Hotspot::draw() const {
	if (!enable || !surface || globalAlpha <= 0) return;

	SDL_Rect from = { int(rect.x / scaleX), int(rect.y / scaleY), int(rect.w / scaleX), int(rect.h / scaleY) };
	SDL_Rect to = { getGlobalX(), getGlobalY(), rect.w, rect.h };

	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));
	SDL_Point center = { int(xAnchor), int(yAnchor) };

	pushToRender(surface, globalRotate, intAlpha, &from, &to, &center);
}
