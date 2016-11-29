#ifndef SCREENELIF_H
#define SCREENELIF_H

#include "screen_container.h"

class ScreenElif: public ScreenContainer {
private:
	String condition;

public:
	virtual bool enabled() const;

	ScreenElif(Node *node, ScreenChild *screenParent, ScreenContainer *prevContainer);
	virtual void updateProps();
};

#endif // SCREENELIF_H
