#ifndef SCREENFOR_H
#define SCREENFOR_H

#include "screen_container.h"

#include "utils/string.h"

class ScreenFor: public ScreenContainer {
private:
	String init;
	String onStep;

public:
	ScreenFor(Node *node, ScreenChild *screenParent);
	virtual void updateProps();
};

#endif // SCREENFOR_H
