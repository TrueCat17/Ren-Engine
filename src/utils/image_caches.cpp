#include "image_caches.h"

#include <vector>
#include <map>
#include <algorithm>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "config.h"
#include "gv.h"

#include "gui/display_object.h"
#include "media/image_manipulator.h"
#include "utils/string.h"
#include "utils/utils.h"


static std::map<SurfacePtr, std::pair<int, TexturePtr>> textures;

static std::mutex surfaceMutex;
static std::map<std::string, std::pair<int, SurfacePtr>> surfaces;


static void trimTexturesCache(const SurfacePtr &last) {
	/* If texture-cache size overpass need, then remove unused now textures while cache size more needed
	 * Textures removes in order last using
	 * If all textures uses now, nothing is removed
	 * After trim add new texture
	 */

	const size_t MAX_SIZE = size_t(String::toInt(Config::get("max_size_textures_cache"))) * (1 << 20);//in MegaBytes
	size_t cacheSize = size_t(last->w * last->h * 4);

	typedef std::map<SurfacePtr, std::pair<int, TexturePtr>>::const_iterator Iter;

	for (Iter it = textures.begin(); it != textures.end(); ++it) {
		const SurfacePtr &surface = it->first;
		cacheSize += size_t(surface->w * surface->h * 4);
	}
	if (cacheSize < MAX_SIZE) return;


	class KV {
	private:
		Iter iter;

	public:
		KV(const Iter &iter):
			iter(iter)
		{}

		const SurfacePtr& surface() const { return iter->first; }
		int time() const { return iter->second.first; }
		const TexturePtr& texture() const { return iter->second.second; }
	};

	std::vector<SurfacePtr> toDelete;
	std::vector<KV> candidatesToDelete;

	for (Iter it = textures.begin(); it != textures.end(); ++it) {
		KV kv(it);
		const SurfacePtr &surface = kv.surface();

		if (surface.use_count() == 1) {
			cacheSize -= size_t(surface->w * surface->h * 4);
			toDelete.push_back(surface);
		}else
		if (kv.texture().use_count() == 1) {//1 - value in textures
			candidatesToDelete.push_back(kv);
		}
	}

	if (cacheSize > MAX_SIZE) {
		std::sort(candidatesToDelete.begin(), candidatesToDelete.end(),
				  [](const KV &a, const KV &b) { return a.time() < b.time(); });

		for (const KV &kv : candidatesToDelete) {
			const SurfacePtr &surface = kv.surface();

			cacheSize -= size_t(surface->w * surface->h * 4);
			toDelete.push_back(surface);

			if (cacheSize < MAX_SIZE) break;
		}
	}

	for (const SurfacePtr &surface : toDelete) {
		textures.erase(surface);
	}
}

TexturePtr ImageCaches::getTexture(const SurfacePtr &surface) {
	if (!surface) return nullptr;

	auto it = textures.find(surface);
	if (it != textures.end()) {
		it->second.first = Utils::getTimer();
		return it->second.second;
	}

	TexturePtr texture(SDL_CreateTextureFromSurface(GV::mainRenderer, surface.get()), SDL_DestroyTexture);
	if (texture) {
		trimTexturesCache(surface);
		textures[surface] = std::make_pair(Utils::getTimer(), texture);
		SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND);
		return texture;
	}

	Utils::outMsg("SDL_CreateTextureFromSurface", SDL_GetError());
	return nullptr;
}
void ImageCaches::clearTextures() {
	textures.clear();
}

