#include "screen_container.h"

#include "screen.h"

#include "screen_text.h"
#include "screen_textbutton.h"

#include "screen_if.h"
#include "screen_elif.h"
#include "screen_else.h"

#include "screen_break.h"
#include "screen_continue.h"

#include "screen_for.h"
#include "screen_while.h"

#include "screen_null.h"
#include "screen_vbox.h"
#include "screen_hbox.h"

#include "screen_imagemap.h"
#include "screen_hotspot.h"

#include "screen_image.h"
#include "screen_key.h"
#include "screen_python.h"


#include "media/py_utils.h"

#include "utils/algo.h"
#include "utils/utils.h"


ScreenContainer::ScreenContainer(Node *node, ScreenContainer *screenParent, Screen *screen):
	ScreenChild(node, screenParent, screen)
{
	setProp(ScreenProp::SPACING, node->getPropCode("spacing"));

	preparationToUpdateCalcProps();
}

void ScreenContainer::calculateProps() {
	if (!inited) {
		addChildrenFromNode();
		inited = true;
	}

	ScreenChild::calculateProps();
	if (!isFakeContainer() && (hasVBox || hasHBox)) {
		if (propWasChanged[ScreenProp::SPACING]) {
			propWasChanged[ScreenProp::SPACING] = false;

			py::object &indentObj = propValues[ScreenProp::SPACING];
			bool isInt = PyUtils::isInt(indentObj);
			bool isFloat = !isInt && PyUtils::isFloat(indentObj);

			if (isInt || isFloat) {
				indent = PyUtils::getDouble(indentObj, isFloat);
			}else {
				indent = 0;
				Utils::outMsg("ScreenContainer::calculateProps",
							  "indent is not a number (" + PyUtils::getStr(indentObj) + ")\n" + node->getPlace());
			}
		}
	}
}

void ScreenContainer::updateSize() {
	ScreenChild::updateSize();

	//Установлено через [x/y/]size
	int userWidth = getWidth();
	int userHeight = getHeight();

	using std::max;

	int width;
	if (hasHBox) {
		width = 0;
		for (DisplayObject *child : children) {
			ScreenChild *scrChild = static_cast<ScreenChild*>(child);
			if (!scrChild->enable || scrChild->isFakeContainer()) continue;

			child->setX(width);

			int w = child->getWidth();
			if (w > 0) {
				width += w + indent;
			}
		}
		if (width) {
			width -= indent;
		}
	}else if (userWidth > 0) {
		width = userWidth;
	}else {
		width = 0;
		for (DisplayObject *child : children) {
			if (child->enable) {
				width = max(width, child->getWidth());
			}
		}
	}

	int height;
	if (hasVBox) {
		height = 0;
		for (DisplayObject *child : children) {
			ScreenChild *scrChild = static_cast<ScreenChild*>(child);
			if (!scrChild->enable || scrChild->isFakeContainer()) continue;

			child->setY(height);

			int h = child->getHeight();
			if (h > 0) {
				height += h + indent;
			}
		}
		if (height) {
			height -= indent;
		}
	}else if (userHeight > 0) {
		height = userHeight;
	}else {
		height = 0;
		for (DisplayObject *child : children) {
			if (child->enable) {
				height = max(height, child->getHeight());
			}
		}
	}

	setSize(width, height);
}

void ScreenContainer::draw() const {
	DisplayObject::draw();

	for (DisplayObject *child : children) {
		ScreenChild *scrChild = static_cast<ScreenChild*>(child);
		if (scrChild->enable && !scrChild->isFakeContainer()) {
			child->draw();
		}
	}
}

bool ScreenContainer::prevContainersSkipped() const {
	ScreenContainer *t = prevContainer;
	while (t) {
		if (!t->skipped) return false;
		t = t->prevContainer;
	}
	return true;
}

