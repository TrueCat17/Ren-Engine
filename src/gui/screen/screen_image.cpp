#include "screen_image.h"

#include "media/py_utils.h"
#include "parser/node.h"
#include "utils/utils.h"

ScreenImage::ScreenImage(Node *node):
	ScreenContainer(node, this),
	imageCode(node->getFirstParam())
{ }

void ScreenImage::updateProps() {
	imagePath = PyUtils::exec(imageCode, true);
	texture = Utils::getTexture(imagePath);

	ScreenContainer::updateProps();
}

void ScreenImage::updateSize() {
	if (xSize <= 0) xSize = Utils::getTextureWidth(texture);
	if (ySize <= 0) ySize = Utils::getTextureHeight(texture);

	ScreenContainer::updateSize();
}
