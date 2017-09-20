#ifndef SCREENTEXT_H
#define SCREENTEXT_H

#include "gui/screen/screen_child.h"

#include "utils/string.h"

class Text;

class ScreenText: public ScreenChild {
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

	int prevWidth = 0;
	int prevHeight = 0;

protected:
	Text *tf;

public:
	ScreenText(Node *node);
	virtual void calculateProps();
	virtual void updateSize();

	virtual void updateGlobalPos();
};

#endif // SCREENTEXT_H
