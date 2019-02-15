#include "text.h"

#include "gv.h"

#include "gui/text_field.h"

#include "media/py_utils.h"
#include "utils/utils.h"

Text::Text(Node *node, Screen *screen):
	Child(node, nullptr, screen),
	tf(new TextField())
{
	tf->wordWrap = true;
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
		bool needUpdate = false;

		if (tf->getMaxWidth() != width) {
			tf->setMaxWidth(width);
			needUpdate = true;
		}
		if (tf->getMaxHeight() != height) {
			tf->setMaxHeight(height);
			needUpdate = true;
		}

		if (tf->getFontName() != font || tf->getFontSize() != text_size) {
			tf->setFont(font, text_size);
			needUpdate = true;
		}
		if (first_param != prevText || color != prevColor) {
			prevText = first_param;
			prevColor = color;
			needUpdate = true;
		}

		if (needUpdate) {
			tf->setText(first_param, color);
		}

		if (!first_param.empty() &&
			(needUpdate || tf->getHAlign() != textHAlign || tf->getVAlign() != textVAlign ||
			 width != prevWidth || height != prevHeight))
		{
			int w = width  <= 0 ? tf->getWidth()  : width;
			int h = height <= 0 ? tf->getHeight() : height;
			tf->setWidth(w);
			tf->setHeight(h);
			setWidth(w);
			setHeight(h);

			tf->setAlign(textHAlign, textVAlign);
		}
	}

	prevWidth = width;
	prevHeight = height;

	updatePos();
}

void Text::updateGlobal() {
	int prevGlobalRotate = getGlobalRotate();
	Child::updateGlobal();

	if (!tf->getText().empty() && prevGlobalRotate != getGlobalRotate()) {
		tf->setAlign(tf->getHAlign(), tf->getVAlign());
	}
}
