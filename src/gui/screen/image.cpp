#include "image.h"

#include "media/image_manipulator.h"
#include "utils/utils.h"


Image::Image(Node *node, Screen *screen):
    Container(node, this, screen)
{}

void Image::updateTexture() {
	Container::updateTexture();

	if (first_param == prevImagePath) return;

	prevImagePath = first_param;

	surface = ImageManipulator::getImage(first_param, false);
	if (!surface) {
		Utils::outMsg("Image::updateTexture",
		              "Failed to load image <" + first_param + ">\n" +
		              node->getPlace());
	}
}
