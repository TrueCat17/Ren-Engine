#include "child.h"

#include <algorithm>
#include <set>

#include "gv.h"

#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "parser/node.h"
#include "parser/syntax_checker.h"

#include "media/py_utils.h"
#include "utils/utils.h"


Child::Child(Node *node, Container *screenParent, Screen *screen):
	screen(screen),
	node(node),
	screenParent(screenParent),
	updateFuncs(ScreenNodeUtils::getUpdateFuncs(node))
{}

bool Child::isModal() const {
	return !Screen::hasModal() || (screen && screen->modal);
}

void Child::updateProps() {
	if (!inited) {
		if (!isFakeContainer()) {
			const std::vector<String> &propNames = SyntaxChecker::getScreenProps(node->command);

			String style = node->command;
			for (Node *child : node->children) {
				if (child->command == "style") {
					style = child->params;
					break;
				}
			}

			PyObject *prevProps = props;
			props = PyUtils::tuple1;
			for (const String &prop : propNames) {
				if (prop == "style") continue;

				PyObject *res = Style::getProp(style, prop);
				PyTuple_SET_ITEM(props, 0, res);

				ScreenUpdateFunc func = ScreenNodeUtils::getUpdateFunc(node->command, prop);
				if (func) {
					func(this, 0);
				}
			}

			bool needAddChildren = false;
			for (Node *child : node->children) {
				if (!child->isScreenConst) continue;
				if (!child->isScreenProp) {
					needAddChildren = true;
					continue;
				}
				if (child->command == "style") continue;

				PyObject *res = PyUtils::execRetObj(getFileName(), getNumLine(), child->params);
				PyTuple_SET_ITEM(props, 0, res);

				ScreenUpdateFunc func = ScreenNodeUtils::getUpdateFunc(node->command, child->command);
				if (func) {
					func(this, 0);
				}
			}
			if (needAddChildren && node->isScreenConst) {
				static_cast<Container*>(this)->addChildrenFromNode();
			}

			updateRect();
			updateTexture(true);

			props = prevProps;
		}

		inited = true;
	}

	if (!props || props == Py_None) return;
	if (!PyList_CheckExact(props) && ! PyTuple_CheckExact(props)) {
		Utils::outMsg("Child::updateProps", String("Expected list or tuple, got ") + props->ob_type->tp_name);
		return;
	}


	enable = true;

	for (size_t i = 0; i < updateFuncs->size(); ++i) {
		ScreenUpdateFunc updateFunc = (*updateFuncs)[i];
		if (updateFunc) {
			updateFunc(this, i);
		}
	}

	updateTexture();
}

void Child::updatePos() {
	xAnchor = int(xanchorPre * (xanchorPreIsDouble ? getWidth() : 1));
	yAnchor = int(yanchorPre * (yanchorPreIsDouble ? getHeight() : 1));

	if (!inHBox) {
		int endXPos = int(xpos * (xposIsDouble ? parent->getWidth() : 1));
		int x = endXPos - xAnchor;
		setX(x);
	}
	if (!inVBox) {
		int endYPos = int(ypos * (yposIsDouble ? parent->getHeight() : 1));
		int y = endYPos - yAnchor;
		setY(y);
	}
}

void Child::updateRect(bool needUpdatePos) {
	setSize(
		int(xsize * (xsizeIsDouble ? GV::width : 1)),
		int(ysize * (ysizeIsDouble ? GV::height : 1))
	);

	if (surface) {
		crop.x = int(xcrop * (xcropIsDouble ? surface->w : 1));
		crop.y = int(ycrop * (ycropIsDouble ? surface->h : 1));
		crop.w = int(hcrop * (wcropIsDouble ? surface->w : 1));
		crop.h = int(wcrop * (hcropIsDouble ? surface->h : 1));
	}

	if (needUpdatePos) {
		updatePos();
	}
}

void Child::updateTexture(bool) {}
void Child::checkEvents() {}
