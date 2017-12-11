#ifndef SCREENFOR_H
#define SCREENFOR_H

#include "screen_container.h"

#include "utils/string.h"

class ScreenFor: public ScreenContainer {
private:
	bool used = false;

	String iterName;
	String init;
	String onStep;

	String propName;

public:
	ScreenFor(Node *node, ScreenChild *screenParent, Screen *screen);
	virtual ~ScreenFor();
	virtual void calculateProps();
};

#endif // SCREENFOR_H
