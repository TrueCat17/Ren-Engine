#ifndef CONTAINER_H
#define CONTAINER_H

#include "child.h"

class Container: public Child {
private:
	void addChildrenFromNode();

public:
	bool hasVBox = false;
	bool hasHBox = false;

	int spacing = 0;
	Container *prevContainer = nullptr;

	Container(Node* node, Container *screenParent, Screen *screen);

	virtual void addChildAt(DisplayObject *child, size_t index);

	virtual void updateProps();
	virtual void updateSize();
};

#endif // CONTAINER_H
