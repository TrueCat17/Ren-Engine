#include "imagemap.h"

#include "media/image_manipulator.h"
#include "media/py_utils.h"
#include "utils/utils.h"


Imagemap::Imagemap(Node *node, Screen *screen):
    Container(node, this, screen)
{}

void Imagemap::updateTexture() {
	Container::updateTexture();

	if (prevGroundPath == groundPath && prevHoverPath == hoverPath && surface) return;

	prevGroundPath = groundPath;
	prevHoverPath = hoverPath;

	if (hoverPath.empty()) {
		hoverPath = PyUtils::exec("CPP_EMBED: imagemap.cpp", __LINE__,
		                          "im.MatrixColor(r'" + groundPath + "', im.matrix.brightness(0.1))", true);
	}

	surface = ImageManipulator::getImage(groundPath, false);
	if (!surface) {
		Utils::outMsg("Imagemap::updateTexture",
		              "Failed to load ground image <" + groundPath + ">\n" +
		              node->getPlace());
	}
	hover = ImageManipulator::getImage(hoverPath, false);
	if (!hover) {
		Utils::outMsg("Imagemap::updateTexture",
		              "Failed to load hover image <" + hoverPath + ">\n" +
		              node->getPlace());
	}
}
