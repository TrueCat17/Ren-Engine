#ifndef IMAGEMAP_H
#define IMAGEMAP_H

#include "container.h"


class Imagemap: public Container {
private:
	std::string prevGroundPath;
	std::string prevHoverPath;

public:
	std::string groundPath;
	std::string hoverPath;

	SurfacePtr hover = nullptr;

	Imagemap(Node *node, Screen *screen);

	virtual void updateTexture();
};

#endif // IMAGEMAP_H
