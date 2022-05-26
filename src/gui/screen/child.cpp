#include "child.h"

#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "parser/syntax_checker.h"

#include "media/py_utils.h"
#include "utils/stage.h"
#include "utils/utils.h"


Child::Child(Node *node, Container *screenParent, Screen *screen):
	screen(screen),
	node(node),
	screenParent(screenParent)
{}

bool Child::isModal() const {
	return !Screen::hasModal() || (screen && screen->modal);
}

void Child::updateProps() {
	if (!inited) {
		Node *prevNode = node;
		PyObject *prevProps = props;

		if (!isFakeContainer()) {
			if (node->command == "use") {
				node = Screen::getDeclared(node->params);
			}

			updateFuncs = ScreenNodeUtils::getUpdateFuncs(node);

			std::string style = node->command;
			for (const Node *child : node->children) {
				if (child->command == "has") {
					style = child->params;
					break;
				}
			}
			for (const Node *child : node->children) {
				if (child->command == "style") {
					style = child->params;
					break;
				}
			}
			bool defaultStyle = style == node->command;

			const std::vector<std::string> &propNames = SyntaxChecker::getScreenProps(node->command);

			props = PyUtils::tuple1;
			for (const std::string &prop : propNames) {
				PyObject *res = Style::getProp(style, prop);
				if (res == Py_None && !defaultStyle) {
					res = Style::getProp(node->command, prop); // try find in default style
				}
				PyTuple_SET_ITEM(props, 0, res);

				ScreenUpdateFunc func = ScreenNodeUtils::getUpdateFunc(node->command, prop);
				if (func) {
					func(this, 0);
				}
				PyTuple_SET_ITEM(props, 0, nullptr);
			}
		}

		bool needAddChildren = false;
		bool hasNotConstChild = false;
		for (const Node *child : node->children) {
			if (!child->isScreenConst) {
				if (!child->isScreenProp) {
					hasNotConstChild = true;
				}
				continue;
			}
			if (!child->isScreenProp) {
				needAddChildren = true;
				continue;
			}
			if (child->command == "style" || child->command == "has" || child->command == "pass") continue;

			PyObject *res = PyUtils::execRetObj(getFileName(), getNumLine(), child->params);
			if (!res) continue;

			PyTuple_SET_ITEM(props, 0, res);

			ScreenUpdateFunc func = ScreenNodeUtils::getUpdateFunc(node->command, child->command);
			if (func) {
				func(this, 0);
			}
		}
		if (needAddChildren && !hasNotConstChild) {
			static_cast<Container*>(this)->addChildrenFromNode();
		}

		updateRect();
		updateTexture(true);

		props = prevProps;
		node = prevNode;

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

	updateTexture();
}

void Child::updateZoom() {
	globalZoomX = (parent ? parent->getGlobalZoomX() : 1) * xzoom;
	globalZoomY = (parent ? parent->getGlobalZoomY() : 1) * yzoom;
}

void Child::updatePos() {
	xAnchor = xanchorPre * (xanchorPreIsFloat ? getWidth() : globalZoomX);
	yAnchor = yanchorPre * (yanchorPreIsFloat ? getHeight() : globalZoomY);

	if (!inHBox) {
		float parentGlobalZoomX = parent ? parent->getGlobalZoomX() : 1;
		float endXPos = xpos * (xposIsFloat ? parent->getWidth() : parentGlobalZoomX);
		float x = endXPos - xAnchor;
		setX(x);
	}
	if (!inVBox) {
		float parentGlobalZoomY = parent ? parent->getGlobalZoomY() : 1;
		float endYPos = ypos * (yposIsFloat ? parent->getHeight() : parentGlobalZoomY);
		float y = endYPos - yAnchor;
		setY(y);
	}
}

void Child::updateRect(bool needUpdatePos) {
	setWidth( xsize * float(xsizeIsFloat ? Stage::width  : 1) * globalZoomX);
	setHeight(ysize * float(ysizeIsFloat ? Stage::height : 1) * globalZoomY);

	if (surface) {
		crop.x = int(xcrop * float(xcropIsFloat ? surface->w : 1));
		crop.y = int(ycrop * float(ycropIsFloat ? surface->h : 1));
		crop.w = int(wcrop * float(wcropIsFloat ? surface->w : 1));
		crop.h = int(hcrop * float(hcropIsFloat ? surface->h : 1));
	}

	if (needUpdatePos) {
		updatePos();
	}
}

void Child::updateTexture(bool) {}
void Child::checkEvents() {}
