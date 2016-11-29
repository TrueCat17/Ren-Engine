#ifndef SCREENIF_H
#define SCREENIF_H

#include "screen_container.h"

class ScreenIf: public ScreenContainer {
private:
	String condition;
public:
	virtual bool enabled() const;

	ScreenIf(Node* node, ScreenChild *screenParent);
	virtual void updateProps();
};

#endif // SCREENIF_H
