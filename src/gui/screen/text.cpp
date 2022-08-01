#include "text.h"

#include "gui/text_field.h"

#include "utils/math.h"
#include "utils/stage.h"

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

	int width  = int(xsize * float(xsizeIsFloat ? Stage::width  : 1) * globalZoomX);
	int height = int(ysize * float(ysizeIsFloat ? Stage::height : 1) * globalZoomY);

	if (first_param.empty() && prevText.empty()) {
		setWidth(float(std::max(width, 0)));
		setHeight(float(std::max(height, 0)));
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

		if (needUpdate ||
		    !Math::floatsAreEq(tf->getHAlign(), textHAlign) ||
		    !Math::floatsAreEq(tf->getVAlign(), textVAlign) ||
		    width != prevWidth || height != prevHeight
		) {
			float w = width  <= 0 ? tf->getWidth()  : float(width);
			float h = height <= 0 ? tf->getHeight() : float(height);
			setWidth(w);
			setHeight(h);

			tf->setWidth(w);
			tf->setHeight(h);
			tf->setAlign(textHAlign, textVAlign);

			prevWidth = width;
			prevHeight = height;

			needUpdate = false;
		}
	}

	updatePos();
}

void Text::updateGlobal() {
	float prevGlobalRotate = getGlobalRotate();
	Child::updateGlobal();

	if (!prevText.empty() && !Math::floatsAreEq(prevGlobalRotate, getGlobalRotate())) {
		tf->setAlign(tf->getHAlign(), tf->getVAlign());
	}
}
