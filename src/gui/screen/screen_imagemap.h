#ifndef SCREENIMAGEMAP_H
#define SCREENIMAGEMAP_H

#include "screen_container.h"

class ScreenImagemap: public ScreenContainer {
public:
	SDL_Texture *hover = nullptr;

	ScreenImagemap(Node *node);

	virtual void updateProps();
};

#endif // SCREENIMAGEMAP_H
