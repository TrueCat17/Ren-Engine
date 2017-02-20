#ifndef SCREENIMAGEMAP_H
#define SCREENIMAGEMAP_H

#include "screen_container.h"
#include "utils/string.h"

class ScreenImagemap: public ScreenContainer {
private:
	String groundPath;
	String hoverPath;

public:
	SDL_Texture *hover = nullptr;

	ScreenImagemap(Node *node);

	virtual void calculateProps();
};

#endif // SCREENIMAGEMAP_H
