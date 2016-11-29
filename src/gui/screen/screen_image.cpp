#include "screen_image.h"

#include "utils/utils.h"

ScreenImage::ScreenImage(Node *node): ScreenContainer(node, this) {
	imageCode = node->getFirstParam();
}

void ScreenImage::updateProps() {
	ScreenContainer::updateProps();

	imagePath = Utils::execPython(imageCode, true);
}

void ScreenImage::update() {
	texture = Utils::getTexture(imagePath);

	if (xSize <= 0) xSize = Utils::getTextureWidth(texture);
	if (ySize <= 0) ySize = Utils::getTextureHeight(texture);

	ScreenContainer::update();
}

void ScreenImage::draw() const {
	ScreenContainer::draw();
}
