#ifndef SCREENWHILE_H
#define SCREENWHILE_H

#include "screen_container.h"

#include "utils/string.h"

class ScreenWhile: public ScreenContainer {
private:
	String condition;

public:
	ScreenWhile(Node *node, ScreenChild *screenParent);
	virtual void updateProps();
};

#endif // SCREENWHILE_H