void ScreenContainer::addChildrenFromNode() {
	if (!inited) {
		countInitChildren = node->children.size();
	}

	ScreenContainer *prev = nullptr;
	for (Node *childNode : node->children) {
		ScreenChild *child = nullptr;
		const String &childCommand = childNode->command;

		if (childCommand == "has") {
			if (!inited) {
				--countInitChildren;
			}

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
			child = new ScreenKey(childNode, screen);
		}else

		if (childCommand == "vbox") {
			child = new ScreenVBox(childNode, screen);
		}else

		if (childCommand == "hbox") {
			child = new ScreenHBox(childNode, screen);
		}else

		if (childCommand == "null") {
			child = new ScreenNull(childNode, screen);
		}else

		if (childCommand == "imagemap") {
			child = new ScreenImagemap(childNode, screen);
		}else

		if (childCommand == "hotspot") {
			child = new ScreenHotspot(childNode, screen);
		}else

		if (childCommand == "image") {
			child = new ScreenImage(childNode, screen);
		}else

		if (childCommand == "text") {
			child = new ScreenText(childNode, screen);
		}else

		if (childCommand == "button" || childCommand == "textbutton") {//button is textbutton with empty text
			child = new ScreenTextButton(childNode, screen);
		}else

		if (childCommand == "if") {
			child = new ScreenIf(childNode, screenParent, screen);
			prev = static_cast<ScreenContainer*>(child);
		}else

		if (childCommand == "elif") {
			child = new ScreenElif(childNode, screenParent, screen, prev);
			prev = static_cast<ScreenContainer*>(child);
		}else

		if (childCommand == "else") {
			child = new ScreenElse(childNode, screenParent, screen, prev);
		}else

		if (childCommand == "for") {
			child = new ScreenFor(childNode, screenParent, screen);
			prev = static_cast<ScreenContainer*>(child);
		}else

		if (childCommand == "while") {
			child = new ScreenWhile(childNode, screenParent, screen);
			prev = static_cast<ScreenContainer*>(child);
		}else

		if (childCommand == "break") {
			child = new ScreenBreak(childNode);
		}else

		if (childCommand == "continue") {
			child = new ScreenContinue(childNode);
		}else

		if (childCommand == "$" || childCommand == "python") {
			child = new ScreenPython(childNode, childCommand != "$");
		}else

		{
			if (!inited) {
				--countInitChildren;
			}

			static const std::vector<String> props = String(
				"pass, "
				"has, spacing, "
				"xalign, yalign, xanchor, yanchor, xpos, ypos, xsize, ysize, "
				"align, anchor, pos, size, crop, rotate, alpha, "
				"modal, zorder, ground, hover, "
				"action, alternate, hovered, unhovered, activate_sound, hover_sound, mouse, "
				"color, font, text_size, text_align, text_valign"
			).split(", ");

			if (!Algo::in(childNode->command, props)) {
				Utils::outMsg("ScreenContainer::addChildrenFromNode",
							  "Неизвестный тип потомка <" + childNode->command + ">\n" +
							  childNode->getPlace());
			}
		}

		if (child) {
			child->propsUpdater = this;
			child->setInBox(screenParent->hasVBox, screenParent->hasHBox);

			int index;
			if (screenParent == this) {
				index = children.size();
			}else {
				ScreenChild *prev = this;
				while (prev->isFakeContainer() && prev->screenChildren.size()) {
					prev = prev->screenChildren.back();
				}
				index = screenParent->getChildIndex(prev) + 1;
			}

			addChildAt(child, index);
			screenChildren.push_back(child);
		}
	}
}

void ScreenContainer::addChildAt(DisplayObject *child, size_t index) {
	if (child == screenParent) {
		Utils::outMsg("ScreenContainer::addChild(At)", "Добавление объекта в самого себя");
		return;
	}

	Group *parent = screenParent;
	auto &pChildren = parent->children;

	if (child->parent) {
		child->parent->removeChild(child);
	}
	auto to = std::min(pChildren.begin() + index, pChildren.end());
	pChildren.insert(to, child);

	child->parent = parent;
	child->updateGlobalPos();
}
