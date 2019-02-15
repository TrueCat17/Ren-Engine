#include "container.h"

#include <set>


#include "screen.h"

#include "text.h"
#include "textbutton.h"

#include "imagemap.h"
#include "hotspot.h"

#include "image.h"
#include "key.h"


#include "media/py_utils.h"

#include "utils/string.h"
#include "utils/utils.h"


Container::Container(Node *node, Container *screenParent, Screen *screen):
	Child(node, screenParent, screen)
{}


void Container::addChildAt(DisplayObject *child, size_t index) {
	Group *parent = screenParent;
	auto &pChildren = parent->children;

	if (child->parent) {
		child->parent->removeChild(child);
	}
	auto to = std::min(pChildren.begin() + int(index), pChildren.end());
	pChildren.insert(to, child);

	child->parent = parent;
}


void Container::updateRect(bool) {
	for (Child *child : screenChildren) {
		if (child->enable) {
			child->updateRect();
		}else
		if (child->node->isScreenConst) {
			child->enable = true;
			child->updateRect();
		}
	}
	Child::updateRect(false);

	using std::max;

	if (!hasHBox) {
		if (getWidth() <= 0) {
			int width = 0;
			for (DisplayObject *child : children) {
				if (child->enable) {
					width = max(width, child->getWidth());
				}
			}
			setWidth(width);
		}
	}else {
		int width = 0;
		for (DisplayObject *child : children) {
			if (!child->enable) continue;

			child->setX(width);

			int w = child->getWidth();
			if (w > 0) {
				width += w + spacing;
			}
		}
		if (width) {
			width -= spacing;
		}
		setWidth(width);
	}

	if (!hasVBox) {
		if (getHeight() <= 0) {
			int height = 0;
			for (DisplayObject *child : children) {
				if (child->enable) {
					height = max(height, child->getHeight());
				}
			}
			setHeight(height);
		}
	}else {
		int height = 0;
		for (DisplayObject *child : children) {
			if (!child->enable) continue;

			child->setY(height);

			int h = child->getHeight();
			if (h > 0) {
				height += h + spacing;
			}
		}
		if (height) {
			height -= spacing;
		}
		setHeight(height);
	}

	updatePos();
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
			}
		}else

		if (childCommand == "use") {
			Node *scrNode = Screen::getDeclared(childNode->params);
			if (!scrNode) {
				Utils::outMsg("Screen::show",
				              "Screen <" + childNode->params + "> is not defined\n" +
							  childNode->getPlace());
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

			if (childNode->command == "vbox") {
				tmp->hasVBox = true;
			}else
			if (childNode->command == "hbox") {
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
			static const std::vector<std::string> propsVec = String::split(
				"break, continue, $, python, "
				"pass, "
				"has, first_param, spacing, "
				"xalign, yalign, xanchor, yanchor, xpos, ypos, xsize, ysize, "
			    "align, anchor, pos, size, crop, rotate, alpha, clip, "
				"modal, zorder, ground, hover, "
				"action, alternate, hovered, unhovered, activate_sound, hover_sound, mouse, "
				"color, font, text_size, text_align, text_valign"
			, ", ");

			static const std::set<std::string> props = { propsVec.begin(), propsVec.end() };

			if (!props.count(childNode->command)) {
				Utils::outMsg("Container::addChildrenFromNode",
				              "Unknown type <" + childNode->command + ">\n" +
							  childNode->getPlace());
			}
		}

		if (child) {
			child->setInBox(screenParent->hasVBox, screenParent->hasHBox);

			size_t index;
			if (screenParent == this) {
				index = children.size();
			}else {
				Child *prev = this;
				while (prev->isFakeContainer() &&
					   static_cast<Container*>(prev)->screenChildren.size())
				{
					prev = static_cast<Container*>(prev)->screenChildren.back();
				}
				index = screenParent->getChildIndex(prev) + 1;
			}

			addChildAt(child, index);
			screenChildren.push_back(child);

			if (child->node->isScreenConst) {
				child->updateProps();
			}
		}
	}
}
