#include "screen_image.h"

#include "media/py_utils.h"
#include "media/image.h"
#include "parser/node.h"


ScreenImage::ScreenImage(Node *node, Screen *screen):
	ScreenContainer(node, this, screen)
{
	setProp(ScreenProp::IMAGE_PATH, NodeProp::initPyExpr(node->getFirstParam(), getNumLine()));

	preparationToUpdateCalcProps();
}

void ScreenImage::calculateProps() {
	ScreenContainer::calculateProps();
}
void ScreenImage::updateTexture() {
	if (!surface || propWasChanged[ScreenProp::IMAGE_PATH]) {
		propWasChanged[ScreenProp::IMAGE_PATH] = false;

		surface = Image::getImage(PyUtils::getStr(propValues[ScreenProp::IMAGE_PATH]));
		if (surface) {
			if (xSizeIsTextureWidth) xSize = surface->w;
			if (ySizeIsTextureHeight) ySize = surface->h;
		}
	}
}

void ScreenImage::updateSize() {
	if (xSize <= 0 && surface) {
		xSizeIsTextureWidth = true;
		xSize = surface->w;
	}else {
		xSizeIsTextureWidth = false;
	}

	if (ySize <= 0 && surface) {
		ySizeIsTextureHeight = true;
		ySize = surface->h;
	}else {
		ySizeIsTextureHeight = false;
	}

	ScreenContainer::updateSize();
}
