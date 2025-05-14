#ifndef CONTAINER_BOX_H
#define CONTAINER_BOX_H

#include "container.h"

class ContainerBox: public Container {
private:
	bool hasVBox:1;
	bool hasHBox:1;

public:
	bool spacing_is_float:1;
	bool spacing_min_is_float:1;
	bool spacing_max_is_float:1;

	float spacing = 0;
	float spacing_min = 0;
	float spacing_max = 0;


	ContainerBox(Node *node, Container *screenParent, Screen *screen);

	virtual void updateWidth();
	virtual void updateHeight();

	virtual bool getHasVBox() const { return hasVBox; }
	virtual bool getHasHBox() const { return hasHBox; }
};

#endif // CONTAINER_BOX_H
