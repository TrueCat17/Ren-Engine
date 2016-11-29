#ifndef SCREENTEXTBUTTON_H
#define SCREENTEXTBUTTON_H

#include "gui/screen/screen_text.h"
#include "parser/node.h"
#include "utils/btn_rect.h"


class ScreenTextButton: public ScreenText {
private:
	String background;
	String hoverBackground;

public:
	BtnRect btnRect;

	ScreenTextButton(Node *node);
	virtual void updateProps();
	virtual void update();
};

#endif // SCREENTEXTBUTTON_H
