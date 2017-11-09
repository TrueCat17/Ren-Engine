#ifndef SCREENIMAGEMAP_H
#define SCREENIMAGEMAP_H

#include "screen_container.h"


class ScreenImagemap: public ScreenContainer {
private:
	String groundPath;
	String hoverPath;

public:
	SurfacePtr hover = nullptr;

	ScreenImagemap(Node *node);

	virtual void calculateProps();
	virtual void updateTexture();
};

#endif // SCREENIMAGEMAP_H
