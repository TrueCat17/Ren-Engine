#include "textbutton.h"

#include "media/image_manipulator.h"
#include "media/py_utils.h"
#include "utils/utils.h"


TextButton::TextButton(Node* node, Screen *screen):
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
		hover = PyUtils::exec("CPP_EMBED: textbutton.cpp", __LINE__,
		                      "im.MatrixColor(r'" + ground + "', im.matrix.brightness(0.1))", true);
	}

	const std::string &path = curParamsIsHover ? hover : ground;
	surface = ImageManipulator::getImage(path, false);
	if (!surface) {
		std::string desc = curParamsIsHover ? "hover" : "ground";
		Utils::outMsg("TextButton::updateTexture",
		              "Failed to load " + desc + " image <" + path + ">\n" +
		              node->getPlace());
	}
}

void TextButton::checkEvents() {
	btnRect.checkEvents();
	curParamsIsHover = btnRect.mouseOvered || selected;
}
