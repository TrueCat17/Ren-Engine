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

bool Hotspot::checkAlpha(int x, int y) const {
	if (!enable || globalAlpha <= 0) return false;

	const Imagemap *imageMap = dynamic_cast<Imagemap*>(parent);
	if (!imageMap) {
		Utils::outMsg("ScreenHotspot::checkAlpha", "Тип родителя должен быть ScreenImagemap");
		return false;
	}

	x = getX() + x;
	y = getY() + y;
	if (x < getX() || x >= getX() + getWidth() || y < getY() || y >= getY() + getHeight()) return false;

	SDL_Rect rect = {x, y, imageMap->getDrawRect().w, imageMap->getDrawRect().h};

	SurfacePtr hover = imageMap->hover;
	Uint32 hoverPixel = Utils::getPixel(hover, rect, imageMap->getCropRect());

	Uint8 alpha = hoverPixel & 255;
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
