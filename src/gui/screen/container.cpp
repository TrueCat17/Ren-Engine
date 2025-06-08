#include "container.h"


#include "screen.h"

#include "text.h"
#include "textbutton.h"

#include "imagemap.h"
#include "hotspot.h"

#include "image.h"
#include "key.h"

#include "parser/syntax_checker.h"
#include "utils/utils.h"


Container::Container(Node *node, Container *screenParent, Screen *screen):
    Child(node, screenParent, screen)
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

	updateWidth();
	updateHeight();
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

		if (childCommand == "use") {
			Node *screenNode = Screen::getDeclared(childNode->params);
			if (!screenNode) {
				Utils::outError("Screen::show",
				                "Screen <%> is not defined\n%",
				                childNode->params, childNode->getPlace());
				continue;
			}else {
				child = new Screen(childNode, screen);
			}
		}else

		if (childCommand == "key") {
			child = new Key(childNode, screen);
		}else

		if (childCommand == "vbox" || childCommand == "hbox") {
			ContainerBox *tmp = new ContainerBox(childNode, nullptr, screen);
			tmp->screenParent = tmp;
			child = tmp;
		}else

		if (childCommand == "null") {
			Container *tmp = new Container(childNode, nullptr, screen);
			tmp->screenParent = tmp;
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
				Utils::outError("Container::addChildrenFromNode",
				                "Unknown type <%>\n%",
				                childCommand, childNode->getPlace());
			}
			continue;
		}

		child->setInBox(screenParent->getHasVBox(), screenParent->getHasHBox());

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


void Container::updateWidth() {
	if (getWidth() <= 0) {
		float width = 0;
		for (const DisplayObject *child : children) {
			if (child->enable) {
				width = std::max(width, child->getWidth());
			}
		}
		setWidthWithMinMax(width / globalZoomX);
	}
}
void Container::updateHeight() {
	if (getHeight() <= 0) {
		float height = 0;
		for (const DisplayObject *child : children) {
			if (child->enable) {
				height = std::max(height, child->getHeight());
			}
		}
		setHeightWithMinMax(height / globalZoomY);
	}
}
