#include "screen_imagemap.h"

#include "media/py_utils.h"
#include "media/image.h"
#include "parser/node.h"

ScreenImagemap::ScreenImagemap(Node *node):
	ScreenContainer(node, this)
{
	setProp(ScreenProp::GROUND, node->getPropCode("ground"));
	setProp(ScreenProp::HOVER, node->getPropCode("hover"));

	preparationToUpdateCalcProps();
}

void ScreenImagemap::calculateProps() {
	ScreenContainer::calculateProps();
}
void ScreenImagemap::updateTexture() {
	if (propWasChanged[ScreenProp::GROUND] || propWasChanged[ScreenProp::HOVER]) {
		propWasChanged[ScreenProp::GROUND] = false;
		propWasChanged[ScreenProp::HOVER] = false;

		const String newGroundPath = PyUtils::getStr(propValues[ScreenProp::GROUND]);
		const String newHoverPath = PyUtils::getStr(propValues[ScreenProp::HOVER]);

		if (newHoverPath) {
			hoverPath = newHoverPath;
		}else
			if (groundPath != newGroundPath) {
				hoverPath = PyUtils::exec("CPP_EMBED: screen_imagemap.cpp", __LINE__, "im.MatrixColor(r'" + groundPath + "', im.matrix.contrast(1.5))", true);
			}
		groundPath = newGroundPath;

		surface = Image::getImage(groundPath);
		hover = Image::getImage(hoverPath);
	}
}
