#include "screen_container.h"

#include <algorithm>


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


#include "utils/utils.h"


ScreenContainer::ScreenContainer(Node *node, ScreenChild *screenParent):
	ScreenChild(node, screenParent) { }

void ScreenContainer::updateProps() {
	if (!inited) {
		addChildrenFromNode();
		inited = true;
	}

	if (hasVBox || hasHBox) {
		String indentStr = node->getProp("spacing");
		indent = indentStr.toInt();
	}

	ScreenChild::updateProps();
}
void ScreenContainer::updateSize() {
	ScreenChild::updateSize();

	//Установлено через [x/y/xy]size
	int userWidth = getWidth();
	int userHeight = getHeight();

	int width = 0;
	if (hasHBox) {
		for (DisplayObject *child : children) {
			if (!child->enable) continue;
			ScreenChild *scrChild = dynamic_cast<ScreenChild*>(child);
			if (scrChild && scrChild->isFakeContainer()) continue;

			child->setX(width + indent);

			int w = child->getWidth();
			if (w) {
				width += w + indent;
			}
		}
		if (width && children.size()) {
			width -= indent;
		}
	}else {
		width = getWidth();
		if (!width) {
			for (DisplayObject *child : children) {
				if (child->enable) {
					int w = child->getWidth();
					if (w > width) {
						width = w;
					}
				}
			}
		}
	}

	int height = 0;
	if (hasVBox) {
		for (DisplayObject *child : children) {
			if (!child->enable) continue;
			ScreenChild *scrChild = dynamic_cast<ScreenChild*>(child);
			if (scrChild && scrChild->isFakeContainer()) continue;

			child->setY(height + indent);

			int h = child->getHeight();
			if (h) {
				height += h + indent;
			}
		}
		if (height && children.size()) {
			height -= indent;
		}
	}else {
		height = getHeight();
		if (!height) {
			for (DisplayObject *child : children) {
				if (child->enable) {
					int h = child->getHeight();
					if (h > height) {
						height = h;
					}
				}
			}
		}
	}

	if (userWidth <= 0 || userHeight <= 0) {
		if (userWidth > 0) width = userWidth;
		if (userHeight > 0) height = userHeight;
		setSize(width, height);
	}
}

void ScreenContainer::draw() const {
	DisplayObject::draw();

	for (DisplayObject *child : children) {
		if (!child->enable) continue;
		ScreenChild *scrChild = dynamic_cast<ScreenChild*>(child);
		if (scrChild && scrChild->isFakeContainer()) continue;

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

			DisplayObject *t = this;
			while (t && !dynamic_cast<Screen*>(t)) {
				t = t->parent;
			}
			Screen *scr = dynamic_cast<Screen*>(t);
			Screen::addToShow(childNode->params, scr ? scr->name : "");
		}else

		if (childCommand == "key") {
			child = new ScreenKey(childNode);
		}else

		if (childCommand == "vbox") {
			child = new ScreenVBox(childNode);
		}else

		if (childCommand == "hbox") {
			child = new ScreenHBox(childNode);
		}else

		if (childCommand == "null") {
			child = new ScreenNull(childNode);
		}else

		if (childCommand == "imagemap") {
			child = new ScreenImagemap(childNode);
		}else

		if (childCommand == "hotspot") {
			child = new ScreenHotspot(childNode);
		}else

		if (childCommand == "image") {
			child = new ScreenImage(childNode);
		}else

		if (childCommand == "text") {
			child = new ScreenText(childNode);
		}else

		if (childCommand == "button" || childCommand == "textbutton") {//button is textbutton with empty text
			child = new ScreenTextButton(childNode);
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
			child = new ScreenBreak(childNode);
		}else

		if (childCommand == "continue") {
			child = new ScreenContinue(childNode);
		}else

		if (childCommand == "command" || childCommand == "python") {
			child = new ScreenPython(childNode);
		}else

		{
			if (!inited) {
				--countInitChildren;
			}

			static const std::vector<String> props = String("has, spacing, xalign, yalign, xanchor, yanchor, xpos, ypos, xsize, ysize, "
															"align, anchor, pos, xysize, "
															"modal, background, hover_background, ground, hover, action, "
															"color, font, size, text_align").split(", ");

			if (!Utils::in(childNode->command, props)) {
				Utils::outMsg("ScreenContainer::addChildrenFromNode", "Неизвестный тип потомка <" + childNode->command + ">");
			}
		}

		if (child) {
			child->propsUpdater = this;

			Group *parent = screenParent;

			int index;
			if (parent == this) {
				index = children.size();
			}else {
				ScreenChild *prev = this;
				while (prev->isFakeContainer() && prev->screenChildren.size()) {
					prev = prev->screenChildren.back();
				}
				index = parent->getChildIndex(prev) + 1;
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
	child->updateGlobalX();
	child->updateGlobalY();
}
