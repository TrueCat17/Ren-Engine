#ifndef SCREENELSE_H
#define SCREENELSE_H

#include "screen_container.h"

class ScreenElse: public ScreenContainer {
public:
	ScreenElse(Node *node, ScreenChild *screenParent, ScreenContainer *prevContainer);
	virtual void calculateProps();
};

#endif // SCREENELSE_H
