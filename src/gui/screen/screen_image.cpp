#include "screen_image.h"

#include "media/py_utils.h"
#include "parser/node.h"


ScreenImage::ScreenImage(Node *node):
	ScreenContainer(node, this),
	imageCode(node->getFirstParam())
{
	setProp("image_path", NodeProp::initPyExpr(imageCode, getNumLine()));
}

void ScreenImage::calculateProps() {
	ScreenContainer::calculateProps();
}
void ScreenImage::updateTexture() {
	texture = Utils::getTexture(propValues.at("image_path"));
}

void ScreenImage::updateSize() {
	if (xSize <= 0) xSize = Utils::getTextureWidth(texture);
	if (ySize <= 0) ySize = Utils::getTextureHeight(texture);

	ScreenContainer::updateSize();
}
