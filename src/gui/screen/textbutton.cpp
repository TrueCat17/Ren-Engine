#include "textbutton.h"

#include "media/image_manipulator.h"
#include "media/py_utils.h"
#include "utils/utils.h"


TextButton::TextButton(Node *node, Screen *screen):
    Text(node, screen),
    hoverIsModifiedGround(node->getProp("ground") && !node->getProp("hover")),
    btnRect(this)
{ }

void TextButton::updateTexture() {
	Text::updateTexture();

	auto prevParams = std::tie(prevGround, prevHover, prevParamsIsHover);
	auto curParams = std::tie(ground, hover, curParamsIsHover);
	if (prevParams == curParams && surface) return;

	bool changedGround = prevGround != ground;
	prevParams = curParams;

	if (hoverIsModifiedGround && changedGround) {
		hover = PyUtils::execWithSetTmp("CPP_EMBED: textbutton.cpp", __LINE__,
		                                "im.MatrixColor(tmp, im.matrix.brightness(0.1))", ground, true);
	}

	const std::string &path = curParamsIsHover ? hover : ground;
	surface = ImageManipulator::getImage(path, false);
	if (!surface) {
		std::string prop = curParamsIsHover ? "hover" : "ground";
		Utils::outError("TextButton::updateTexture",
		                "Failed to load % image <%>\n%",
		                prop, path, node->getPlace());
	}
}

void TextButton::checkEvents() {
	btnRect.checkEvents();
	curParamsIsHover = btnRect.isHovered() || selected;
}
