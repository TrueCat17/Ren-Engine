#include "screen_imagemap.h"

#include "media/py_utils.h"
#include "parser/node.h"
#include "utils/utils.h"

ScreenImagemap::ScreenImagemap(Node *node):
	ScreenContainer(node, this)
{ }

void ScreenImagemap::calculateProps() {
	String newGroundPath = node->getProp("ground");
	String newHoverPath = node->getProp("hover");

	if (newHoverPath) {
		hoverPath = newHoverPath;
	}else
	if (groundPath != newGroundPath) {
		hoverPath = PyUtils::exec("CPP_EMBED: screen_imagemap.cpp", __LINE__, "im.MatrixColor('" + groundPath + "', im.matrix.contrast(1.5))", true);
	}
	groundPath = newGroundPath;

	texture = Utils::getTexture(groundPath);
	hover = Utils::getTexture(hoverPath);

	ScreenContainer::calculateProps();
}
