#include "imagemap.h"

#include "style.h"

#include "media/image_manipulator.h"
#include "media/py_utils.h"

#include "utils/utils.h"

Imagemap::Imagemap(Node *node, Screen *screen):
	Container(node, this, screen)
{}

void Imagemap::updateRect(bool) {
	Container::updateRect(false);

	if (xsize < 0) { xsize = 0; }
	if (ysize < 0) { ysize = 0; }
}

void Imagemap::updateTexture(bool skipError) {
	if (skipError && groundPath.empty()) return;
	if (surface && hover && prevGroundPath == groundPath && prevHoverPath == hoverPath) return;

	if (hoverPath.empty() && (prevGroundPath != groundPath || prevHoverPath != hoverPath)) {
		hoverPath = PyUtils::exec("CPP_EMBED: imagemap.cpp", __LINE__,
		                          "im.MatrixColor(r'" + groundPath + "', im.matrix.brightness(0.1))", true);
	}
	prevGroundPath = groundPath;
	prevHoverPath = hoverPath;

	surface = ImageManipulator::getImage(groundPath, false);
	hover = ImageManipulator::getImage(hoverPath, false);
	updateRect(false);
}
