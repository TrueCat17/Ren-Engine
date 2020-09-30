#include "imagemap.h"

#include "style.h"

#include "media/image_manipulator.h"
#include "media/py_utils.h"

#include "utils/utils.h"

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
			const Node *style = node->getProp("style");
			const std::string &styleName = style ? style->params : node->command;

			PyObject *hoverObj = Style::getProp(styleName, "hover");
			if (PyString_CheckExact(hoverObj)) {
				hoverPath = PyString_AS_STRING(hoverObj);
				if (hoverPath.empty()) {
					hoverPath = PyUtils::exec("CPP_EMBED: imagemap.cpp", __LINE__, "im.MatrixColor(r'" + groundPath + "', im.matrix.contrast(1.5))", true);
				}
			}else {
				std::string type = hoverObj->ob_type->tp_name;
				Utils::outMsg("Imagemap::hover",
				              "In style." + styleName + ".hover expected type str, got " + type);
			}
		}
		prevGroundPath = groundPath;
		prevHoverPath = hoverPath;

		surface = ImageManipulator::getImage(groundPath, false);
		hover = ImageManipulator::getImage(hoverPath, false);
		updateRect(false);
	}
}
