#include "screen_imagemap.h"

#include "utils/utils.h"

ScreenImagemap::ScreenImagemap(Node *node): ScreenContainer(node, this) {

}
void ScreenImagemap::updateProps() {
	String groundPath = node->getProp("ground");
	String hoverPath = node->getProp("hover");

	if (!hoverPath && groundPath) {
		hoverPath = Utils::execPython("im.MatrixColor('" + groundPath + "', im.matrix.contrast(1.5))", true);
	}

	texture = Utils::getTexture(groundPath);
	hover = Utils::getTexture(hoverPath);

	ScreenContainer::updateProps();
}
