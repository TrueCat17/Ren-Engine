#ifndef CONTAINER_H
#define CONTAINER_H

#include "child.h"

class Container: public Child {
public:
	bool hasVBox = false;
	bool hasHBox = false;

	bool spacingIsFloat;
	float spacing = 0;

	std::vector<Child*> screenChildren;


	Container(Node* node, Container *screenParent, Screen *screen);
	void addChildrenFromNode();

	virtual void addChildAt(DisplayObject *child, size_t index);

	virtual void updateZoom();
	virtual void updatePos();
	virtual void updateRect(bool needUpdatePos = true);
};

#endif // CONTAINER_H
