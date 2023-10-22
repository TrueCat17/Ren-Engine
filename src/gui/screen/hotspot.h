#ifndef HOTSPOT_H
#define HOTSPOT_H

#include "child.h"

#include "utils/btn_rect.h"


class Hotspot: public Child {
private:
	float scaleX = 1;
	float scaleY = 1;

public:
	BtnRect btnRect;

	Hotspot(Node *node, Screen *screen);

	virtual void updatePos();
	virtual void updateRect(bool needUpdatePos = true);
	virtual void checkEvents();

	virtual bool transparentForMouse(int x, int y) const;
	virtual void draw() const;
};

#endif // HOTSPOT_H
