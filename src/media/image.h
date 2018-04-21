#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <map>
#include <deque>
#include <functional>
#include <mutex>

#include <SDL2/SDL_endian.h>

#include "utils/image_typedefs.h"
#include "utils/string.h"

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


class Image {
private:
	static std::map<String, std::function<SurfacePtr(const std::vector<String>&)>> functions;

	static std::deque<String> toLoadImages;
	static std::mutex toLoadMutex;
	static void preloadThread();

	static SurfacePtr scale(const std::vector<String> &args);
	static SurfacePtr factorScale(const std::vector<String> &args);
	static SurfacePtr rendererScale(const std::vector<String> &args);
	static SurfacePtr crop(const std::vector<String> &args);
	static SurfacePtr composite(const std::vector<String> &args);
	static SurfacePtr flip(const std::vector<String> &args);
	static SurfacePtr matrixColor(const std::vector<String> &args);
	static SurfacePtr reColor(const std::vector<String> &args);
	static SurfacePtr rotozoom(const std::vector<String> &args);
	static SurfacePtr mask(const std::vector<String> &args);

public:
	static void init();
	static void save(const std::string &imageStr, const std::string &path, const std::string &width, const std::string &height);

	static SurfacePtr getNewNotClear(int w, int h);

	static void loadImage(const std::string &desc);
	static SurfacePtr getImage(String desc);
};

#endif // IMAGE_H
