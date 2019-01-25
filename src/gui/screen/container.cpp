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

#include "utils/utils.h"


Container::Container(Node *node, Container *screenParent, Screen *screen):
	Child(node, screenParent, screen)
{}


void Container::updateProps() {
	Child::updateProps();

	if (!props || props == Py_None) return;
	if (!PyList_CheckExact(props) && ! PyTuple_CheckExact(props)) {
		Utils::outMsg("Container::updateProps", String("Expected list or tuple, got ") + props->ob_type->tp_name);
		return;
	}

	const size_t countCalcs = size_t(Py_SIZE(props));
	const size_t countChildren = countCalcs - node->countPropsToCalc;
	while (countChildren > screenChildren.size()) {
		addChildrenFromNode();
	}


	if (node->countPropsToCalc) {
		for (Child *child : screenChildren) {
			size_t num = child->node->screenNum;
			if (num == size_t(-1)) {
				child->enable = true;
				continue;
			}

			child->props = PySequence_Fast_GET_ITEM(props, num);
			child->updateProps();
		}
	}else {
		size_t i = 0;
		for (Child *child : screenChildren) {
			if (child->node->screenNum == size_t(-1)) {
				child->enable = true;
				continue;
			}

			child->props = PySequence_Fast_GET_ITEM(props, i);
			child->updateProps();

			++i;
			if (i == countCalcs) break;
		}
	}
}

void Container::addChildrenFromNode() {
	for (Node *childNode : node->children) {
		Child *child = nullptr;
		const String &childCommand = childNode->command;

		if (childCommand == "has") {
			if (childNode->params == "vbox") {
				hasVBox = true;
			}else
			if (childNode->params == "hbox") {
				hasHBox = true;
			}
		}else

		if (childCommand == "use") {
			const String &scrName = childNode->params;
			Node *scrNode = Screen::getDeclared(scrName);
			if (!scrNode) {
				Utils::outMsg("Screen::show",
							  "Скрин с именем <" + scrName + "> не существует\n" +
							  childNode->getPlace());
			}else {
				child = new Screen(scrNode, screen);
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
			static const std::vector<String> propsVec = String(
				"break, continue, $, python, "
				"pass, "
				"has, first_param, spacing, "
				"xalign, yalign, xanchor, yanchor, xpos, ypos, xsize, ysize, "
				"align, anchor, pos, size, crop, rotate, alpha, "
				"modal, zorder, ground, hover, "
				"action, alternate, hovered, unhovered, activate_sound, hover_sound, mouse, "
				"color, font, text_size, text_align, text_valign"
			).split(", ");

			static const std::set<String> props = { propsVec.begin(), propsVec.end() };

			if (!props.count(childNode->command)) {
				Utils::outMsg("ScreenContainer::addChildrenFromNode",
							  "Неизвестный тип потомка <" + childNode->command + ">\n" +
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
				//prev->screenParent != prev mean that prev is fakeContainer (if/elif/else/while/for)
				while (prev->screenParent != prev && prev->screenChildren.size()) {
					prev = prev->screenChildren.back();
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

void Container::addChildAt(DisplayObject *child, size_t index) {
	if (child == screenParent) {
		Utils::outMsg("ScreenContainer::addChild(At)", "Добавление объекта в самого себя");
		return;
	}

	Group *parent = screenParent;
	auto &pChildren = parent->children;

	if (child->parent) {
		child->parent->removeChild(child);
	}
	auto to = std::min(pChildren.begin() + int(index), pChildren.end());
	pChildren.insert(to, child);

	child->parent = parent;
	child->updateGlobal();
}
