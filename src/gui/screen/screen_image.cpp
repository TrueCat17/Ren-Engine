#include "screen_image.h"

#include "media/py_utils.h"
#include "parser/node.h"


ScreenImage::ScreenImage(Node *node):
	ScreenContainer(node, this)
{
	setProp(ScreenProp::IMAGE_PATH, NodeProp::initPyExpr(node->getFirstParam(), getNumLine()));

	preparationToUpdateCalcProps();
}

void ScreenImage::calculateProps() {
	ScreenContainer::calculateProps();
}
void ScreenImage::updateTexture() {
	if (propWasChanged[ScreenProp::IMAGE_PATH]) {
		propWasChanged[ScreenProp::IMAGE_PATH] = false;

		texture = Utils::getTexture(PyUtils::getStr(propValues[ScreenProp::IMAGE_PATH]));

		if (xSizeIsTextureWidth) xSize = Utils::getTextureWidth(texture);
		if (ySizeIsTextureHeight) ySize = Utils::getTextureHeight(texture);
	}
}

void ScreenImage::updateSize() {
	if (xSize <= 0) {
		xSizeIsTextureWidth = true;
		xSize = Utils::getTextureWidth(texture);
	}else {
		xSizeIsTextureWidth = false;
	}

	if (ySize <= 0) {
		ySizeIsTextureHeight = true;
		ySize = Utils::getTextureHeight(texture);
	}else {
		ySizeIsTextureHeight = false;
	}

	ScreenContainer::updateSize();
}
