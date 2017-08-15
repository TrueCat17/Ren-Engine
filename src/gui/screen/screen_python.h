#ifndef SCREENPYTHON_H
#define SCREENPYTHON_H

#include "screen_child.h"

class ScreenPython: public ScreenChild {
private:
	bool isBlock;

public:
	ScreenPython(Node *node, bool isBlock);
	virtual void calculateProps();

	virtual void updateSize() {}
	virtual void updatePos() {}
	virtual void draw() const {}
};

#endif // SCREENPYTHON_H
