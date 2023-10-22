#include "textbutton.h"

#include "media/image_manipulator.h"
#include "media/py_utils.h"

#include "utils/file_system.h"


TextButton::TextButton(Node* node, Screen *screen):
    Text(node, screen),
    hoverIsModifiedGround(node->getProp("ground") && !node->getProp("hover")),
    btnRect(this)
{ }

void TextButton::updateRect(bool) {
	Child::updateRect(false);
	Text::updateRect();

	if (xsize < 0) { xsize = 0; }
	if (ysize < 0) { ysize = 0; }
}

void TextButton::updateTexture(bool skipError) {
	if (skipError && ground.empty()) return;

	auto prevParams = std::tie(prevGround, prevHover, prevParamsIsHover);
	auto curParams = std::tie(ground, hover, curParamsIsHover);
	if (surface && prevParams == curParams) return;

	if (hoverIsModifiedGround && prevGround != ground) {
		hover = PyUtils::exec("CPP_EMBED: textbutton.cpp", __LINE__,
		                      "im.MatrixColor(r'" + ground + "', im.matrix.brightness(0.1))", true);
	}
	prevGround = ground;
	prevHover = hover;
	prevParamsIsHover = curParamsIsHover;

	const std::string &path = curParamsIsHover ? hover : ground;
	if (skipError && !FileSystem::exists(path)) return;

	surface = ImageManipulator::getImage(path, false);
	updateRect();
}

void TextButton::checkEvents() {
	if (alpha <= 0 || !isModal() || globalSkipMouse) {
		btnRect.mouseOvered = false;
		btnRect.mouseLeftDown = false;
		btnRect.mouseRightDown = false;
	}

	curParamsIsHover = btnRect.mouseOvered || selected;

	btnRect.checkEvents();
}
