#include "image.h"

#include "media/image_manipulator.h"


Image::Image(Node *node, Screen *screen):
	Container(node, this, screen)
{}

void Image::updateRect(bool) {
	Container::updateRect();

	if (xsize <= 0 && surface) {
		xsizeIsTextureWidth = true;
		xsize = surface->w;
	}else {
		xsizeIsTextureWidth = false;
	}

	if (ysize <= 0 && surface) {
		ysizeIsTextureHeight = true;
		ysize = surface->h;
	}else {
		ysizeIsTextureHeight = false;
	}
}

void Image::updateTexture(bool skipError) {
	if (skipError && first_param.empty()) return;

	if (!surface || first_param != prevImagePath) {
		prevImagePath = first_param;

		surface = ImageManipulator::getImage(first_param);
		if (surface) {
			if (xsizeIsTextureWidth) xsize = surface->w;
			if (ysizeIsTextureHeight) ysize = surface->h;
		}
	}
}
