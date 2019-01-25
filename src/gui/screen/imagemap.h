#ifndef IMAGEMAP_H
#define IMAGEMAP_H

#include "container.h"


class Imagemap: public Container {
private:
	String prevGroundPath;
	String prevHoverPath;

public:
	String groundPath;
	String hoverPath;

	SurfacePtr hover = nullptr;

	Imagemap(Node *node, Screen *screen);

	virtual void updateTexture(bool skipError = false);
};

#endif // IMAGEMAP_H