static void trimSurfacesCache(const SurfacePtr &last) {
	const size_t MAX_SIZE = size_t(String::toInt(Config::get("max_size_surfaces_cache"))) * (1 << 20);
	size_t cacheSize = size_t(last->w * last->h * 4);

	typedef std::map<std::string, std::pair<int, SurfacePtr>>::const_iterator Iter;

	std::map<SDL_Surface*, int> countSurfaces;
	for (Iter it = surfaces.begin(); it != surfaces.end(); ++it) {
		const SurfacePtr &surface = it->second.second;
		if (countSurfaces.find(surface.get()) == countSurfaces.end()) {
			cacheSize += size_t(surface->w * surface->h * 4);
		}
		countSurfaces[surface.get()] += 1;
	}
	if (cacheSize < MAX_SIZE) return;

	class KV {
	private:
		Iter iter;

	public:
		KV(const Iter &iter):
			iter(iter)
		{}

		const std::string &path() const { return iter->first; }
		int time() const { return iter->second.first; }
		const SurfacePtr& surface() const { return iter->second.second; }
	};

	std::map<SDL_Surface*, std::vector<std::string>> paths;
	std::vector<SDL_Surface*> toDelete;
	std::vector<KV> candidatesToDelete;

	std::map<SDL_Surface*, std::vector<DisplayObject*>> objs;

	for (Iter it = surfaces.begin(); it != surfaces.end(); ++it) {
		KV kv(it);
		const SurfacePtr &surface = kv.surface();
		paths[surface.get()].push_back(kv.path());
	}

	for (Iter it = surfaces.begin(); it != surfaces.end(); ++it) {
		KV kv(it);
		const SurfacePtr &surface = kv.surface();

		if (textures.find(surface) != textures.end()) continue;

		bool enabled = false;
		std::vector<DisplayObject*> &vec = objs[surface.get()];
		for (DisplayObject *obj : DisplayObject::objects) {
			if (obj->surface == surface) {
				if (obj->enable) {
					enabled = true;
					break;
				}
				vec.push_back(obj);
			}
		}
		if (enabled) continue;
		if (size_t(surface.use_count()) > objs.size() + paths[surface.get()].size()) continue;

		candidatesToDelete.push_back(kv);
	}
	std::sort(candidatesToDelete.begin(), candidatesToDelete.end(),
			  [](const KV &a, const KV &b) { return a.time() < b.time(); });

	for (const KV &kv : candidatesToDelete) {
		if (cacheSize < MAX_SIZE) break;

		const SurfacePtr &surface = kv.surface();

		if (!(countSurfaces[surface.get()] -= 1)) {
			cacheSize -= size_t(surface->w * surface->h * 4);
			toDelete.push_back(surface.get());
		}
	}

	for (SDL_Surface *surface : toDelete) {
		std::vector<DisplayObject*> &vec = objs[surface];
		for (DisplayObject *obj : vec) {
			obj->surface.reset();
		}

		for (const std::string &path : paths[surface]) {
			surfaces.erase(path);
		}
	}
}
SurfacePtr ImageCaches::getThereIsSurfaceOrNull(const std::string &path) {
	std::lock_guard g(surfaceMutex);

	auto it = surfaces.find(path);
	if (it != surfaces.end()) {
		it->second.first = Utils::getTimer();
		return it->second.second;
	}
	return nullptr;
}


