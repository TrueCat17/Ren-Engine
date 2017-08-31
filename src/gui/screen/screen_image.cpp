#include "screen_image.h"

#include "media/py_utils.h"
#include "parser/node.h"


ScreenImage::ScreenImage(Node *node):
	ScreenContainer(node, this)
{
	setProp(ScreenProp::IMAGE_PATH, NodeProp::initPyExpr(node->getFirstParam(), getNumLine()));
}

void ScreenImage::calculateProps() {
	ScreenContainer::calculateProps();
}
void ScreenImage::updateTexture() {
	texture = Utils::getTexture(propValues.at(ScreenProp::IMAGE_PATH));
}

void ScreenImage::updateSize() {
	if (xSize <= 0) xSize = Utils::getTextureWidth(texture);
	if (ySize <= 0) ySize = Utils::getTextureHeight(texture);

	ScreenContainer::updateSize();
}
