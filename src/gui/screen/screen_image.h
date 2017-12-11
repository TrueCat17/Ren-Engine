#ifndef SCREENIMAGE_H
#define SCREENIMAGE_H

#include "screen_container.h"


class ScreenImage: public ScreenContainer {
public:
	ScreenImage(Node *node, Screen *screen);

	virtual void calculateProps();
	virtual void updateTexture();
	virtual void updateSize();
};

#endif // SCREENIMAGE_H
