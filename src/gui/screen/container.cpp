#include "container.h"


#include "screen.h"

#include "text.h"
#include "textbutton.h"

#include "imagemap.h"
#include "hotspot.h"

#include "image.h"
#include "key.h"

#include "parser/syntax_checker.h"
#include "utils/stage.h"
#include "utils/utils.h"


Container::Container(Node *node, Container *screenParent, Screen *screen):
    Child(node, screenParent, screen),
    hasVBox(false),
    hasHBox(false)
{}


void Container::addChildAt(DisplayObject *child, uint32_t index) {
	child->removeFromParent();

	Group *parent = screenParent;
	auto &pChildren = parent->children;

	index = std::min(index, uint32_t(pChildren.size()));
	pChildren.insert(pChildren.begin() + long(index), child);

	child->index = index;
	for (size_t i = index + 1; i < pChildren.size(); ++i) {
		++pChildren[i]->index;
	}

	child->parent = parent;
}

void Container::updateProps() {
	Child::updateProps();
	for (Child *child : screenChildren) {
		if (child->node->isScreenConst) {
			child->enable = true;
			child->updateProps();
		}
	}
}

void Container::updateZoom() {
	Child::updateZoom();
	for (Child *child : screenChildren) {
		if (child->enable) {
			child->updateZoom();
		}
	}
}

void Container::updatePos() {
	Child::updatePos();
	for (Child *child : screenChildren) {
		if (child->enable) {
			child->updatePos();
		}
	}
}

void Container::updateSize() {
	Child::updateSize();
	for (Child *child : screenChildren) {
		if (child->enable) {
			child->updateSize();
		}
	}

	if (globalZoomX <= 0 || globalZoomY <= 0) {
		setWidth(0);
		setHeight(0);
		return;
	}

	if (!hasHBox) {
		if (getWidth() <= 0) {
			float width = 0;
			for (const DisplayObject *child : children) {
				if (child->enable) {
					width = std::max(width, child->getWidth());
				}
			}
			setWidthWithMinMax(width / globalZoomX);
		}
	}else {
		float size = spacing * float(spacing_is_float ? Stage::width : 1);
		float min = spacing_min * float(spacing_min_is_float ? Stage::width : 1);
		float max = spacing_max * float(spacing_max_is_float ? Stage::width : 1);
		if (min > 0 && size < min) size = min;
		if (max > 0 && size > max) size = max;
		float globalSpacing = size * globalZoomX;

		float width = 0;
		for (DisplayObject *child : children) {
			if (!child->enable) continue;

			child->setX(width);

			float w = child->getWidth();
			if (w > 0) {
				width += w + globalSpacing;
			}
		}
		if (width > 0) {
			width -= globalSpacing;
		}
		setWidthWithMinMax(width / globalZoomX);
	}

	if (!hasVBox) {
		if (getHeight() <= 0) {
			float height = 0;
			for (const DisplayObject *child : children) {
				if (child->enable) {
					height = std::max(height, child->getHeight());
				}
			}
			setHeightWithMinMax(height / globalZoomY);
		}
	}else {
		float size = spacing * float(spacing_is_float ? Stage::height : 1);
		float min = spacing_min * float(spacing_min_is_float ? Stage::height : 1);
		float max = spacing_max * float(spacing_max_is_float ? Stage::height : 1);
		if (min > 0 && size < min) size = min;
		if (max > 0 && size > max) size = max;
		float globalSpacing = size * globalZoomY;

		float height = 0;
		for (DisplayObject *child : children) {
			if (!child->enable) continue;

			child->setY(height);

			float h = child->getHeight();
			if (h > 0) {
				height += h + globalSpacing;
			}
		}
		if (height > 0) {
			height -= globalSpacing;
		}
		setHeightWithMinMax(height / globalZoomY);
	}
}

void Container::updateTexture() {
	for (Child *child : screenChildren) {
		if (child->enable) {
			child->updateTexture();
		}
	}
	Child::updateTexture();
}



void Container::addChildrenFromNode() {
	Node *node = this->node;
	if (node->command == "use") {
		node = Screen::getDeclared(node->params);
		if (!node) return;
	}

	for (Node *childNode : node->children) {
		Child *child = nullptr;
		const std::string &childCommand = childNode->command;

		if (childCommand == "has") {
			if (childNode->params == "vbox") {
				hasVBox = true;
			}else
			if (childNode->params == "hbox") {
				hasHBox = true;
			}else
			{//unreachable
				Utils::outMsg("Screen::show",
				              "<has> expected value <vbox> or <hbox>, got <" + childNode->params + ">\n" +
				              childNode->getPlace());
			}
			continue;
		}


		if (childCommand == "use") {
			Node *scrNode = Screen::getDeclared(childNode->params);
			if (!scrNode) {
				Utils::outMsg("Screen::show",
				              "Screen <" + childNode->params + "> is not defined\n" +
							  childNode->getPlace());
				continue;
			}else {
				child = new Screen(childNode, screen);
			}
		}else

		if (childCommand == "key") {
			child = new Key(childNode, screen);
		}else

		if (childCommand == "vbox" || childCommand == "hbox" || childCommand == "null") {
			Container *tmp = new Container(childNode, nullptr, screen);
			tmp->screenParent = tmp;

			if (childCommand == "vbox") {
				tmp->hasVBox = true;
			}else
			if (childCommand == "hbox") {
				tmp->hasHBox = true;
			}
			child = tmp;
		}else

		if (childCommand == "imagemap") {
			child = new Imagemap(childNode, screen);
		}else

		if (childCommand == "hotspot") {
			child = new Hotspot(childNode, screen);
		}else

		if (childCommand == "image") {
			child = new Image(childNode, screen);
		}else

		if (childCommand == "text") {
			child = new Text(childNode, screen);
		}else

		if (childCommand == "button" || childCommand == "textbutton") {//button is textbutton with empty text
			child = new TextButton(childNode, screen);
		}else

		if (childCommand == "if" || childCommand == "elif" || childCommand == "else" ||
			childCommand == "for" || childCommand == "while")
		{
			child = new Container(childNode, screenParent, screen);
		}else

		{
			if (!SyntaxChecker::isKnownScreenLeaf(childCommand)) {
				Utils::outMsg("Container::addChildrenFromNode",
				              "Unknown type <" + childCommand + ">\n" +
				              childNode->getPlace());
			}
			continue;
		}

		child->setInBox(screenParent->hasVBox, screenParent->hasHBox);

		uint32_t index;
		if (screenParent == this) {
			index = uint32_t(children.size());
		}else {
			Child *prev = this;
			while (prev->isFakeContainer() &&
			       !static_cast<Container*>(prev)->screenChildren.empty())
			{
				prev = static_cast<Container*>(prev)->screenChildren.back();
			}
			index = prev->index + 1;
		}

		addChildAt(child, index);
		screenChildren.push_back(child);

		if (child->node->isScreenConst) {
			child->updateProps();
		}
	}
}
