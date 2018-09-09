#ifndef SCREENCONTAINER_H
#define SCREENCONTAINER_H

#include "screen_child.h"

class ScreenContainer: public ScreenChild {
private:
	Uint16 indent = 0;

protected:
	Uint16 countInitChildren = 0;

	bool hasVBox = false;
	bool hasHBox = false;

	bool inited = false;
	void addChildrenFromNode();

	bool prevContainersSkipped() const;

public:
	bool skipped = false;
	ScreenContainer *prevContainer = nullptr;

	ScreenContainer(Node* node, ScreenContainer *screenParent, Screen *screen);

	virtual void addChildAt(DisplayObject *child, size_t index);

	virtual void calculateProps();
	virtual void updateSize();

	virtual void draw() const;
};

#endif // SCREENCONTAINER_H
