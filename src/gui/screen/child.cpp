#include "child.h"

#include "screen.h"
#include "style.h"

#include "parser/syntax_checker.h"

#include "media/py_utils.h"
#include "utils/scope_exit.h"
#include "utils/stage.h"
#include "utils/utils.h"


Child::Child(Node *node, Container *screenParent, Screen *screen):
    screen(screen),
    inited(false),
    inVBox(false),
    inHBox(false),
    selected(false),
    node(node),
    screenParent(screenParent)
{}

bool Child::isModal() const {
	return !Screen::hasModal() || screen->modal || screen->ignore_modal;
}

void Child::updateStyle() {
	Node *prevNode = node;
	PyObject *prevProps = props;
	ScopeExit se([this, prevProps, prevNode]() {
		this->props = prevProps;
		this->node = prevNode;
	});

	if (!isFakeContainer()) {
		if (node->command == "use") {
			node = Screen::getDeclared(node->params);
		}

		updateFuncs = ScreenUpdateFuncs::getFuncs(node);

		if (!style) {
			style = StyleManager::getByObject(this);
		}

		const std::vector<std::string> &propNames = SyntaxChecker::getScreenProps(node->command);

		props = PyUtils::tuple1;
		for (const std::string &prop : propNames) {
			PyObject *res = StyleManager::getProp(style, prop);

			ScreenUpdateFunc func = ScreenUpdateFuncs::getFunc(node->command, prop);
			if (!func) continue;

			PyTuple_SET_ITEM(props, 0, res);
			func(this, 0);
			PyTuple_SET_ITEM(props, 0, nullptr);
		}
	}

	bool needAddChildren = false;
	bool hasNonConstChild = false;
	for (const Node *child : node->children) {
		if (!child->isScreenConst) {
			if (!child->isScreenProp) {
				hasNonConstChild = true;
			}
			continue;
		}
		if (!child->isScreenProp) {
			needAddChildren = true;
			continue;
		}
		if (child->command == "has" || child->command == "pass") continue;

		PyObject *res = PyUtils::execRetObj(child->getFileName(), child->getNumLine(), child->params);
		if (!res) continue;

		ScreenUpdateFunc func = ScreenUpdateFuncs::getFunc(node->command, child->command);
		if (!func) continue;

		PyTuple_SET_ITEM(props, 0, res);
		func(this, 0);
		PyTuple_SET_ITEM(props, 0, nullptr);

		Py_DECREF(res);
	}
	if (!inited && needAddChildren && !hasNonConstChild) {
		static_cast<Container*>(this)->addChildrenFromNode();
	}

	updateSize();
	updatePos();
}

void Child::updateProps() {
	if (!inited || prevStyle != style) {
		updateStyle();
		prevStyle = style;
		inited = true;
	}

	if (!props || props == Py_None) return;
	if (!PyList_CheckExact(props) && ! PyTuple_CheckExact(props)) {
		std::string type = props->ob_type->tp_name;
		Utils::outError("Child::updateProps", "Expected list or tuple, got %", type);
		return;
	}


	enable = true;

	if (updateFuncs) {
		for (size_t i = 0; i < updateFuncs->size(); ++i) {
			ScreenUpdateFunc updateFunc = (*updateFuncs)[i];
			if (updateFunc) {
				updateFunc(this, i);
			}
		}
	}
}

void Child::updateZoom() {
	globalZoomX = (parent ? parent->getGlobalZoomX() : 1) * std::max(xzoom, float(1e-6));
	globalZoomY = (parent ? parent->getGlobalZoomY() : 1) * std::max(yzoom, float(1e-6));
}

void Child::updatePos() {
	calcedXanchor = xanchor * (xanchor_is_float ? getWidth() : globalZoomX);
	calcedYanchor = yanchor * (yanchor_is_float ? getHeight() : globalZoomY);

	if (!inHBox) {
		float parentGlobalZoomX = parent ? parent->getGlobalZoomX() : 1;
		float endXPos = xpos * (xpos_is_float ? parent->getWidth() : parentGlobalZoomX);
		float x = endXPos - calcedXanchor;
		setX(x);
	}
	if (!inVBox) {
		float parentGlobalZoomY = parent ? parent->getGlobalZoomY() : 1;
		float endYPos = ypos * (ypos_is_float ? parent->getHeight() : parentGlobalZoomY);
		float y = endYPos - calcedYanchor;
		setY(y);
	}
}


void Child::setWidthWithMinMax(float sizeWithoutZoom) {
	float min = xsize_min * float(xsize_min_is_float ? Stage::width : 1);
	float max = xsize_max * float(xsize_max_is_float ? Stage::width : 1);
	if (min > 0 && sizeWithoutZoom < min) sizeWithoutZoom = min;
	if (max > 0 && sizeWithoutZoom > max) sizeWithoutZoom = max;
	setWidth(sizeWithoutZoom * globalZoomX);
}
void Child::setHeightWithMinMax(float sizeWithoutZoom) {
	float min = ysize_min * float(ysize_min_is_float ? Stage::height : 1);
	float max = ysize_max * float(ysize_max_is_float ? Stage::height : 1);
	if (min > 0 && sizeWithoutZoom < min) sizeWithoutZoom = min;
	if (max > 0 && sizeWithoutZoom > max) sizeWithoutZoom = max;
	setHeight(sizeWithoutZoom * globalZoomY);
}

void Child::updateSize() {
	float width = xsize * float(xsize_is_float ? Stage::width : 1);
	setWidthWithMinMax(width);

	float height = ysize * float(ysize_is_float ? Stage::height : 1);
	setHeightWithMinMax(height);

	if (surface) {
		float x = xcrop * float(xcrop_is_float ? surface->w : 1);
		float y = ycrop * float(ycrop_is_float ? surface->h : 1);
		float w = wcrop * float(wcrop_is_float ? surface->w : 1);
		float h = hcrop * float(hcrop_is_float ? surface->h : 1);

		crop.x = int(std::round(x));
		crop.y = int(std::round(y));
		crop.w = int(std::round(x + w)) - crop.x;
		crop.h = int(std::round(y + h)) - crop.y;
	}
}

void Child::updateTexture() {}
void Child::checkEvents() {}
