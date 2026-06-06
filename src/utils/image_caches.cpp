#include "image_caches.h"

#include <algorithm>
#include <map>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>

#include "config.h"
#include "renderer.h"

#include "gui/display_object.h"
#include "media/image_manipulator.h"
#include "utils/algo.h"
#include "utils/file_system.h"
#include "utils/message.h"
#include "utils/string.h"
#include "utils/utils.h"


static std::vector<std::string> supportedExts = { "png", "jpg", "jpeg", "webp" };

struct TextureWithInfo {
	TexturePtr texture;
	float time;
	int size;
};
static std::map<SurfacePtr, TextureWithInfo> textures;

static std::recursive_mutex surfaceMutex;
static std::map<std::string, std::pair<double, SurfacePtr>> surfaces;


bool ImageCaches::surfaceIsOpaque(const SurfacePtr &surface) {
	SDL_Palette *palette = SDL_GetSurfacePalette(surface.get());
	if (palette) {
		for (int i = 0; i < palette->ncolors; ++i) {
			if (palette->colors[i].a != 255) return false;
		}
		return true;
	}
	return !SDL_ISPIXELFORMAT_ALPHA(surface->format);
}


SurfacePtr ImageCaches::convertToRGBA32(const SurfacePtr &surface) {
	if (surface->format == SDL_PIXELFORMAT_RGBA32) return surface;

	SurfacePtr res = ImageManipulator::getNewNotClear(surface->w, surface->h);
	if (!surfaceIsOpaque(surface)) {
		if (!SDL_ClearSurface(res.get(), 0, 0, 0, 0)) {
			Message::outMsg("SDL_FillSurfaceRect", SDL_GetError());
		}
	}

	if (!SDL_BlitSurface(surface.get(), nullptr, res.get(), nullptr)) {
		Message::outMsg("SDL_BlitSurface", SDL_GetError());
	}

	return res;
}



static void trimTexturesCache(const SurfacePtr &last) {
	/* If texture-cache size overpass need, then remove unused now textures while cache size more needed
	 * Textures removes in order last using
	 * If all textures uses now, nothing is removed
	 * After trim add new texture
	 */

	const int MAX_SIZE = String::toInt(Config::get("max_size_textures_cache")) * (1 << 20);//in MegaBytes
	int cacheSize = last->h * last->pitch;

	for (const auto &[surface, textureWithInfo] : textures) {
		cacheSize += textureWithInfo.size;
	}
	if (cacheSize < MAX_SIZE) return;


	std::vector<SurfacePtr> toDelete;

	using Pair = std::pair<SurfacePtr, TextureWithInfo>;
	std::vector<Pair> candidatesToDelete;

	for (const auto &[surface, textureWithInfo] : textures) {
		if (surface.use_count() == 1) {
			cacheSize -= textureWithInfo.size;
			toDelete.push_back(surface);
		}else
		if (textureWithInfo.texture.use_count() == 1) {//1 - value in textures
			candidatesToDelete.push_back(std::make_pair(surface, textureWithInfo));
		}
	}

	if (cacheSize > MAX_SIZE) {
		std::sort(candidatesToDelete.begin(), candidatesToDelete.end(),
		          [](const Pair &a, const Pair &b) { return a.second.time < b.second.time; });

		for (const auto &[surface, textureWithInfo] : candidatesToDelete) {
			cacheSize -= textureWithInfo.size;
			toDelete.push_back(surface);

			if (cacheSize < MAX_SIZE) break;
		}
	}

	for (const SurfacePtr &surface : toDelete) {
		textures.erase(surface);
	}
}

TexturePtr ImageCaches::getTexture(SDL_Renderer *renderer, const SurfacePtr &surface) {
	if (!surface) return nullptr;

	auto it = textures.find(surface); 
	if (it != textures.end()) {
		it->second.time = float(Utils::getTimer());
		return it->second.texture;
	}

	//software renderer draws palette texture slow, convert it
	//"fastOpenGL" renderer does not know how to work with palettes, convert it too
	SurfacePtr src;
	if (SDL_GetSurfacePalette(surface.get())) {
		if (Renderer::driver == "software" || Renderer::useFastOpenGL()) {
			SDL_PixelFormat format = surfaceIsOpaque(surface) ? SDL_PIXELFORMAT_RGB24 : SDL_PIXELFORMAT_RGBA32;
			src = SDL_ConvertSurface(surface.get(), format);
		}
	}
	if (!src) {
		src = surface;
	}

	TexturePtr texture = SDL_CreateTextureFromSurface(renderer, src.get());
	if (texture) {
		trimTexturesCache(src);
		textures[surface] = TextureWithInfo{ texture, float(Utils::getTimer()), src->h * src->pitch };

		SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND);
		SDL_SetTextureScaleMode(texture.get(), Config::getScaleQuality());

		return texture;
	}

	Message::outMsg("SDL_CreateTextureFromSurface", SDL_GetError());
	return nullptr;
}
void ImageCaches::clearTextures() {
	textures.clear();
}

