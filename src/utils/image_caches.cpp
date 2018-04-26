#include "image_caches.h"

#include <vector>
#include <algorithm>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "config.h"
#include "gv.h"

#include "gui/screen/screen_child.h"

#include "media/image.h"

#include "utils/utils.h"


std::map<SurfacePtr, std::pair<int, TexturePtr>> ImageCaches::textures;

std::mutex ImageCaches::surfaceMutex;
std::map<String, std::pair<int, SurfacePtr>> ImageCaches::surfaces;



void ImageCaches::trimTexturesCache(const SurfacePtr &last) {
	/* Если размер кэша текстур превышает желаемый, то удалять неиспользуемые сейчас текстуры до тех пор,
	 * пока кэш не уменьшится до нужных размеров
	 * Текстуры удаляются в порядке последнего использования (сначала те, что использовались давно)
	 * Может оказаться так, что все текстуры нужны, тогда никакая из них не удаляется и кэш превышает желаемый размер
	 * После всего этого новая текстура добавляется в конец кэша
	 */

	const size_t MAX_SIZE = Config::get("max_size_textures_cache").toInt() * (1 << 20);//в МБ
	size_t cacheSize = last->w * last->h * 4;

	typedef std::map<SurfacePtr, std::pair<int, TexturePtr>>::const_iterator Iter;

	for (Iter it = textures.begin(); it != textures.end(); ++it) {
		const SurfacePtr &surface = it->first;
		cacheSize += surface->w * surface->h * 4;
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
			cacheSize -= surface->w * surface->h * 4;
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
			if (cacheSize < MAX_SIZE) break;

			const SurfacePtr &surface = kv.surface();

			cacheSize -= surface->w * surface->h * 4;
			toDelete.push_back(surface);
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
		return texture;
	}

	Utils::outMsg("SDL_CreateTextureFromSurface", SDL_GetError());
	return nullptr;
}

void ImageCaches::trimSurfacesCache(const SurfacePtr &last) {
	const size_t MAX_SIZE = Config::get("max_size_surfaces_cache").toInt() * (1 << 20);
	size_t cacheSize = last->w * last->h * 4;

	typedef std::map<String, std::pair<int, SurfacePtr>>::const_iterator Iter;

	std::map<SDL_Surface*, int> countSurfaces;
	for (Iter it = surfaces.begin(); it != surfaces.end(); ++it) {
		const SurfacePtr &surface = it->second.second;
		if (countSurfaces.find(surface.get()) == countSurfaces.end()) {
			cacheSize += surface->w * surface->h * 4;
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

		const String &path() const { return iter->first; }
		int time() const { return iter->second.first; }
		const SurfacePtr& surface() const { return iter->second.second; }
	};

	std::map<SDL_Surface*, std::vector<String>> paths;
	std::vector<SDL_Surface*> toDelete;
	std::vector<KV> candidatesToDelete;

	const std::vector<ScreenChild*> &screenObjects = ScreenChild::getScreenObjects();
	std::map<SDL_Surface*, std::vector<ScreenChild*>> objs;

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
		std::vector<ScreenChild*> &vec = objs[surface.get()];
		for (ScreenChild *obj : screenObjects) {
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
			cacheSize -= surface->w * surface->h * 4;
			toDelete.push_back(surface.get());
		}
	}

	for (SDL_Surface *surface : toDelete) {
		std::vector<ScreenChild*> &vec = objs[surface];
		for (ScreenChild *obj : vec) {
			obj->surface.reset();
		}

		for (const String &path : paths[surface]) {
			surfaces.erase(path);
		}
	}
}
SurfacePtr ImageCaches::getThereIsSurfaceOrNull(const String &path) {
	std::lock_guard<std::mutex> g(surfaceMutex);

	auto it = surfaces.find(path);
	if (it != surfaces.end()) {
		it->second.first = Utils::getTimer();
		return it->second.second;
	}
	return nullptr;
}

SurfacePtr ImageCaches::getSurface(const String &path) {
	if (!path) return nullptr;

	SurfacePtr t = getThereIsSurfaceOrNull(path);
	if (t) return t;

	static std::mutex mutex;
	std::lock_guard<std::mutex> g(mutex);

	t = getThereIsSurfaceOrNull(path);
	if (t) return t;


	String realPath = path;
	size_t end = realPath.find('?');
	if (end != size_t(-1)) {
		realPath.erase(realPath.begin() + end, realPath.end());
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

	if (surface->format->format != SDL_PIXELFORMAT_RGBA32) {
		SurfacePtr newSurface = Image::getNewNotClear(surface->w, surface->h);

		const SDL_Palette *palette = surface->format->palette;
		if (palette) {
			for (int y = 0; y < surface->h; ++y) {
				const Uint8 *oldPixels = (const Uint8 *)surface->pixels + y * surface->pitch;
				Uint8 *newPixels = (Uint8 *)newSurface->pixels + y * newSurface->pitch;

				for (int x = 0; x < surface->w; ++x) {
					const SDL_Color &color = palette->colors[*oldPixels];

					newPixels[Rshift / 8] = color.r;
					newPixels[Gshift / 8] = color.g;
					newPixels[Bshift / 8] = color.b;
					newPixels[Ashift / 8] = color.a;

					++oldPixels;
					newPixels += 4;
				}
			}
		}else {
			SDL_BlitSurface(surface.get(), nullptr, newSurface.get(), nullptr);
		}
		surface = newSurface;
	}

	std::lock_guard<std::mutex> g2(surfaceMutex);
	trimSurfacesCache(surface);
	surfaces[path] = std::make_pair(Utils::getTimer(), surface);

	return surface;
}

void ImageCaches::setSurface(const String &path, const SurfacePtr &surface) {
	if (!surface) return;
	SDL_SetSurfaceBlendMode(surface.get(), SDL_BLENDMODE_NONE);

	std::lock_guard<std::mutex> g(surfaceMutex);
	trimSurfacesCache(surface);
	surfaces[path] = std::make_pair(Utils::getTimer(), surface);
}
