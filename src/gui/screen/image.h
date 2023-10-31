#ifndef IMAGE_H
#define IMAGE_H

#include "container.h"


class Image: public Container {
private:
	std::string prevImagePath;

public:
	Image(Node *node, Screen *screen);
	virtual void updateTexture();
};

#endif // IMAGE_H
