#ifndef SCREENIF_H
#define SCREENIF_H

#include "screen_container.h"
#include "utils/string.h"

class ScreenIf: public ScreenContainer {
private:
	String condition;
public:
	ScreenIf(Node* node, ScreenChild *screenParent, Screen *screen);
	virtual void calculateProps();
};

#endif // SCREENIF_H
