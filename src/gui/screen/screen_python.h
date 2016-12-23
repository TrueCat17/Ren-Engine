#ifndef SCREENPYTHON_H
#define SCREENPYTHON_H

#include "screen_child.h"

class ScreenPython: public ScreenChild {
public:
	ScreenPython(Node *node);
	virtual void updateProps();
	virtual void update();
	virtual void draw() const;
};

#endif // SCREENPYTHON_H
