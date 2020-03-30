#ifndef TEXT_H
#define TEXT_H

#include "gui/screen/child.h"

class TextField;

class Text: public Child {
private:
	std::string prevText;

	int prevWidth = 0;
	int prevHeight = 0;

public:
	TextField *tf;

	float text_size = 20;
	Uint32 color = 0;

	std::string font;
	std::string textHAlign;
	std::string textVAlign;

	Text(Node *node, Screen *screen);
	virtual ~Text();

	virtual void updateRect(bool needUpdatePos = true);
	virtual void updateGlobal();
};

#endif // TEXT_H
