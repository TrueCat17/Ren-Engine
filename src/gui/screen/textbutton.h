#ifndef TEXTBUTTON_H
#define TEXTBUTTON_H

#include "gui/screen/text.h"
#include "parser/node.h"
#include "utils/btn_rect.h"


class TextButton: public Text {
private:
	String prevGround;
	String prevHover;

	bool prevMouseOver = false;

public:
	String ground;
	String hover;

	BtnRect btnRect;

	TextButton(Node *node, Screen *screen);

	virtual void updateRect(bool callFromContainer = false);
	virtual void updateTexture(bool skipError = false);
	virtual void checkEvents();
};

#endif // TEXTBUTTON_H
