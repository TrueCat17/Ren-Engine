#ifndef IMAGE_H
#define IMAGE_H

#include "container.h"


class Image: public Container {
private:
	String prevImagePath;

public:
	Image(Node *node, Screen *screen);

	virtual void updateSize();
	virtual void updateTexture(bool skipError = false);
};

#endif // IMAGE_H
