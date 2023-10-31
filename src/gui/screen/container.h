#ifndef CONTAINER_H
#define CONTAINER_H

#include "child.h"

class Container: public Child {
public:
	bool hasVBox:1;
	bool hasHBox:1;

	bool spacing_is_float:1;
	bool spacing_min_is_float:1;
	bool spacing_max_is_float:1;

	float spacing = 0;
	float spacing_min = 0;
	float spacing_max = 0;

	std::vector<Child*> screenChildren;


	Container(Node* node, Container *screenParent, Screen *screen);
	void addChildrenFromNode();

	virtual void addChildAt(DisplayObject *child, size_t index);

	virtual void updateZoom();
	virtual void updatePos();
	virtual void updateRect(bool needUpdatePos = true);
	virtual void updateTexture();
};

#endif // CONTAINER_H
