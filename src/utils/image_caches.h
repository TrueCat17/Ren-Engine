#ifndef IMAGE_CACHES_H
#define IMAGE_CACHES_H

#include <map>
#include <mutex>

#include "utils/image_typedefs.h"
#include "utils/string.h"

class ImageCaches {
private:
	static std::map<SurfacePtr, std::pair<int, TexturePtr>> textures;

	static std::mutex surfaceMutex;
	static std::map<String, std::pair<int, SurfacePtr>> surfaces;

	static void trimSurfacesCache(const SurfacePtr &last);
	static void trimTexturesCache(const SurfacePtr &last);

public:
	static SurfacePtr getThereIsSurfaceOrNull(const String &path);
	static SurfacePtr getSurface(const String &path);
	static void setSurface(const String &path, const SurfacePtr &surface);

	static TexturePtr getTexture(const SurfacePtr &surface);
	static void clearTextures() { textures.clear(); }
};

#endif // IMAGE_CACHES_H
