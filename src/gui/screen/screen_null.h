#ifndef SCREENNULL_H
#define SCREENNULL_H

#include "screen_child.h"

class ScreenNull: public ScreenChild {
public:
	virtual int getMaxX() const;
	virtual int getMaxY() const;

	ScreenNull(Node* node);

	virtual void updateProps();
	virtual void updateSize();
	virtual void updatePos();
	virtual void draw() const;
};

#endif // SCREENNULL_H
