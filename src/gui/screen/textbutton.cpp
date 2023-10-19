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
	if (surface && prevGround == ground && prevHover == hover && prevMouseOver == btnRect.mouseOvered) return;

	if (hoverIsModifiedGround && prevGround != ground) {
		hover = PyUtils::exec("CPP_EMBED: textbutton.cpp", __LINE__,
		                      "im.MatrixColor(r'" + ground + "', im.matrix.brightness(0.1))", true);
	}
	prevGround = ground;
	prevHover = hover;

	const std::string &path = !btnRect.mouseOvered ? ground : hover;
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

	curParamsIsHover = btnRect.mouseOvered || ground == hover;

	if (btnRect.mouseOvered) {
		if (!prevMouseOver) {
			btnRect.onHovered();
		}
	}else {
		if (prevMouseOver) {
			btnRect.onUnhovered();
		}
	}
	prevMouseOver = btnRect.mouseOvered;

	if (btnRect.mouseLeftDown) {
		btnRect.onLeftClick();
	}
	if (btnRect.mouseRightDown) {
		btnRect.onRightClick();
	}
}
