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

		const Style *defaultStyle = StyleManager::getByName(node, node->command);
		bool isDefaultStyle = style->pyStyle == defaultStyle->pyStyle;

		const std::vector<std::string> &propNames = SyntaxChecker::getScreenProps(node->command);

		props = PyUtils::tuple1;
		for (const std::string &prop : propNames) {
			PyObject *res = StyleManager::getProp(style, prop);
			if (res == Py_None && !isDefaultStyle) {
				res = StyleManager::getProp(defaultStyle, prop);
			}

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
	}
	if (!inited && needAddChildren && !hasNonConstChild) {
		static_cast<Container*>(this)->addChildrenFromNode();
	}

	updateRect();
}

void Child::updateProps() {
	if (!inited || prevStyle != style) {
		updateStyle();
		prevStyle = style;
		inited = true;
	}

	if (!props || props == Py_None) return;
	if (!PyList_CheckExact(props) && ! PyTuple_CheckExact(props)) {
		Utils::outMsg("Child::updateProps",
		              std::string("Expected list or tuple, got ") + props->ob_type->tp_name);
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
	globalZoomX = (parent ? parent->getGlobalZoomX() : 1) * xzoom;
	globalZoomY = (parent ? parent->getGlobalZoomY() : 1) * yzoom;
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

void Child::updateRect(bool needUpdatePos) {
	float size, min, max;

	size = xsize * float(xsize_is_float ? Stage::width : 1);
	min = xsize_min * float(xsize_min_is_float ? Stage::width : 1);
	max = xsize_max * float(xsize_max_is_float ? Stage::width : 1);
	if (min > 0 && size < min) size = min;
	if (max > 0 && size > max) size = max;
	setWidth(size * globalZoomX);

	size = ysize * float(ysize_is_float ? Stage::height : 1);
	min = ysize_min * float(ysize_min_is_float ? Stage::height : 1);
	max = ysize_max * float(ysize_max_is_float ? Stage::height : 1);
	if (min > 0 && size < min) size = min;
	if (max > 0 && size > max) size = max;
	setHeight(size * globalZoomY);

	if (surface) {
		crop.x = int(xcrop * float(xcrop_is_float ? surface->w : 1));
		crop.y = int(ycrop * float(ycrop_is_float ? surface->h : 1));
		crop.w = int(wcrop * float(wcrop_is_float ? surface->w : 1));
		crop.h = int(hcrop * float(hcrop_is_float ? surface->h : 1));
	}

	if (needUpdatePos) {
		updatePos();
	}
}

void Child::updateTexture() {}
void Child::checkEvents() {}
