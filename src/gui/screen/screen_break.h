#ifndef SCREENBREAK_H
#define SCREENBREAK_H

#include "screen_child.h"

class ScreenBreak: public ScreenChild {
public:
	ScreenBreak(Node *node);
	virtual void updateProps();
};

#endif // SCREENBREAK_H
