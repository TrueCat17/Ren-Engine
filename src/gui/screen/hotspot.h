#ifndef HOTSPOT_H
#define HOTSPOT_H

#include "child.h"

#include "utils/btn_rect.h"


class Hotspot: public Child {
private:
	String rectStr;
	double scaleX = 1;
	double scaleY = 1;

	bool prevMouseOver = false;

public:
	BtnRect btnRect;

	Hotspot(Node *node, Screen *screen);

	virtual bool checkAlpha(int x, int y) const;

	virtual void draw() const;
};

#endif // HOTSPOT_H
