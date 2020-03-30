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

	int width = int(xsize * (xsizeIsDouble ? GV::width : 1));
	int height = int(ysize * (ysizeIsDouble ? GV::height : 1));

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

		if (tf->mainStyle.fontName != font || !Math::floatsAreEq(tf->mainStyle.fontSize, text_size)) {
			tf->setFont(font, text_size);
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

			tf->setAlign(textHAlign, textVAlign);
		}
	}

	needUpdate = false;
	prevWidth = width;
	prevHeight = height;

	updatePos();
}

void Text::updateGlobal() {
	int prevGlobalRotate = getGlobalRotate();
	Child::updateGlobal();

	if (!prevText.empty() && prevGlobalRotate != getGlobalRotate()) {
		tf->setAlign(tf->getHAlign(), tf->getVAlign());
	}
}
