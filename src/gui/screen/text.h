#ifndef TEXT_H
#define TEXT_H

#include "gui/screen/child.h"

#include "utils/string.h"

class TextField;

class Text: public Child {
private:
	String prevText;

	Uint32 prevColor = 0;

	int prevWidth = 0;
	int prevHeight = 0;

protected:
	TextField *tf;

public:
	int text_size = 20;
	Uint32 color = 0;

	String font;
	String textHAlign;
	String textVAlign;

	Text(Node *node, Screen *screen);
	virtual ~Text();

	virtual void updateRect(bool callFromContainer = false);
	virtual void updateGlobal();
};

#endif // TEXT_H
