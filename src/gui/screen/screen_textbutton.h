#ifndef SCREENTEXTBUTTON_H
#define SCREENTEXTBUTTON_H

#include "gui/screen/screen_text.h"
#include "parser/node.h"
#include "utils/btn_rect.h"


class ScreenTextButton: public ScreenText {
private:
	String ground;
	String hover;

public:
	BtnRect btnRect;

	ScreenTextButton(Node *node);
	virtual void calculateProps();
	virtual void updateSize();
};

#endif // SCREENTEXTBUTTON_H
