#ifndef SCREENCONTAINER_H
#define SCREENCONTAINER_H

#include "screen_child.h"

class ScreenContainer: public ScreenChild {
private:
	size_t indent = 0;

protected:
	size_t countInitChildren = 0;

	bool hasVBox = false;
	bool hasHBox = false;

	bool inited = false;
	void addChildrenFromNode();

	bool prevContainersSkipped() const;

public:
	bool skipped = false;
	ScreenContainer *prevContainer = nullptr;

	virtual int getMinX() const;
	virtual int getMinY() const;
	virtual int getMaxX() const;
	virtual int getMaxY() const;

	ScreenContainer(Node* node, ScreenChild *screenParent);

	virtual void addChildAt(DisplayObject *child, size_t index);

	virtual void updateProps();
	virtual void draw() const;
};

#endif // SCREENCONTAINER_H
