#ifndef CONTAINER_H
#define CONTAINER_H

#include "child.h"

class Container: public Child {
public:
	std::vector<Child*> screenChildren;


	Container(Node *node, Container *screenParent, Screen *screen);

	void addChildrenFromNode();

	virtual void addChildAt(DisplayObject *child, uint32_t index);

	virtual void updateProps();
	virtual void updateZoom();
	virtual void updatePos();
	virtual void updateSize();
	virtual void updateTexture();

	virtual void updateWidth();
	virtual void updateHeight();

	virtual bool getHasVBox() const { return false; }
	virtual bool getHasHBox() const { return false; }
};

#endif // CONTAINER_H
