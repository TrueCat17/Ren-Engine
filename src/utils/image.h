#ifndef IMAGE_H
#define IMAGE_H

#include <SDL2/SDL.h>

#include "utils/string.h"

class Image {
private:
	static String clear(String s);
public:
	static SDL_Surface* getImage(String desc);
};

#endif // IMAGE_H
