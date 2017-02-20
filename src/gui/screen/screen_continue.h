#ifndef SCREENCONTINUE_H
#define SCREENCONTINUE_H

#include "screen_child.h"

class ScreenContinue: public ScreenChild {
public:
	ScreenContinue(Node *node);
	virtual void calculateProps();
};

#endif // SCREENCONTINUE_H
