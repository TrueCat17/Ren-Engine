#ifndef IMAGE_CACHES_H
#define IMAGE_CACHES_H

#include <string>

#include "utils/image_typedefs.h"

class ImageCaches {
public:
	static SurfacePtr getThereIsSurfaceOrNull(const std::string &path, bool formatRGBA32 = true);
	static SurfacePtr getSurface(const std::string &path, bool formatRGBA32 = true);
	static void setSurface(const std::string &path, const SurfacePtr &surface);

	static TexturePtr getTexture(SDL_Renderer *renderer, const SurfacePtr &surface);
	static void clearTextures();
};

#endif // IMAGE_CACHES_H
