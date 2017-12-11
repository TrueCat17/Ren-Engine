#ifndef SCREENHOTSPOT_H
#define SCREENHOTSPOT_H

#include "screen_child.h"

#include "utils/btn_rect.h"


class ScreenHotspot: public ScreenChild {
private:
	String rectStr;
	double scaleX = 1;
	double scaleY = 1;

	bool prevMouseOver = false;

public:
	BtnRect btnRect;

	ScreenHotspot(Node *node, Screen *screen);

	virtual bool checkAlpha(int x, int y) const;

	virtual void calculateProps();
	virtual void updateSize() {}
	virtual void updatePos() {}
	virtual void draw() const;
};

#endif // SCREENHOTSPOT_H
