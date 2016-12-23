#ifndef SCREENELSE_H
#define SCREENELSE_H

#include "screen_container.h"

class ScreenElse: public ScreenContainer {
public:
	virtual bool enabled() const;

	ScreenElse(Node *node, ScreenChild *screenParent, ScreenContainer *prevContainer);
	virtual void updateProps();
};

#endif // SCREENELSE_H
