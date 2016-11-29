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
#include "screen_window.h"

#include "screen_imagemap.h"
#include "screen_hotspot.h"

#include "screen_image.h"
#include "screen_key.h"
#include "screen_python.h"


#include "utils/utils.h"


ScreenContainer::ScreenContainer(Node *node, ScreenChild *screenParent):
	ScreenChild(node, screenParent) { }

int ScreenContainer::getMinX() const {
	if (hasHBox) {
		return 0;
	}
	return Group::getMinX();
}
int ScreenContainer::getMinY() const {
	if (hasVBox) {
		return 0;
	}
	return Group::getMinY();
}

int ScreenContainer::getMaxX() const {
	if (hasHBox) {
		int res = 0;
		for (DisplayObject *child : children) {
			if (!child->enabled()) continue;

			size_t w = child->getWidth();
			if (w) {
				res += w + indent;
			}
		}
		if (res && children.size()) {
			res -= indent;
		}
		return res;
	}
	return Group::getMaxX();
}
int ScreenContainer::getMaxY() const {
	if (hasVBox) {
		int res = 0;
		for (DisplayObject *child : children) {
			if (!child->enabled()) continue;

			size_t h = child->getHeight();
			if (h) {
				res += h + indent;
			}
		}
		if (res && children.size()) {
			res -= indent;
		}
		return res;
	}
	return Group::getMaxY();
}

void ScreenContainer::updateProps() {
	if (!inited) {
		addChildrenFromNode();
		inited = true;
	}

	String indentStr = node->getProp("spacing");
	indent = indentStr.toInt();

	ScreenChild::updateProps();
}
void ScreenContainer::draw() const {
	if (!enabled()) return;

	DisplayObject::draw();

	size_t indentX = 0;
	size_t indentY = 0;

	for (DisplayObject *child : children) {
		if (!child->enabled()) continue;

		if (hasVBox) {
			child->setY(indentY);
			size_t h = child->getHeight();
			if (h) {
				indentY += h + indent;
			}
		}
		if (hasHBox) {
			child->setX(indentX);
			size_t w = child->getWidth();
			if (w) {
				indentX += w + indent;
			}
		}
		child->draw();
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
		const String childCommand = childNode->command;

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
			if (!inited) --countInitChildren;

			Screen *scr = Screen::show(childNode->params, true);
			usedScreens.push_back(scr);
		}else

		if (childCommand == "key") {
			child = new ScreenKey(childNode);
		}else

		if (childCommand == "window") {
			child = new ScreenWindow(childNode);
			addChild(child);
		}else

		if (childCommand == "vbox") {
			child = new ScreenVBox(childNode);
			screenParent->addChild(child);
		}else

		if (childCommand == "hbox") {
			child = new ScreenHBox(childNode);
			screenParent->addChild(child);
		}else

		if (childCommand == "null") {
			child = new ScreenNull(childNode, screenParent);
		}else

		if (childCommand == "imagemap") {
			child = new ScreenImagemap(childNode);
			screenParent->addChild(child);
		}else

		if (childCommand == "hotspot") {
			child = new ScreenHotspot(childNode);
			screenParent->addChild(child);
		}else

		if (childCommand == "add" || childCommand == "image") {
			child = new ScreenImage(childNode);
			screenParent->addChild(child);
		}else

		if (childCommand == "text") {
			child = new ScreenText(childNode);
			screenParent->addChild(child);
		}else

		if (childCommand == "button" || childCommand == "textbutton") {//button is textbutton with empty text
			child = new ScreenTextButton(childNode);
			screenParent->addChild(child);
		}else

		if (childCommand == "if") {
			child = new ScreenIf(childNode, screenParent);
			prev = dynamic_cast<ScreenContainer*>(child);
		}else

		if (childCommand == "elif") {
			child = new ScreenElif(childNode, screenParent, prev);
			prev = dynamic_cast<ScreenContainer*>(child);
		}else

		if (childCommand == "else") {
			child = new ScreenElse(childNode, screenParent, prev);
		}else

		if (childCommand == "for") {
			child = new ScreenFor(childNode, screenParent);
			prev = dynamic_cast<ScreenContainer*>(child);
		}else

		if (childCommand == "while") {
			child = new ScreenWhile(childNode, screenParent);
			prev = dynamic_cast<ScreenContainer*>(child);
		}else

		if (childCommand == "break") {
			if (!inited) --countInitChildren;

			child = new ScreenBreak(childNode);
		}else

		if (childCommand == "continue") {
			if (!inited) --countInitChildren;

			child = new ScreenContinue(childNode);
		}else

		if (childCommand == "command" || childCommand == "python") {
			if (!inited) --countInitChildren;

			child = new ScreenPython(childNode);
		}else

		{
			if (!inited) {
				--countInitChildren;
			}

			static const std::vector<String> props = String("has, spacing, xalign, yalign, xanchor, yanchor, xpos, ypos, xsize, ysize, "
															"align, anchor, pos, xysize, "
															"background, hover_background, ground, hover, action, "
															"color, font, size, text_align").split(", ");

			if (!Utils::in(childNode->command, props)) {
				Utils::outMsg("ScreenContainer::addChildrenFromNode", "Неизвестный тип потомка <" + childNode->command + ">");
			}
		}

		if (child) {
			child->propsUpdater = this;
			screenChildren.push_back(child);
		}
	}
}

void ScreenContainer::addChildAt(DisplayObject *child, size_t index) {
	if (child == screenParent) {
		Utils::outMsg("ScreenChild::addChild(At)", "Добавление объекта в самого себя");
		return;
	}

	Group *parent = screenParent;
	if (!parent) {
		parent = this;
	}

	if (child->parent == parent) {
		if (parent->children[parent->children.size() - 1] != child) {
			for (size_t i = 0; i < parent->children.size(); ++i) {
				if (parent->children[i] == child) {
					parent->children.erase(parent->children.begin() + i);
					break;
				}
			}
			parent->children.push_back(child);
		}
		return;
	}

	if (child->parent) {
		child->parent->removeChild(child);
	}

	child->parent = parent;
	child->updateGlobalX();
	child->updateGlobalY();

	if (parent->children.size() < index) {
		parent->children.insert(parent->children.begin() + index, child);
	}else {
		parent->children.push_back(child);
	}
}

void ScreenContainer::removeChild(DisplayObject *child) {
	Group *parent = child->parent;

	for (size_t i = 0; i < parent->children.size(); ++i) {
		if (parent->children[i] == child) {
			parent->children.erase(parent->children.begin() + i);
			break;
		}
	}
}

void ScreenContainer::removeChildAt(size_t index) {
	DisplayObject *child = children[index];
	Group *parent = child->parent;

	parent->children.erase(parent->children.begin() + index);
}


ScreenContainer::~ScreenContainer() {
	for (ScreenContainer *obj : usedScreens) {
		Screen *scr = dynamic_cast<Screen*>(obj);
		Screen::hide(scr->name, true);
	}
	usedScreens.clear();
}
