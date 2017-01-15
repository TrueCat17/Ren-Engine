#ifndef SCREENIF_H
#define SCREENIF_H

#include "screen_container.h"

#include "utils/string.h"

class ScreenIf: public ScreenContainer {
private:
	String condition;
public:
	ScreenIf(Node* node, ScreenChild *screenParent);
	virtual void updateProps();
};

#endif // SCREENIF_H
