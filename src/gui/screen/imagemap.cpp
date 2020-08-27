#include "imagemap.h"

#include "media/image_manipulator.h"
#include "media/py_utils.h"

Imagemap::Imagemap(Node *node, Screen *screen):
	Container(node, this, screen)
{}

void Imagemap::updateRect(bool) {
	Container::updateRect();

	xsize = std::max(xsize, 0.0);
	ysize = std::max(ysize, 0.0);
}

void Imagemap::updateTexture(bool skipError) {
	if (skipError && groundPath.empty()) return;

	if (!surface || !hover || prevGroundPath != groundPath || prevHoverPath != hoverPath) {
		if (prevGroundPath != groundPath && hoverPath.empty()) {
			hoverPath = PyUtils::exec("CPP_EMBED: imagemap.cpp", __LINE__, "im.MatrixColor(r'" + groundPath + "', im.matrix.contrast(1.5))", true);
		}
		prevGroundPath = groundPath;
		prevHoverPath = hoverPath;

		surface = ImageManipulator::getImage(groundPath, false);
		hover = ImageManipulator::getImage(hoverPath, false);
		updateRect(false);
	}
}
