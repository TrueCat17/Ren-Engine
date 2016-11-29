#ifndef SCREENTEXT_H
#define SCREENTEXT_H

#include "gui/text.h"
#include "gui/screen/screen_container.h"
#include "parser/node.h"

class ScreenText: public ScreenContainer {
private:
	String prevText;
	String text;
	String textExec;

	String font;
	int size = 20;

	int color = 0;
	int prevColor = 0;

protected:
	Text *tf;

	virtual double getDefaultWidth() const;
	virtual double getDefaultHeight() const;

public:
	ScreenText(Node *node);
	virtual void updateProps();
	virtual void update();
};

#endif // SCREENTEXT_H