SurfacePtr ImageCaches::getSurface(const std::string &path) {
	if (path.empty()) return nullptr;

	SurfacePtr t = getThereIsSurfaceOrNull(path);
	if (t) return t;

	static std::mutex mutex;
	std::lock_guard g(mutex);

	t = getThereIsSurfaceOrNull(path);
	if (t) return t;


	std::string realPath = path;
	size_t end = realPath.find('?');
	if (end != size_t(-1)) {
		realPath.erase(realPath.begin() + int(end), realPath.end());
	}
	for (size_t i = 0; i < realPath.size(); ++i) {
		if (realPath[i] == '\\') {
			realPath[i] = '/';
		}
	}

	SurfacePtr surface(IMG_Load(realPath.c_str()), SDL_FreeSurface);
	if (!surface) {
		Utils::outMsg("IMG_Load", IMG_GetError());
		return nullptr;
	}

	SDL_SetSurfaceBlendMode(surface.get(), SDL_BLENDMODE_NONE);

	const int w = surface->w;
	const int h = surface->h;
	const int pitch = surface->pitch;
	const Uint8 *pixels = (const Uint8*)surface->pixels;

	Uint32 format = surface->format->format;
	if (format != SDL_PIXELFORMAT_RGBA32) {
		SurfacePtr newSurface = ImageManipulator::getNewNotClear(w, h);
		const int newPitch = newSurface->pitch;
		Uint8 *newPixels = (Uint8*)newSurface->pixels;

		const SDL_Palette *palette = surface->format->palette;
		if (palette) {
			for (int y = 0; y < h; ++y) {
				const Uint8 *src = pixels + y * pitch;
				const Uint8 *srcEnd = src + w - (w % 4);
				Uint8 *dst = newPixels + y * newPitch;

				while (src != srcEnd) {
					const SDL_Color &color1 = palette->colors[*src++];
					const SDL_Color &color2 = palette->colors[*src++];
					const SDL_Color &color3 = palette->colors[*src++];
					const SDL_Color &color4 = palette->colors[*src++];

					dst[Rshift / 8] = color1.r;
					dst[Gshift / 8] = color1.g;
					dst[Bshift / 8] = color1.b;
					dst[Ashift / 8] = color1.a;

					dst[Rshift / 8 + 4] = color2.r;
					dst[Gshift / 8 + 4] = color2.g;
					dst[Bshift / 8 + 4] = color2.b;
					dst[Ashift / 8 + 4] = color2.a;

					dst[Rshift / 8 + 8] = color3.r;
					dst[Gshift / 8 + 8] = color3.g;
					dst[Bshift / 8 + 8] = color3.b;
					dst[Ashift / 8 + 8] = color3.a;

					dst[Rshift / 8 + 12] = color4.r;
					dst[Gshift / 8 + 12] = color4.g;
					dst[Bshift / 8 + 12] = color4.b;
					dst[Ashift / 8 + 12] = color4.a;

					dst += 16;
				}
				for (int i = 0; i < surface->w % 4; ++i) {
					const SDL_Color &color = palette->colors[*src++];

					dst[Rshift / 8] = color.r;
					dst[Gshift / 8] = color.g;
					dst[Bshift / 8] = color.b;
					dst[Ashift / 8] = color.a;

					dst += 4;
				}
			}
		}else

		if (format == SDL_PIXELFORMAT_RGB24) {
			for (int y = 0; y < surface->h; ++y) {
				const Uint8 *src = pixels + y * pitch;
				const Uint8 *srcEnd = src + (w - (w % 4)) * 3;
				Uint8 *dst = newPixels + y * newPitch;

				while (src != srcEnd) {
					dst[Rshift / 8] = src[Rshift / 8];
					dst[Gshift / 8] = src[Gshift / 8];
					dst[Bshift / 8] = src[Bshift / 8];
					dst[Ashift / 8] = 255;

					dst[Rshift / 8 + 4] = src[Rshift / 8 + 3];
					dst[Gshift / 8 + 4] = src[Gshift / 8 + 3];
					dst[Bshift / 8 + 4] = src[Bshift / 8 + 3];
					dst[Ashift / 8 + 4] = 255;

					dst[Rshift / 8 + 8] = src[Rshift / 8 + 6];
					dst[Gshift / 8 + 8] = src[Gshift / 8 + 6];
					dst[Bshift / 8 + 8] = src[Bshift / 8 + 6];
					dst[Ashift / 8 + 8] = 255;

					dst[Rshift / 8 + 12] = src[Rshift / 8 + 9];
					dst[Gshift / 8 + 12] = src[Gshift / 8 + 9];
					dst[Bshift / 8 + 12] = src[Bshift / 8 + 9];
					dst[Ashift / 8 + 12] = 255;

					src += 12;
					dst += 16;
				}
				for (int i = 0; i < surface->w % 4; ++i) {
					dst[Rshift / 8] = src[Rshift / 8];
					dst[Gshift / 8] = src[Gshift / 8];
					dst[Bshift / 8] = src[Bshift / 8];
					dst[Ashift / 8] = 255;

					src += 3;
					dst += 4;
				}
			}
		}

		else {
			SDL_BlitSurface(surface.get(), nullptr, newSurface.get(), nullptr);
		}

		surface = newSurface;
	}

	std::lock_guard g2(surfaceMutex);
	trimSurfacesCache(surface);
	surfaces[path] = std::make_pair(Utils::getTimer(), surface);

	return surface;
}

void ImageCaches::setSurface(const std::string &path, const SurfacePtr &surface) {
	if (!surface) return;
	SDL_SetSurfaceBlendMode(surface.get(), SDL_BLENDMODE_NONE);

	std::lock_guard g(surfaceMutex);
	trimSurfacesCache(surface);
	surfaces[path] = std::make_pair(Utils::getTimer(), surface);
}
