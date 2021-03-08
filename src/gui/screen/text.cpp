#include "text.h"

#include "gv.h"

#include "gui/text_field.h"

#include "utils/math.h"
#include "utils/utils.h"

Text::Text(Node *node, Screen *screen):
    Child(node, nullptr, screen),
    tf(new TextField())
{
	tf->wordwrap = true;
	addChildAt(tf, 0);
}
Text::~Text() {
	delete tf;
}


void Text::updateRect(bool) {
	tf->enable = true;

	globalZoomX = (parent ? parent->getGlobalZoomX() : 1) * xzoom;
	globalZoomY = (parent ? parent->getGlobalZoomY() : 1) * yzoom;

	int width  = int(xsize * float(xsizeIsFloat ? GV::width  : 1) * globalZoomX);
	int height = int(ysize * float(ysizeIsFloat ? GV::height : 1) * globalZoomY);

	if (first_param.empty() && prevText.empty()) {
		setWidth(width);
		setHeight(height);
	}else {
		if (tf->maxWidth != width) {
			tf->maxWidth = width;
			needUpdate = true;
		}
		if (tf->maxHeight != height) {
			tf->maxHeight = height;
			needUpdate = true;
		}

		if (tf->mainStyle.fontName != font || !Math::floatsAreEq(tf->mainStyle.fontSize, text_size * globalZoomY)) {
			tf->setFont(font, text_size * globalZoomY);
			needUpdate = true;
		}
		if (first_param != prevText) {
			prevText = first_param;
			needUpdate = true;
		}

		if (needUpdate) {
			tf->setText(first_param);
		}

		if (!first_param.empty() &&
			(needUpdate || tf->getHAlign() != textHAlign || tf->getVAlign() != textVAlign ||
			 width != prevWidth || height != prevHeight))
		{
			int w = width  <= 0 ? tf->getWidth()  : width;
			int h = height <= 0 ? tf->getHeight() : height;
			setWidth(w);
			setHeight(h);

			tf->setWidth(w);
			tf->setHeight(h);
			tf->setAlign(textHAlign, textVAlign);
		}
	}

	needUpdate = false;
	prevWidth = width;
	prevHeight = height;

	updatePos();
}

void Text::updateGlobal() {
	float prevGlobalRotate = getGlobalRotate();
	Child::updateGlobal();

	if (!prevText.empty() && !Math::floatsAreEq(prevGlobalRotate, getGlobalRotate())) {
		tf->setAlign(tf->getHAlign(), tf->getVAlign());
	}
}
