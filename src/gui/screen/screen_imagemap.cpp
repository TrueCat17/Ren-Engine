#include "screen_imagemap.h"

#include "media/py_utils.h"
#include "parser/node.h"

ScreenImagemap::ScreenImagemap(Node *node):
	ScreenContainer(node, this)
{
	setProp("ground", node->getPropCode("ground"));
	setProp("hover", node->getPropCode("hover"));
}

void ScreenImagemap::calculateProps() {
	ScreenContainer::calculateProps();
}
void ScreenImagemap::updateTexture() {
	const String &newGroundPath = propValues.at("ground");
	const String &newHoverPath = propValues.at("hover");

	if (newHoverPath) {
		hoverPath = newHoverPath;
	}else
	if (groundPath != newGroundPath) {
		hoverPath = PyUtils::exec("CPP_EMBED: screen_imagemap.cpp", __LINE__, "im.MatrixColor('" + groundPath + "', im.matrix.contrast(1.5))", true);
	}
	groundPath = newGroundPath;

	texture = Utils::getTexture(groundPath);
	hover = Utils::getTexture(hoverPath);
}
