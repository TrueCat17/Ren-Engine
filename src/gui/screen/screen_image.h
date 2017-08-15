#ifndef SCREENIMAGE_H
#define SCREENIMAGE_H

#include "screen_container.h"

#include "utils/string.h"

class ScreenImage: public ScreenContainer {
private:
	String imageCode;
	String imagePath;

public:
	ScreenImage(Node *node);

	virtual void calculateProps();
	virtual void updateTexture();
	virtual void updateSize();
};

#endif // SCREENIMAGE_H
