#include "imagemap.h"

#include "media/py_utils.h"
#include "media/image_manipulator.h"
#include "parser/node.h"

Imagemap::Imagemap(Node *node, Screen *screen):
	Container(node, this, screen)
{}

void Imagemap::updateSize() {
	Container::updateSize();

	if (xsize <= 0) {
		xsizeIsTextureWidth = true;
		xsize = surface ? surface->w : 0;
	}else {
		xsizeIsTextureWidth = false;
	}

	if (ysize <= 0) {
		ysizeIsTextureHeight = true;
		ysize = surface ? surface->h : 0;
	}else {
		ysizeIsTextureHeight = false;
	}
}

void Imagemap::updateTexture(bool skipError) {
	if (skipError && !groundPath) return;

	if (!surface || !hover || prevGroundPath != groundPath || prevHoverPath != hoverPath) {
		if (prevGroundPath != groundPath && !hoverPath) {
			hoverPath = PyUtils::exec("CPP_EMBED: screen_imagemap.cpp", __LINE__, "im.MatrixColor(r'" + groundPath + "', im.matrix.contrast(1.5))", true);
		}
		prevGroundPath = groundPath;
		prevHoverPath = hoverPath;

		surface = ImageManipulator::getImage(groundPath);
		hover = ImageManipulator::getImage(hoverPath);

		if (xsizeIsTextureWidth)  xsize = surface ? surface->w : 0;
		if (ysizeIsTextureHeight) ysize = surface ? surface->h : 0;
		updateSize();
	}
}
