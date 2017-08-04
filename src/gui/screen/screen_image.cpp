#include "screen_image.h"

#include "media/py_utils.h"
#include "parser/node.h"
#include "utils/utils.h"

ScreenImage::ScreenImage(Node *node):
	ScreenContainer(node, this),
	imageCode(node->getFirstParam())
{
	setProp("image_path", NodeProp::initPyExpr(imageCode, getNumLine()), true);
}

void ScreenImage::calculateProps() {
	ScreenContainer::calculateProps();
}
void ScreenImage::afterPriorityUpdate() {
	texture = Utils::getTexture(propValues["image_path"]);
}

void ScreenImage::updateSize() {
	if (xSize <= 0) xSize = Utils::getTextureWidth(texture);
	if (ySize <= 0) ySize = Utils::getTextureHeight(texture);

	ScreenContainer::updateSize();
}
