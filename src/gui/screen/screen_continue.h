#ifndef SCREENCONTINUE_H
#define SCREENCONTINUE_H

#include "screen_child.h"

class ScreenContinue: public ScreenChild {
public:
	ScreenContinue(Node *node);
	virtual void updateProps();
};

#endif // SCREENCONTINUE_H
