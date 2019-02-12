#ifndef TEXT_H
#define TEXT_H

#include "gui/screen/child.h"

class TextField;

class Text: public Child {
private:
	std::string prevText;

	Uint32 prevColor = 0;

	int prevWidth = 0;
	int prevHeight = 0;

protected:
	TextField *tf;

public:
	int text_size = 20;
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
