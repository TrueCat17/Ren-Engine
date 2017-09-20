#include "screen_imagemap.h"

#include "media/py_utils.h"
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

		const String &newGroundPath = propValues.at(ScreenProp::GROUND);
		const String &newHoverPath = propValues.at(ScreenProp::HOVER);

		if (newHoverPath) {
			hoverPath = newHoverPath;
		}else
			if (groundPath != newGroundPath) {
				hoverPath = PyUtils::exec("CPP_EMBED: screen_imagemap.cpp", __LINE__, "im.MatrixColor('" + groundPath + "', im.matrix.contrast(1.5))", true);
			}
		groundPath = newGroundPath;

		texture = Utils::getTexture(groundPath);
		hover = Utils::getTexture(hoverPath);
	}
}
