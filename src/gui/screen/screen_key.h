#ifndef SCREENKEY_H
#define SCREENKEY_H

#include "screen_child.h"

class ScreenKey: public ScreenChild {
private:
	int lastDown = 0;
	int keyDelay = 15;

	bool prevIsDown = false;
	int firstKeyDelay = 500;

	String keyStr;

public:
	ScreenKey(Node *node);
	virtual void updateProps();
	virtual void update();
};

#endif // SCREENKEY_H
