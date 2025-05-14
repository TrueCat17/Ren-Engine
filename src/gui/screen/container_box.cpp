#include "container_box.h"

#include "screen.h"

#include "utils/stage.h"
#include "utils/utils.h"


ContainerBox::ContainerBox(Node *node, Container *screenParent, Screen *screen):
    Container(node, screenParent, screen),
    hasVBox(false),
    hasHBox(false)
{
	if (node->command == "vbox") {
		hasVBox = true;
	}else
	if (node->command == "hbox") {
		hasHBox = true;
	}else {
		if (node->command == "use") {
			node = Screen::getDeclared(node->params);
		}

		const Node *hasNode = node->getProp("has");
		if (!hasNode) return;

		if (hasNode->params == "vbox") {
			hasVBox = true;
		}else
		if (hasNode->params == "hbox") {
			hasHBox = true;
		}else
		{//unreachable
			Utils::outMsg("ContainerBox::ContainerBox",
			              "<has> expected value <vbox> or <hbox>, got <" + hasNode->params + ">\n" +
			              hasNode->getPlace());
		}
	}
}

void ContainerBox::updateWidth() {
	if (!hasHBox) {
		Container::updateWidth();
		return;
	}

	float size = spacing * float(spacing_is_float ? Stage::width : 1);
	float min = spacing_min * float(spacing_min_is_float ? Stage::width : 1);
	float max = spacing_max * float(spacing_max_is_float ? Stage::width : 1);
	if (min > 0 && size < min) size = min;
	if (max > 0 && size > max) size = max;
	float globalSpacing = size * globalZoomX;

	float width = 0;
	for (DisplayObject *child : children) {
		if (!child->enable) continue;

		child->setX(width);

		float w = child->getWidth();
		if (w > 0) {
			width += w + globalSpacing;
		}
	}
	if (width > 0) {
		width -= globalSpacing;
	}
	setWidthWithMinMax(width / globalZoomX);
}
void ContainerBox::updateHeight() {
	if (!hasVBox) {
		Container::updateHeight();
		return;
	}

	float size = spacing * float(spacing_is_float ? Stage::height : 1);
	float min = spacing_min * float(spacing_min_is_float ? Stage::height : 1);
	float max = spacing_max * float(spacing_max_is_float ? Stage::height : 1);
	if (min > 0 && size < min) size = min;
	if (max > 0 && size > max) size = max;
	float globalSpacing = size * globalZoomY;

	float height = 0;
	for (DisplayObject *child : children) {
		if (!child->enable) continue;

		child->setY(height);

		float h = child->getHeight();
		if (h > 0) {
			height += h + globalSpacing;
		}
	}
	if (height > 0) {
		height -= globalSpacing;
	}
	setHeightWithMinMax(height / globalZoomY);
}
