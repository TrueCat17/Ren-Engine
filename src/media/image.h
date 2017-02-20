#ifndef IMAGE_H
#define IMAGE_H

#include <SDL2/SDL.h>

#include "utils/string.h"

class Image {
public:
	static SDL_Surface* getImage(String desc);
};

#endif // IMAGE_H
