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
		inited = true;

		if (!screenParent || screenParent == this) {
			const std::vector<String> &propNames = SyntaxChecker::getScreenProps(node->command);

			String style = node->command;
			for (Node *child : node->children) {
				if (child->command == "style") {
					style = child->command;
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

			for (Node *child : node->children) {
				if (!child->isScreenProp || !child->isScreenConst) continue;

				PyObject *res = PyUtils::execRetObj(getFileName(), getNumLine(), child->params);
				PyTuple_SET_ITEM(props, 0, res);

				ScreenUpdateFunc func = ScreenNodeUtils::getUpdateFunc(node->command, child->command);
				if (func) {
					func(this, 0);
				}
			}
			updatePos();
			updateSize();
			updateGlobal();

			updateTexture(true);

			props = prevProps;
		}
	}

	if (!props || props == Py_None) return;

	enable = true;

	for (size_t i = 0; i < updateFuncs.size(); ++i) {
		ScreenUpdateFunc updateFunc = updateFuncs[i];
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

	for (Child *child : screenChildren) {
		if (child->enable) {
			child->updatePos();
		}
	}
}

void Child::updateSize() {
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

	for (Child *child : screenChildren) {
		if (child->enable) {
			child->updateSize();
		}
	}
}

void Child::updateTexture(bool) {}
