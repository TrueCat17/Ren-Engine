#include "image.h"

#include "media/image_manipulator.h"


Image::Image(Node *node, Screen *screen):
	Container(node, this, screen)
{}

void Image::updateRect(bool) {
	Container::updateRect();

	if (xsize < 0) { xsize = 0; }
	if (ysize < 0) { ysize = 0; }
}

void Image::updateTexture(bool skipError) {
	if (skipError && first_param.empty()) return;

	if (!surface || first_param != prevImagePath) {
		prevImagePath = first_param;

		surface = ImageManipulator::getImage(first_param, false);
	}
}
