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
	SDL_PixelFormat format = surface->format;
	if (format == SDL_PIXELFORMAT_RGBA32) return surface;

	const int w = surface->w;
	const int h = surface->h;
	const int pitch = surface->pitch;
	const Uint8 *pixels = (const Uint8*)surface->pixels;

	SurfacePtr newSurface = ImageManipulator::getNewNotClear(w, h);
	const int newPitch = newSurface->pitch;
	Uint8 *newPixels = (Uint8*)newSurface->pixels;

	const SDL_Palette *palette = SDL_GetSurfacePalette(surface.get());
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
			for (int i = 0; i < w % 4; ++i) {
				const SDL_Color &color = palette->colors[*src++];

				dst[Rshift / 8] = color.r;
				dst[Gshift / 8] = color.g;
				dst[Bshift / 8] = color.b;
				dst[Ashift / 8] = color.a;

				dst += 4;
			}
		}
	}else {
		bool hasColorKey = SDL_SurfaceHasColorKey(surface.get());
		if (format == SDL_PIXELFORMAT_RGB24 && !hasColorKey) {
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
				for (int i = 0; i < w % 4; ++i) {
					dst[Rshift / 8] = src[Rshift / 8];
					dst[Gshift / 8] = src[Gshift / 8];
					dst[Bshift / 8] = src[Bshift / 8];
					dst[Ashift / 8] = 255;

					src += 3;
					dst += 4;
				}
			}
		}else {
			if (hasColorKey) {
				SDL_FillSurfaceRect(newSurface.get(), nullptr, 0);
			}
			SDL_BlitSurface(surface.get(), nullptr, newSurface.get(), nullptr);
		}
	}

	return newSurface;
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

	Utils::outMsg("SDL_CreateTextureFromSurface", SDL_GetError());
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
			Utils::outMsg("SDL_CreateSurface", SDL_GetError());
			return nullptr;
		}
		if (!SDL_WriteSurfacePixel(res.get(), 0, 0, 0, 0, 0, 255)) {
			Utils::outMsg("SDL_WriteSurfacePixel", SDL_GetError());
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
		Utils::outMsg("IMG_Load", SDL_GetError());
		return nullptr;
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
