#include "imagemap.h"

#include "style.h"

#include "media/image_manipulator.h"
#include "media/py_utils.h"

#include "utils/utils.h"

Imagemap::Imagemap(Node *node, Screen *screen):
	Container(node, this, screen)
{
	for (Node *child : node->children) {
		if (child->command == "ground") {
			groundIsStd = false;
		}else
		if (child->command == "hover") {
			hoverIsStd = false;
		}
	}
}

void Imagemap::updateRect(bool) {
	Container::updateRect(false);

	xsize = std::max<float>(xsize, 0);
	ysize = std::max<float>(ysize, 0);
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


		if (prevGroundPath != groundPath && (hoverIsStd || hoverPath.empty())) {
			std::string toConvert;

			if (!groundIsStd) {
				toConvert = groundPath;
			}else {
				const Node *style = node->getProp("style");
				const std::string &styleName = style ? style->params : node->command;

				PyObject *hoverObj = Style::getProp(styleName, "hover");
				if (PyString_CheckExact(hoverObj)) {
					hoverPath = PyString_AS_STRING(hoverObj);
					if (hoverPath.empty()) {
						PyObject *groundObj = Style::getProp(styleName, "ground");
						if (PyString_CheckExact(groundObj)) {
							toConvert = PyString_AS_STRING(groundObj);
						}else {
							std::string type = groundObj->ob_type->tp_name;
							Utils::outMsg("Imagemap::hover",
							              "In style." + styleName + ".ground expected type str, got " + type);
						}
						toConvert = groundPath;
					}
				}else {
					std::string type = hoverObj->ob_type->tp_name;
					Utils::outMsg("Imagemap::hover",
					              "In style." + styleName + ".hover expected type str, got " + type);
				}
			}

			if (!toConvert.empty()) {
				hoverPath = PyUtils::exec("CPP_EMBED: imagemap.cpp", __LINE__, "im.MatrixColor(r'" + toConvert + "', im.matrix.brightness(0.1))", true);
			}
		}


		prevGroundPath = groundPath;
		prevHoverPath = hoverPath;

		surface = ImageManipulator::getImage(groundPath, false);
		hover = ImageManipulator::getImage(hoverPath, false);
		updateRect(false);
	}
}
