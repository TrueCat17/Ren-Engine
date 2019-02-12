#ifndef IMAGE_MANIPULATOR_H
#define IMAGE_MANIPULATOR_H

#include <SDL2/SDL_endian.h>

#include "utils/image_typedefs.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
static const Uint8 Rshift = 24;
static const Uint8 Gshift = 16;
static const Uint8 Bshift = 8;
static const Uint8 Ashift = 0;
#else
static const Uint8 Rshift = 0;
static const Uint8 Gshift = 8;
static const Uint8 Bshift = 16;
static const Uint8 Ashift = 24;
#endif


class ImageManipulator {
public:
	static void init();

	static void save(const std::string &imageStr, const std::string &path, const std::string &width, const std::string &height);
	static void saveSurface(const SurfacePtr &imageStr, const std::string &path, const std::string &width, const std::string &height, bool now = false);

	static SurfacePtr getNewNotClear(int w, int h);

	static void loadImage(const std::string &desc);
	static SurfacePtr getImage(std::string desc);
};

#endif // IMAGE_MANIPULATOR_H
