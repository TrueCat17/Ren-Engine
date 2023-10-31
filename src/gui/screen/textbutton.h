#ifndef TEXTBUTTON_H
#define TEXTBUTTON_H

#include "text.h"
#include "utils/btn_rect.h"


class TextButton: public Text {
private:
	std::string prevGround;
	std::string prevHover;

	const bool hoverIsModifiedGround;

public:
	std::string ground;
	std::string hover;

	BtnRect btnRect;

	TextButton(Node *node, Screen *screen);

	virtual void updateTexture();
	virtual void checkEvents();
};

#endif // TEXTBUTTON_H
