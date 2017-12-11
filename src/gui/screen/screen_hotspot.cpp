#include "screen_hotspot.h"

#include "screen_imagemap.h"
#include "style.h"

#include "gv.h"

#include "media/music.h"
#include "media/py_utils.h"

#include "parser/node.h"


ScreenHotspot::ScreenHotspot(Node *node, Screen *screen):
	ScreenChild(node, this, screen),
	rectStr(node->getFirstParam())
{
	auto onLeftClick = [this](DisplayObject*) {
		const NodeProp activateSound = this->node->getPropCode("activate_sound");
		if (activateSound.pyExpr) {
			Music::play("button_click " + activateSound.pyExpr,
						this->getFileName(), activateSound.numLine);
		}else if (activateSound.styleName) {
			const std::string sound = PyUtils::getStr(Style::getProp(activateSound.styleName, "activate_sound"));
			if (sound != "None") {
				Music::play("button_click '" + sound + "'",
							this->getFileName(), this->getNumLine());
			}
		}

		const NodeProp action = this->node->getPropCode("action");
		if (action.pyExpr) {
			PyUtils::exec(this->getFileName(), action.numLine,
						  "exec_funcs(" + action.pyExpr + ")");
		}else if (action.styleName) {
			PyUtils::exec(this->getFileName(), this->getNumLine(),
						  "exec_funcs(style." + action.styleName + "." + action.propName + ")");
		}
	};
	auto onRightClick = [this](DisplayObject*) {
		const NodeProp alternate = this->node->getPropCode("alternate");
		if (alternate.pyExpr) {
			PyUtils::exec(this->getFileName(), alternate.numLine,
						  "exec_funcs(" + alternate.pyExpr + ")");
		}else if (alternate.styleName) {
			PyUtils::exec(this->getFileName(), this->getNumLine(),
						  "exec_funcs(style." + alternate.styleName + ".alternate)");
		}
	};
	btnRect.init(this, onLeftClick, onRightClick);

	clearProps();
	needUpdateFields = false;
	setProp(ScreenProp::RECTANGLE, NodeProp::initPyExpr(rectStr, node->getNumLine()));
	setProp(ScreenProp::MOUSE, node->getPropCode("mouse"));

	preparationToUpdateCalcProps();
}

bool ScreenHotspot::checkAlpha(int x, int y) const {
	if (!parent) return false;

	x = (getX() + x) / scaleX;
	y = (getY() + y) / scaleY;

	if (x < getX() || x >= getX() + getWidth() || y < getY() || y >= getY() + getHeight()) return false;


	ScreenImagemap *imagemap = dynamic_cast<ScreenImagemap*>(parent);
	if (!imagemap) {
		Utils::outMsg("ScreenHotspot::checkAlpha", "Тип родителя должен быть ScreenImagemap");
		return false;
	}

	SDL_Rect rect = {x, y, parent->getDrawRect().w, parent->getDrawRect().h};

	SurfacePtr hover = imagemap->hover;
	Uint32 hoverPixel = Utils::getPixel(hover, rect, parent->getCropRect());

	Uint8 alpha = hoverPixel & 0xFF;
	return alpha > 0;
}

void ScreenHotspot::calculateProps() {
	ScreenChild::calculateProps();

	py::object &rectObj = propValues[ScreenProp::RECTANGLE];
	bool ok = true;
	if (rectObj.is_none()) {
		ok = false;
	}else
	if ((PyUtils::isTuple(rectObj) || PyUtils::isList(rectObj)) && Py_SIZE(rectObj.ptr()) == 4) {
		for (size_t i = 0; i < 4; ++i) {
			if (!PyUtils::isInt(rectObj[i]) && !PyUtils::isFloat(rectObj[i])) {
				ok = false;
				break;
			}
		}
	}
	if (!ok) {
		enable = false;
		Utils::outMsg("ScreenHotspot::updateProps", String() +
					  "Ожидалось 4 параметра (x, y, width, height), получено \n" +
					  PyUtils::getStr(rectObj) + '\n' +
					  node->getPlace());
		return;
	}

	if (propWasChanged[ScreenProp::MOUSE]) {
		propWasChanged[ScreenProp::MOUSE] = false;

		std::lock_guard<std::mutex> g(PyUtils::pyExecMutex);
		py::object &mouseObj = propValues[ScreenProp::MOUSE];
		btnRect.buttonMode = py::extract<bool>(mouseObj);
	}

	SurfacePtr ground = parent->surface;
	scaleX = 1;
	scaleY = 1;
	int parentWidth = parent->getWidth();
	int parentHeight = parent->getHeight();
	if (ground && parentWidth && parentHeight) {
		scaleX = double(parentWidth) / ground->w;
		scaleY = double(parentHeight) / ground->h;
	}

	int x = PyUtils::getDouble(rectObj[0], PyUtils::isFloat(rectObj[0])) * scaleX;
	int y = PyUtils::getDouble(rectObj[1], PyUtils::isFloat(rectObj[1])) * scaleY;
	setPos(x, y);

	int w = PyUtils::getDouble(rectObj[2], PyUtils::isFloat(rectObj[2])) * scaleX;
	int h = PyUtils::getDouble(rectObj[3], PyUtils::isFloat(rectObj[3])) * scaleY;
	setSize(w, h);

	if (btnRect.mouseOvered) {
		if (!prevMouseOver) {
			const NodeProp hoverSound = node->getPropCode("hover_sound");
			if (hoverSound.pyExpr) {
				Music::play("button_hover " + hoverSound.pyExpr, getFileName(), hoverSound.numLine);
			}else if (hoverSound.styleName) {
				const std::string sound = PyUtils::getStr(Style::getProp(hoverSound.styleName, "hover_sound"));
				if (sound != "None") {
					Music::play("button_hover '" + sound + "'", getFileName(), getNumLine());
				}
			}

			const NodeProp hovered = node->getPropCode("hovered");
			if (hovered.pyExpr) {
				PyUtils::exec(getFileName(), hovered.numLine, "exec_funcs(" + hovered.pyExpr + ")");
			}else if (hovered.styleName) {
				PyUtils::exec(getFileName(), getNumLine(),
							  "exec_funcs(style." + hovered.styleName + ".hovered)");
			}
		}

		ScreenImagemap *imagemap = dynamic_cast<ScreenImagemap*>(parent);
		if (!imagemap) {
			Utils::outMsg("ScreenHotspot::calculateProps", "Тип родителя должен быть ScreenImagemap");
			return;
		}
		surface = imagemap->hover;
	}else {
		if (prevMouseOver) {
			const NodeProp unhovered = node->getPropCode("unhovered");
			if (unhovered.pyExpr) {
				PyUtils::exec(getFileName(), unhovered.numLine, "exec_funcs(" + unhovered.pyExpr + ")");
			}else if (unhovered.styleName) {
				PyUtils::exec(getFileName(), getNumLine(),
							  "exec_funcs(style." + unhovered.styleName + ".unhovered)");
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

void ScreenHotspot::draw() const {
	if (!enable || !surface) return;

	SDL_Rect from = { int(rect.x / scaleX), int(rect.y / scaleY), int(rect.w / scaleX), int(rect.h / scaleY) };
	SDL_Rect to = { getGlobalX(), getGlobalY(), rect.w, rect.h };

	Uint8 intAlpha = Utils::inBounds(int(globalAlpha * 255), 0, 255);
	SDL_Point center = { int(xAnchor), int(yAnchor) };

	pushToRender(surface, globalRotate, intAlpha, &from, &to, &center);
}