static void trimSurfacesCache(const SurfacePtr &last) {
	const size_t MAX_SIZE = size_t(String::toInt(Config::get("max_size_surfaces_cache"))) * (1 << 20);
	size_t cacheSize = size_t(last->h * last->pitch);

	using Iter = std::map<std::string, std::pair<double, SurfacePtr>>::const_iterator;

	std::map<SDL_Surface*, int> countSurfaces;
	for (Iter it = surfaces.cbegin(); it != surfaces.cend(); ++it) {
		const SurfacePtr &surface = it->second.second;
		if (countSurfaces.find(surface.get()) == countSurfaces.end()) {
			cacheSize += size_t(surface->h * surface->pitch);
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

		const std::string& path() const { return iter->first; }
		double time() const { return iter->second.first; }
		const SurfacePtr& surface() const { return iter->second.second; }
	};

	std::map<SDL_Surface*, std::vector<std::string>> paths;
	std::vector<SDL_Surface*> toDelete;
	std::vector<KV> candidatesToDelete;

	std::map<SDL_Surface*, std::vector<DisplayObject*>> objs;

	for (Iter it = surfaces.cbegin(); it != surfaces.cend(); ++it) {
		KV kv(it);
		const SurfacePtr &surface = kv.surface();
		paths[surface.get()].push_back(kv.path());
	}

	for (Iter it = surfaces.cbegin(); it != surfaces.cend(); ++it) {
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
			cacheSize -= size_t(surface->h * surface->pitch);
			toDelete.push_back(surface.get());
		}
	}

	for (SDL_Surface *surface : toDelete) {
		std::vector<DisplayObject*> &vec = objs[surface];
		for (DisplayObject *obj : vec) {
			obj->surface = nullptr;
		}

		for (const std::string &path : paths[surface]) {
			surfaces.erase(path);
		}
	}
}


SurfacePtr ImageCaches::getThereIsSurfaceOrNull(const std::string &path, bool formatRGBA32) {
	std::lock_guard g(surfaceMutex);

	if (formatRGBA32) {
		SurfacePtr res = getThereIsSurfaceOrNull(path, false);
		if (res && res->format != SDL_PIXELFORMAT_RGBA32) {
			res = convertToRGBA32(res);
			setSurface(path, res);
		}
		return res;
	}

	auto it = surfaces.find(path);
	if (it != surfaces.end()) {
		it->second.first = Utils::getTimer();
		return it->second.second;
	}
	return nullptr;
}



static bool unusualFormat(const SurfacePtr &surface) {
	if (SDL_GetSurfacePalette(surface.get())) return false;
	if (surface->format == SDL_PIXELFORMAT_RGBA32) return false;
	if (surface->format == SDL_PIXELFORMAT_RGB24 && !SDL_SurfaceHasColorKey(surface.get())) return false;

	return true;
}

SurfacePtr ImageCaches::getSurface(const std::string &path, bool formatRGBA32) {
	std::lock_guard g(surfaceMutex);

	SurfacePtr res;
	if (formatRGBA32) {
		res = getSurface(path, false);
		if (res && res->format != SDL_PIXELFORMAT_RGBA32) {
			res = convertToRGBA32(res);
			setSurface(path, res);
		}
		return res;
	}

	if (path.empty()) return nullptr;

	res = getThereIsSurfaceOrNull(path, formatRGBA32);
	if (res) return res;

	std::string realPath = path;
	size_t end = realPath.find('?');
	if (end != size_t(-1)) {
		realPath.erase(realPath.begin() + long(end), realPath.end());
	}
	for (size_t i = 0; i < realPath.size(); ++i) {
		if (realPath[i] == '\\') {
			realPath[i] = '/';
		}
	}

	if (String::startsWith(realPath, "images/bg/black.")) {
		res = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_RGBA32);
		if (!res) {
			Message::outMsg("SDL_CreateSurface", SDL_GetError());
			return res;
		}
		if (!SDL_WriteSurfacePixel(res.get(), 0, 0, 0, 0, 0, 255)) {
			Message::outMsg("SDL_WriteSurfacePixel", SDL_GetError());
			return nullptr;
		}

		setSurface(path, res);
		return res;
	}

	if (!FileSystem::exists(realPath)) {
		std::string fileName = FileSystem::getFileName(realPath);
		size_t dot = fileName.rfind('.');
		if (dot != size_t(-1)) {
			fileName.erase(dot);
		}

		std::string dir = FileSystem::getParentDirectory(realPath);
		for (const std::string &tmpFile : FileSystem::getFiles(dir)) {
			size_t tmpDot = tmpFile.rfind('.');
			if (tmpDot == size_t(-1)) continue;

			std::string tmpExt = tmpFile.substr(tmpDot + 1);
			if (!Algo::in(tmpExt, supportedExts)) continue;

			std::string tmpName = tmpFile.substr(dir.size(), tmpDot - dir.size());
			if (tmpName == fileName) {
				realPath = tmpFile;
				break;
			}
		}
	}

	res = IMG_Load(realPath.c_str());
	if (!res) {
		Message::outMsg("IMG_Load", SDL_GetError());
		return res;
	}

	SDL_Palette *palette = SDL_GetSurfacePalette(res.get());
	if (palette) {
		if (SDL_SurfaceHasColorKey(res.get())) {
			Uint32 colorKey;
			SDL_GetSurfaceColorKey(res.get(), &colorKey);

			SDL_Color &color = palette->colors[colorKey];
			color.r = color.g = color.b = color.a = 0;
		}
	}

	if (unusualFormat(res)) {
		res = convertToRGBA32(res);
	}
	setSurface(path, res);

	return res;
}

void ImageCaches::setSurface(const std::string &path, const SurfacePtr &surface) {
	if (!surface) return;
	SDL_SetSurfaceBlendMode(surface.get(), SDL_BLENDMODE_NONE);

	std::lock_guard g(surfaceMutex);
	trimSurfacesCache(surface);
	surfaces[path] = std::make_pair(Utils::getTimer(), surface);
}
