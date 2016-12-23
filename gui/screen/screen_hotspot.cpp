#include "screen_hotspot.h"

#include "screen_imagemap.h"

#include "gv.h"
#include "parser/node.h"
#include "utils/utils.h"

ScreenHotspot::ScreenHotspot(Node *node):
	ScreenChild(node, this),
	rectStr(node->getFirstParam())
{
	auto onClick = [this](DisplayObject*) {
		const String action = this->node->getPropCode("action");
		if (action) {
			Utils::execPython("exec_funcs(" + action + ")");
		}
	};
	btnRect.init(this, onClick);
}

bool ScreenHotspot::checkAlpha(int x, int y) const {
	if (!parent) return false;

	x = (getGlobalX() + x) / scaleX;
	y = (getGlobalY() + y) / scaleY;

	SDL_Texture *ground = parent->texture;
	Uint32 groundPixel = Utils::getPixel(ground, x, y, 0, 0);

	ScreenImagemap *imagemap = dynamic_cast<ScreenImagemap*>(parent);
	if (!imagemap) {
		Utils::outMsg("ScreenHotspot::checkAlpha", "Тип родителя должен быть ScreenImagemap");
		return true;
	}
	SDL_Texture *hover = imagemap->hover;
	Uint32 hoverPixel = Utils::getPixel(hover, x, y, 0, 0);

	return groundPixel != hoverPixel;
}

void ScreenHotspot::updateProps() {
	String t = Utils::execPython("' '.join(map(str, " + rectStr + "))", true);
	std::vector<String> rectVec = t.split(' ');
	if (rectVec.size() != 4) {
		Utils::outMsg("ScreenHotspot::updateProps",
					  String() + "Ожидалось 4 параметра (x, y, width, height), получено " + rectVec.size());
		return;
	}

	enable = true;

	SDL_Texture *ground = parent->texture;
	scaleX = 1;
	scaleY = 1;
	int parentWidth, parentHeight;
	parent->getSize(parentWidth, parentHeight);
	if (ground && parentWidth && parentHeight) {
		scaleX = double(parentWidth) / Utils::getTextureWidth(ground);
		scaleY = double(parentHeight) / Utils::getTextureHeight(ground);
	}

	int x = rectVec[0].toInt() * scaleX;
	int y = rectVec[1].toInt() * scaleY;
	setPos(x, y);

	int w = rectVec[2].toInt() * scaleX;
	int h = rectVec[3].toInt() * scaleY;
	setSize(w, h);


	if (btnRect.mouseDown && isModal()) {
		btnRect.onClick();
	}
}

void ScreenHotspot::update() {
	if (btnRect.mouseOvered) {
		ScreenImagemap *imagemap = dynamic_cast<ScreenImagemap*>(parent);
		if (!imagemap) {
			Utils::outMsg("ScreenHotspot::update", "Тип родителя должен быть ScreenImagemap");
			return;
		}
		texture = imagemap->hover;
	}else {
		texture = nullptr;
	}
}

void ScreenHotspot::draw() const {
	if (!enabled() || !texture) return;

	int x = getGlobalX();
	int y = getGlobalY();

	SDL_Rect from = { int(x / scaleX), int(y / scaleY), int(rect.w / scaleX), int(rect.h / scaleY) };
	SDL_Rect to = { x, y, rect.w, rect.h };
	if (SDL_RenderCopy(GV::mainRenderer, texture, &from, &to)) {
		Utils::outMsg("SDL_RenderCopy", SDL_GetError());
	}
}
