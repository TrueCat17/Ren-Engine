#ifndef SCREENELIF_H
#define SCREENELIF_H

#include "screen_container.h"

#include "utils/string.h"

class ScreenElif: public ScreenContainer {
private:
	String condition;

public:
	ScreenElif(Node *node, ScreenChild *screenParent, ScreenContainer *prevContainer);
	virtual void calculateProps();
};

#endif // SCREENELIF_H
