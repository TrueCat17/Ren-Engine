#ifndef SCREENWHILE_H
#define SCREENWHILE_H

#include "screen_container.h"

#include "utils/string.h"

class ScreenWhile: public ScreenContainer {
private:
	String condition;

public:
	ScreenWhile(Node *node, ScreenContainer *screenParent, Screen *screen);
	virtual void calculateProps();
};

#endif // SCREENWHILE_H
