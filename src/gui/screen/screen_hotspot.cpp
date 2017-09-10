#include "screen_hotspot.h"

#include <iostream>

#include "screen_imagemap.h"

#include "gv.h"

#include "media/music.h"
#include "media/py_utils.h"

#include "parser/node.h"


ScreenHotspot::ScreenHotspot(Node *node):
	ScreenChild(node, this),
	rectStr(node->getFirstParam())
{
	auto onLeftClick = [this](DisplayObject*) {
		const NodeProp activateSound = this->node->getPropCode("activate_sound");
		const String &sound = activateSound.pyExpr;
		if (sound) {
			Music::play("button_click " + sound, this->getFileName(), activateSound.numLine);
		}

		const NodeProp action = this->node->getPropCode("action");
		const String &actionExpr = action.pyExpr;
		if (actionExpr) {
			PyUtils::exec(this->getFileName(), action.numLine, "exec_funcs(" + actionExpr + ")");
		}
	};
	auto onRightClick = [this](DisplayObject*) {
		const NodeProp alternate = this->node->getPropCode("alternate");
		const String &alternateExpr = alternate.pyExpr;
		if (alternateExpr) {
			PyUtils::exec(this->getFileName(), alternate.numLine, "exec_funcs(" + alternateExpr + ")");
		}
	};
	btnRect.init(this, onLeftClick, onRightClick);

	clearProps();
	needUpdateFields = false;
	setProp(ScreenProp::RECT, NodeProp::initPyExpr("' '.join(map(str, " + rectStr + "))", node->getNumLine()));
	setProp(ScreenProp::MOUSE, node->getPropCode("mouse"));
}

bool ScreenHotspot::checkAlpha(int x, int y) const {
	if (!parent) return false;

	x = (getGlobalX() + x) / scaleX;
	y = (getGlobalY() + y) / scaleY;

	if (x < getX() || x >= getX() + getWidth() || y < getY() || y >= getY() + getHeight()) return false;


	TexturePtr ground = parent->texture;
	Uint32 groundPixel = Utils::getPixel(ground, parent->getDrawRect(), parent->getCropRect());

	ScreenImagemap *imagemap = dynamic_cast<ScreenImagemap*>(parent);
	if (!imagemap) {
		Utils::outMsg("ScreenHotspot::checkAlpha", "Тип родителя должен быть ScreenImagemap");
		return true;
	}
	TexturePtr hover = imagemap->hover;
	Uint32 hoverPixel = Utils::getPixel(hover, parent->getDrawRect(), parent->getCropRect());

	return groundPixel != hoverPixel;
}

void ScreenHotspot::calculateProps() {
	ScreenChild::calculateProps();

	const String &rectStr = propValues.at(ScreenProp::RECT);
	std::vector<String> rectVec = rectStr.split(' ');
	if (rectVec.size() != 4) {
		enable = false;
		Utils::outMsg("ScreenHotspot::updateProps",
					  String() + "Ожидалось 4 параметра (x, y, width, height), получено " + rectVec.size());
		return;
	}

	const String &mouse = propValues.at(ScreenProp::MOUSE);
	btnRect.buttonMode = mouse == "True";

	TexturePtr ground = parent->texture;
	scaleX = 1;
	scaleY = 1;
	int parentWidth = parent->getWidth();
	int parentHeight = parent->getHeight();
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

	if (btnRect.mouseOvered) {
		if (!prevMouseOver) {
			const NodeProp hoverSound = node->getPropCode("hover_sound");
			const String &sound = hoverSound.pyExpr;
			if (sound) {
				Music::play("button_hover " + sound, getFileName(), hoverSound.numLine);
			}

			const NodeProp hovered = node->getPropCode("hovered");
			const String &hoveredExpr = hovered.pyExpr;
			if (hoveredExpr) {
				PyUtils::exec(getFileName(), hovered.numLine, "exec_funcs(" + hoveredExpr + ")");
			}
		}

		ScreenImagemap *imagemap = dynamic_cast<ScreenImagemap*>(parent);
		if (!imagemap) {
			Utils::outMsg("ScreenHotspot::update", "Тип родителя должен быть ScreenImagemap");
			return;
		}
		texture = imagemap->hover;
	}else {
		if (prevMouseOver) {
			const NodeProp unhovered = node->getPropCode("unhovered");
			const String &unhoveredExpr = unhovered.pyExpr;
			if (unhoveredExpr) {
				PyUtils::exec(getFileName(), unhovered.numLine, "exec_funcs(" + unhoveredExpr + ")");
			}
		}

		texture = nullptr;
	}
	prevMouseOver = btnRect.mouseOvered;

	if (btnRect.mouseLeftDown && isModal()) {
		btnRect.onLeftClick();
	}
	if (btnRect.mouseRightDown && isModal()) {
		btnRect.onRightClick();
	}
}

void ScreenHotspot::draw() const {
	if (!enable || !texture) return;

	SDL_Rect from = { int(rect.x / scaleX), int(rect.y / scaleY), int(rect.w / scaleX), int(rect.h / scaleY) };
	SDL_Rect to = { getGlobalX(), getGlobalY(), rect.w, rect.h };

	Uint8 intAlpha = Utils::inBounds(int(globalAlpha * 255), 0, 255);
	SDL_Point center = { int(xAnchor), int(yAnchor) };

	pushToRender(texture, intAlpha, &from, &to, globalRotate, &center);
}
