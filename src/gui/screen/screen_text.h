#ifndef SCREENTEXT_H
#define SCREENTEXT_H

#include "gui/screen/screen_container.h"

#include "utils/string.h"

class Text;

class ScreenText: public ScreenContainer {
private:
	String prevText;
	String text;
	String textExec;

	String font;
	String textHAlign;
	String textVAlign;
	int size = 20;

	int color = 0;
	int prevColor = 0;

protected:
	Text *tf;

public:
	ScreenText(Node *node);
	virtual void updateProps();
	virtual void updateSize();
};

#endif // SCREENTEXT_H
