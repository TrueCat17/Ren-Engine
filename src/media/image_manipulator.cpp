#include "image_manipulator.h"

#include <thread>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <deque>

#include <cmath>
#include <cstring>

#include <SDL2/SDL_image.h>

#include "gv.h"
#include "renderer.h"

#include "utils/algo.h"
#include "utils/file_system.h"
#include "utils/image_caches.h"
#include "utils/math.h"
#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/utils.h"


static std::map<std::string, std::function<SurfacePtr(const std::vector<std::string>&)>> functions;

static std::deque<std::string> toLoadImages;
static std::deque<std::tuple<SurfacePtr, std::string, std::string, std::string>> toSaveImages;

static std::mutex preloadMutex;


static inline bool smallImage(int w, int h) {
	return h < 32 && w * h < 64 * 32;
}
static SurfacePtr getNotPaletteImage(const std::string &path) {
	SurfacePtr res = ImageManipulator::getImage(path, false);
	if (res && res->format->BitsPerPixel < 24) {
		res = ImageManipulator::getImage(path);
	}
	return res;
}


static void preloadThread();

static SurfacePtr scale(const std::vector<std::string> &args);
static SurfacePtr factorScale(const std::vector<std::string> &args);
static SurfacePtr rendererScale(const std::vector<std::string> &args);
static SurfacePtr crop(const std::vector<std::string> &args);
static SurfacePtr composite(const std::vector<std::string> &args);
static SurfacePtr flip(const std::vector<std::string> &args);
static SurfacePtr matrixColor(const std::vector<std::string> &args);
static SurfacePtr reColor(const std::vector<std::string> &args);
static SurfacePtr rotozoom(const std::vector<std::string> &args);
static SurfacePtr mask(const std::vector<std::string> &args);

static SurfacePtr blurH(const std::vector<std::string> &args);
static SurfacePtr blurV(const std::vector<std::string> &args);
static SurfacePtr motionBlur(const std::vector<std::string> &args);


void ImageManipulator::init() {
	functions["Scale"] = scale;
	functions["FactorScale"] = factorScale;
	functions["RendererScale"] = rendererScale;
	functions["Crop"] = crop;
	functions["Composite"] = composite;
	functions["Flip"] = flip;
	functions["MatrixColor"] = matrixColor;
	functions["ReColor"] = reColor;
	functions["Rotozoom"] = rotozoom;
	functions["Mask"] = mask;

	functions["BlurH"] = blurH;
	functions["BlurV"] = blurV;
	functions["MotionBlur"] = motionBlur;

	std::thread(preloadThread).detach();
}


SurfacePtr ImageManipulator::getNewNotClear(int w, int h, Uint32 format) {
	if (w <= 0 || h <= 0) return nullptr;

	if (format == SDL_PIXELFORMAT_UNKNOWN || SDL_BITSPERPIXEL(format) < 24) {
		format = SDL_PIXELFORMAT_RGBA32;
	}

	int pitch = w * int(SDL_BITSPERPIXEL(format)) / 8;
	pitch = (pitch + 3) & ~3;//align to 4
	void *pixels = SDL_malloc(size_t(h * pitch));

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormatFrom(pixels, w, h, SDL_BITSPERPIXEL(format), pitch, format),
				   SDL_FreeSurface);
	SDL_SetSurfaceBlendMode(res.get(), SDL_BLENDMODE_NONE);

	res->flags &= Uint32(~SDL_PREALLOC);
	return res;
}


void ImageManipulator::loadImage(const std::string &desc) {
	SurfacePtr image = ImageCaches::getThereIsSurfaceOrNull(desc, false);
	if (image) return;

	std::lock_guard g(preloadMutex);

	if (!Algo::in(desc, toLoadImages)) {
		toLoadImages.push_back(desc);
	}
}
void preloadThread() {
	Utils::setThreadName("preload");

	while (!GV::exit) {
		if (toSaveImages.empty()) {
			if (toLoadImages.empty()) {
				Utils::sleep(0.005, false);
			}else {
				std::string desc;
				{
					std::lock_guard g(preloadMutex);
					desc = toLoadImages.front();
					toLoadImages.pop_front();
				}
				ImageManipulator::getImage(desc, false);
			}
		}else {
			decltype(toSaveImages)::value_type tuple;
			{
				std::lock_guard g(preloadMutex);
				tuple = toSaveImages.front();
				toSaveImages.pop_front();
			}

			auto [img, path, width, height] = tuple;
			ImageManipulator::saveSurface(img, path, width, height, true);
		}
	}
}


static std::mutex vecMutex;
static std::vector<std::string> processingImages;

SurfacePtr ImageManipulator::getImage(std::string desc, bool formatRGBA32) {
	desc = Algo::clear(desc);

	SurfacePtr res = ImageCaches::getThereIsSurfaceOrNull(desc, formatRGBA32);
	if (res) return res;


	bool in = true;
	while (in) {
		{
			std::lock_guard g(vecMutex);
			in = Algo::in(desc, processingImages);
			if (!in) {
				res = ImageCaches::getThereIsSurfaceOrNull(desc, formatRGBA32);
				if (res) return res;

				processingImages.push_back(desc);
			}
		}
		if (in) {
			Utils::sleep(0.005);
		}
	}


	ScopeExit se([&]() {
		std::lock_guard g(vecMutex);

		for (size_t i = 0; i < processingImages.size(); ++i) {
			if (processingImages[i] == desc) {
				processingImages.erase(processingImages.begin() + long(i));
				return;
			}
		}
	});


	const std::vector<std::string> args = Algo::getArgs(desc, '|');

	if (args.empty()) {
		Utils::outMsg("ImageManipulator::getImage", "No arguments");
		return nullptr;
	}
	if (args.size() == 1) {
		const std::string imagePath = Algo::clear(args[0]);

		res = ImageCaches::getSurface(imagePath, formatRGBA32);
		return res;
	}


	const std::string &command = args[0];

	auto it = functions.find(command);
	if (it == functions.end()) {
		Utils::outMsg("ImageManipulator::getImage", "Unknown command <" + command + ">");
	}else {
		auto func = it->second;
		res = func(args);
	}

	ImageCaches::setSurface(desc, res);
	if (res && formatRGBA32 && res->format->format != SDL_PIXELFORMAT_RGBA32) {
		res = ImageCaches::getSurface(desc, true);
	}
	return res;
}


static SurfacePtr scale(const std::vector<std::string> &args) {
	if (args.size() != 4) {
		Utils::outMsg("ImageManipulator::scale", "Expected 3 arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = getNotPaletteImage(args[1]);
	if (!img) return nullptr;

	const int w = String::toInt(Algo::clear(args[2]));
	const int h = String::toInt(Algo::clear(args[3]));
	if (w <= 0 || h <= 0) {
		Utils::outMsg("ImageManipulator::scale", "Sizes are invalid:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	if (w == img->w && h == img->h) {
		return img;
	}

	SurfacePtr res = ImageManipulator::getNewNotClear(w, h, img->format->format);
	if (SDL_BlitScaled(img.get(), nullptr, res.get(), nullptr)) {
		Utils::outMsg("SDL_BlitScaled", SDL_GetError());
	}
	return res;
}

static SurfacePtr factorScale(const std::vector<std::string> &args) {
	if (args.size() != 3) {
		Utils::outMsg("ImageManipulator::factorScale", "Expected 2 arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = getNotPaletteImage(args[1]);
	if (!img) return nullptr;

	const double k = String::toDouble(Algo::clear(args[2]));
	if (k <= 0) {
		Utils::outMsg("ImageManipulator::factorScale", "Scale factor must be > 0:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	if (Math::doublesAreEq(k, 1)) {
		return img;
	}

	int w = std::max(int(img->w * k), 1);
	int h = std::max(int(img->h * k), 1);

	SurfacePtr res = ImageManipulator::getNewNotClear(w, h, img->format->format);
	if (SDL_BlitScaled(img.get(), nullptr, res.get(), nullptr)) {
		Utils::outMsg("SDL_BlitScaled", SDL_GetError());
	}
	return res;
}
static SurfacePtr rendererScale(const std::vector<std::string> &args) {
	if (args.size() != 4) {
		Utils::outMsg("ImageManipulator::rendererScale", "Expected 3 arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const int w = String::toInt(Algo::clear(args[2]));
	const int h = String::toInt(Algo::clear(args[3]));
	if (w <= 0 || h <= 0) {
		Utils::outMsg("ImageManipulator::rendererScale", "Sizes are invalid:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	if (w == img->w && h == img->h) {
		return img;
	}

	if (w > Renderer::info.max_texture_width || h > Renderer::info.max_texture_height) {
		return scale(args);
	}

	return Renderer::getScaled(img, w, h);
}

static SurfacePtr crop(const std::vector<std::string> &args) {
	if (args.size() != 3) {
		Utils::outMsg("ImageManipulator::crop", "Expected 2 arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	SurfacePtr img = getNotPaletteImage(args[1]);
	if (!img) return nullptr;

	const std::string rectStr = Algo::clear(args[2]);
	const std::vector<std::string> rectVec = String::split(rectStr, " ");
	if (rectVec.size() != 4) {
		Utils::outMsg("ImageManipulator::crop",
		              "Expected rect as a sequence with size 4:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	const int x = String::toInt(rectVec[0]);
	const int y = String::toInt(rectVec[1]);
	const int w = String::toInt(rectVec[2]);
	const int h = String::toInt(rectVec[3]);

	if (w <= 0 || h <= 0) {
		Utils::outMsg("ImageManipulator::crop", "Sizes are invalid:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	if (x + w > img->w || y + h > img->h) {
		Utils::outMsg("ImageManipulator::crop", std::string() +
		              "Crop area outside image:\n"
		              "Crop: (" + String::join(rectVec, ", ") + ")\n"
		              "Image size: " + std::to_string(img->w) + "x" + std::to_string(img->h) + "\n"
		              "<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	if (!x && !y && w == img->w && h == img->h) {
		return img;
	}

	SDL_Rect imgRect = {x, y, w, h};

	SurfacePtr res = ImageManipulator::getNewNotClear(w, h, img->format->format);
	if (SDL_BlitSurface(img.get(), &imgRect, res.get(), nullptr)) {
		Utils::outMsg("SDL_BlitSurface", SDL_GetError());
	}

	return res;
}

static SurfacePtr composite(const std::vector<std::string> &args) {
	if (args.size() % 2) {
		Utils::outMsg("ImageManipulator::composite", "Expected odd count of arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	const std::string sizeStr = Algo::clear(args[1]);
	const std::vector<std::string> sizeVec = String::split(sizeStr, " ");

	const int resW = sizeVec.size() == 2 ? String::toInt(sizeVec[0]) : 0;
	const int resH = sizeVec.size() == 2 ? String::toInt(sizeVec[1]) : 0;
	if (sizeVec.size() != 2 || resW <= 0 || resH <= 0) {
		Utils::outMsg("ImageManipulator::composite", "Sizes are invalid <" + sizeStr + ">");
		return nullptr;
	}

	if (args.size() == 4) {
		const std::string posStr = Algo::clear(args[2]);
		const std::string imagePath = Algo::clear(args[3]);
		SurfacePtr image = ImageManipulator::getImage(imagePath);

		if (!image) {
			Utils::outMsg("ImageManipulator::composite", "Failed to get image <" + imagePath + ">");
			return image;
		}

		const std::string imSizeStr = std::to_string(image->w) + ' ' + std::to_string(image->h);
		if (posStr == "0 0" && imSizeStr == sizeStr) {
			return image;
		}
	}


	std::map<std::string, size_t> images;
	for (size_t i = 2; i < args.size(); i += 2) {
		const std::string image = Algo::clear(args[i + 1]);

		if (!images.count(image)) {
			images[image] = i;
		}
	}
	if (!images.empty()) {
		typedef std::pair<std::string, size_t> P;
		std::vector<P> imagesPairs(images.begin(), images.end());

		std::sort(imagesPairs.begin(), imagesPairs.end(),
				  [](const P &a, const P &b) { return a.second < b.second; });

		std::lock_guard g(preloadMutex);

		for (const P &p : imagesPairs) {
			const std::string &desc = p.first;
			if (!Algo::in(desc, toLoadImages) && !ImageCaches::getThereIsSurfaceOrNull(desc)) {
				toLoadImages.push_back(desc);
			}
		}

	}

	SurfacePtr res = ImageManipulator::getNewNotClear(resW, resH);
	Uint8 *resPixels = (Uint8*)res->pixels;
	const int resPitch = res->pitch;


	const std::string firstImgStr = args.size() > 2 ? Algo::clear(args[3]) : "";
	SurfacePtr firstImg = firstImgStr.empty() ? nullptr : ImageManipulator::getImage(firstImgStr);
	if (!firstImg) {
		if (args.size() > 2) {
			Utils::outMsg("ImageManipulator::composite", "Failed to get image <" + firstImgStr + ">");
		}
		::memset(resPixels, 0, size_t(resH * resPitch));
	}else {
		const std::string firstPosStr = Algo::clear(args[2]);
		const std::vector<std::string> firstPosVec = String::split(firstPosStr, " ");
		if (firstPosVec.size() != 2) {
			Utils::outMsg("ImageManipulator::composite",
			              "Expected pos as a sequence with size 2:\n<" + String::join(args, ", ") + ">");
			return nullptr;
		}

		int xOn = String::toInt(firstPosVec[0]);
		int yOn = String::toInt(firstPosVec[1]);
		int w = firstImg->w;
		int h = firstImg->h;

		int xFrom = 0;
		int yFrom = 0;

		if (xOn < 0) {
			w += xOn;
			xFrom = -xOn;
			xOn = 0;
		}
		if (yOn < 0) {
			h += yOn;
			yFrom = -yOn;
			yOn = 0;
		}
		if (w > resW - xOn) w = resW - xOn;
		if (h > resH - yOn) h = resH - yOn;


		if (yOn > 0) {
			::memset(resPixels, 0, size_t(yOn * resPitch));//up
		}

		const int left = xOn * 4;
		const int center = firstImg->pitch;
		const int right = res->pitch - (left + center);

		//image - center
		SDL_Rect from = {xFrom, yFrom, w, h};
		SDL_Rect to = {xOn, yOn, w, h};
		SDL_BlitScaled(firstImg.get(), &from, res.get(), &to);

		for (int y = yOn; y < yOn + h; ++y) {
			if (left > 0) {
				::memset(resPixels + y * resPitch, 0, size_t(left));//left
			}
			if (right > 0) {
				::memset(resPixels + y * resPitch + left + center, 0, size_t(right));//right
			}
		}

		if (resH > yOn + h) {
			::memset(resPixels + (yOn + h) * resPitch, 0, size_t((resH - (yOn + h)) * resPitch));//down
		}
	}


	for (size_t i = 4; i < args.size(); i += 2) {
		const std::string posStr = Algo::clear(args[i]);
		const std::vector<std::string> posVec = String::split(posStr, " ");
		if (posVec.size() != 2) {
			Utils::outMsg("ImageManipulator::composite",
			              "Expected pos as a sequence with size 2:\n<" + String::join(args, ", ") + ">");
			return nullptr;
		}

		const std::string imgStr = Algo::clear(args[i + 1]);
		SurfacePtr img = ImageManipulator::getImage(imgStr);
		if (!img) {
			Utils::outMsg("ImageManipulator::composite", "Failed to get image <" + imgStr + ">");
			continue;
		}

		int xOn = String::toInt(posVec[0]);
		int yOn = String::toInt(posVec[1]);
		if (xOn >= resW || yOn >= resH) continue;
		if (xOn + img->w < 0 || yOn + img->h < 0) continue;

		int xFrom = 0;
		int yFrom = 0;

		int w = img->w;
		int h = img->h;
		if (xOn < 0) {
			w += xOn;
			xFrom = -xOn;
			xOn = 0;
		}
		if (yOn < 0) {
			h += yOn;
			yFrom = -yOn;
			yOn = 0;
		}
		if (w > resW - xOn) w = resW - xOn;
		if (h > resH - yOn) h = resH - yOn;

		const Uint8 *imgPixels = (const Uint8*)img->pixels;
		const int imgPitch = img->pitch;

		auto processLine = [&](int y) {
			const Uint8 *src = imgPixels + (y + yFrom) * imgPitch + xFrom * 4;
			Uint8 *dst = resPixels + (y + yOn) * resPitch + xOn * 4;

			const Uint8 *dstEnd = dst + w * 4;
			while (dst < dstEnd) {
				const Uint16 srcA = src[Ashift / 8];
				if (srcA == 255) {
					*(Uint32*)dst = *(const Uint32*)src;
				}else if (srcA) {
					const Uint16 dstA = dst[Ashift / 8];

					if (!dstA) {
						*(Uint32*)dst = *(const Uint32*)src;
					}else {
						const Uint16 blend = dstA * (255 - srcA);
						const Uint16 outA = std::min<Uint16>(srcA + (blend + 128) / 256, 255);
						const Uint16 sA = srcA * 256 / outA;
						const Uint16 dA = blend / outA;

						const Uint8 srcR = src[Rshift / 8];
						const Uint8 srcG = src[Gshift / 8];
						const Uint8 srcB = src[Bshift / 8];

						const Uint8 dstR = dst[Rshift / 8];
						const Uint8 dstG = dst[Gshift / 8];
						const Uint8 dstB = dst[Bshift / 8];

						dst[Rshift / 8] = Uint8((srcR * sA + dstR * dA + 128) / 256);
						dst[Gshift / 8] = Uint8((srcG * sA + dstG * dA + 128) / 256);
						dst[Bshift / 8] = Uint8((srcB * sA + dstB * dA + 128) / 256);
						dst[Ashift / 8] = Uint8(outA);
					}
				}

				src += 4;
				dst += 4;
			}
		};

		if (smallImage(w, h)) {
			for (int y = 0; y < h; ++y) {
				processLine(y);
			}
		}else {
#pragma omp parallel for
			for (int y = 0; y < h; ++y) {
				processLine(y);
			}
		}
	}
	return res;
}

static SurfacePtr flip(const std::vector<std::string> &args) {
	if (args.size() != 4) {
		Utils::outMsg("ImageManipulator::flip", "Expected 3 arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const bool horizontal = Algo::clear(args[2]) == "True";
	const bool vertical = Algo::clear(args[3]) == "True";
	if (!horizontal && !vertical) {
		return img;
	}

	const int w = img->w;
	const int h = img->h;

	SurfacePtr res = ImageManipulator::getNewNotClear(w, h);

	const Uint32 *imgPixels = (const Uint32*)img->pixels;
	Uint32 *resPixels = (Uint32*)res->pixels;

	auto processLine = [&](int y) {
		const int fromX = horizontal ? w - 1 : 0;
		const int fromY = vertical ? h - y - 1 : y;
		const Uint32 *imgPixel = imgPixels + fromY * w + fromX;

		Uint32 *resPixel = resPixels + y * w;

		if (horizontal) {
			const Uint32 *endResPixel = resPixel + w;
			while (resPixel != endResPixel) {
				*resPixel++ = *imgPixel--;
			}
		}else {
			::memcpy(resPixel, imgPixel, size_t(w) * 4);
		}
	};

	if (smallImage(w, h)) {
		for (int y = 0; y < h; ++y) {
			processLine(y);
		}
	}else {
#pragma omp parallel for
		for (int y = 0; y < h; ++y) {
			processLine(y);
		}
	}

	return res;
}

static SurfacePtr matrixColor(const std::vector<std::string> &args) {
	if (args.size() != 3) {
		Utils::outMsg("ImageManipulator::matrixColor", "Expected 2 arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const std::string matrixStr = Algo::clear(args[2]);
	const std::vector<std::string> matrixVec = String::split(matrixStr, " ");
	if (matrixVec.size() != 25) {
		Utils::outMsg("ImageManipulator::matrixColor",
		              "Expected matrix size is 25,\n"
		              "Matrix: <" + matrixStr + ">");
		return img;
	}

	std::vector<double> matrix;
	const int matrixSizeToUse = 20;
	matrix.resize(matrixSizeToUse);
	for (size_t i = 0; i < matrixSizeToUse; ++i) {//using 20 of 25 values
		matrix[i] = String::toDouble(matrixVec[i]);
	}

	static const std::vector<double> identity = {
		1, 0, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 1, 0
	};
	if (matrix == identity) {
		return img;
	}

	const Uint8 *imgPixels = (const Uint8*)img->pixels;

	const int w = img->w;
	const int h = img->h;
	const int imgPitch = img->pitch;

	SurfacePtr res = ImageManipulator::getNewNotClear(w, h);
	Uint8 *resPixels = (Uint8*)res->pixels;
	if (Math::doublesAreEq(matrix[15], 0) &&
	    Math::doublesAreEq(matrix[16], 0) &&
	    Math::doublesAreEq(matrix[17], 0) &&
	    Math::doublesAreEq(matrix[18], 0) &&
	    Math::doublesAreEq(matrix[19], 0))
	{
		::memset(resPixels, 0, size_t(w * h * 4));
		return res;
	}

	const double extraR = matrix[ 4] * 255;
	const double extraG = matrix[ 9] * 255;
	const double extraB = matrix[14] * 255;
	const double extraA = matrix[19] * 255;

	const bool extraAisNot0 = !Math::doublesAreEq(extraA, 0);

	auto processLine = [&](int y) {
		const Uint8 *src = imgPixels + y * imgPitch;
		Uint8 *dst = resPixels + y * imgPitch;

		const Uint8 *dstEnd = dst + imgPitch;
		while (dst != dstEnd) {
			if (*(const Uint32*)(src) || extraAisNot0) {
				const Uint8 oldR = src[Rshift / 8];
				const Uint8 oldG = src[Gshift / 8];
				const Uint8 oldB = src[Bshift / 8];
				const Uint8 oldA = src[Ashift / 8];

				const Uint8 newA = Uint8(Math::inBounds(matrix[15] * oldR + matrix[16] * oldG + matrix[17] * oldB + matrix[18] * oldA + extraA, 0, 255));
				if (newA) {
					const Uint8 newR = Uint8(Math::inBounds(matrix[ 0] * oldR + matrix[ 1] * oldG + matrix[ 2] * oldB + matrix[ 3] * oldA + extraR, 0, 255));
					const Uint8 newG = Uint8(Math::inBounds(matrix[ 5] * oldR + matrix[ 6] * oldG + matrix[ 7] * oldB + matrix[ 8] * oldA + extraG, 0, 255));
					const Uint8 newB = Uint8(Math::inBounds(matrix[10] * oldR + matrix[11] * oldG + matrix[12] * oldB + matrix[13] * oldA + extraB, 0, 255));

					dst[Rshift / 8] = newR;
					dst[Gshift / 8] = newG;
					dst[Bshift / 8] = newB;
					dst[Ashift / 8] = newA;
				}else {
					*(Uint32*)dst = 0;
				}
			}else {
				*(Uint32*)dst = 0;
			}

			src += 4;
			dst += 4;
		}
	};

	if (smallImage(w, h)) {
		for (int y = 0; y < h; ++y) {
			processLine(y);
		}
	}else {
#pragma omp parallel for
		for (int y = 0; y < h; ++y) {
			processLine(y);
		}
	}

	return res;
}

static SurfacePtr reColor(const std::vector<std::string> &args) {
	if (args.size() != 3) {
		Utils::outMsg("ImageManipulator::reColor", "Expected 2 arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const std::string colorsStr = Algo::clear(args[2]);
	const std::vector<std::string> colorsVec = String::split(colorsStr, " ");
	if (colorsVec.size() != 4) {
		Utils::outMsg("ImageManipulator::reColor", "Expected 4 colors:\n<" + colorsStr + ">");
		return img;
	}

	const Uint16 r = Uint16(String::toDouble(colorsVec[0]));
	const Uint16 g = Uint16(String::toDouble(colorsVec[1]));
	const Uint16 b = Uint16(String::toDouble(colorsVec[2]));
	const Uint16 a = Uint16(String::toDouble(colorsVec[3]));

	if (r == 256 && g == 256 && b == 256 && a == 256) {
		return img;
	}

	const Uint8 *imgPixels = (const Uint8*)img->pixels;

	const int w = img->w;
	const int h = img->h;
	const int imgPitch = img->pitch;

	SurfacePtr res = ImageManipulator::getNewNotClear(w, h);
	Uint8 *resPixels = (Uint8*)res->pixels;
	if (!a) {
		::memset(resPixels, 0, size_t(w * h * 4));
		return res;
	}

	auto processLine = [&](int y) {
		const Uint8 *src = imgPixels + y * imgPitch;
		Uint8 *dst = resPixels + y * imgPitch;

		const Uint8 *dstEnd = dst + imgPitch;
		while (dst != dstEnd) {
			if (src[Ashift / 8]) {
				dst[Rshift / 8] = Uint8(src[Rshift / 8] * r / 256);
				dst[Gshift / 8] = Uint8(src[Gshift / 8] * g / 256);
				dst[Bshift / 8] = Uint8(src[Bshift / 8] * b / 256);
				dst[Ashift / 8] = Uint8(src[Ashift / 8] * a / 256);
			}else {
				*(Uint32*)dst = 0;
			}

			src += 4;
			dst += 4;
		}
	};

	if (smallImage(w, h)) {
		for (int y = 0; y < h; ++y) {
			processLine(y);
		}
	}else {
#pragma omp parallel for
		for (int y = 0; y < h; ++y) {
			processLine(y);
		}
	}

	return res;
}

static SurfacePtr rotozoom(const std::vector<std::string> &args) {
	if (args.size() != 4) {
		Utils::outMsg("ImageManipulator::rotozoom", "Expected 3 arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1], false);
	if (!img) return nullptr;

	const int angle = int(-String::toDouble(Algo::clear(args[2])));
	double zoom = String::toDouble(Algo::clear(args[3]));
	if (Math::doublesAreEq(zoom, 0)) {
		Utils::outMsg("ImageManipulator::rotozoom", "zoom must be not equal 0");
		zoom = 1;
	}
	if (Math::doublesAreEq(zoom, 1) && angle == 0) {
		return img;
	}

	SDL_RendererFlip flip = SDL_FLIP_NONE;
	if (zoom < 0) {
		zoom = -zoom;
		flip = SDL_RendererFlip(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
	}

	const int w = std::max(int(img->w * zoom), 1);
	const int h = std::max(int(img->h * zoom), 1);

	auto abs = [](float d) { return d < 0 ? -d : d; };

	const float absSin = abs(Math::getSin(angle));
	const float absCos = abs(Math::getCos(angle));
	const int resW = int(float(w) * absCos + float(h) * absSin);
	const int resH = int(float(w) * absSin + float(h) * absCos);

	const SDL_Rect dstRect = {(resW - w) / 2, (resH - h) / 2, w, h};

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(0, resW, resH, 32, SDL_PIXELFORMAT_RGBA32),
				   SDL_FreeSurface);

	SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(res.get());
	ScopeExit se([&]() { SDL_DestroyRenderer(renderer); });

	if (!renderer) {
		Utils::outMsg("ImageManipulator::rotozoom, SDL_CreateSoftwareRenderer", SDL_GetError());
		return nullptr;
	}

	TexturePtr texture(SDL_CreateTextureFromSurface(renderer, img.get()), SDL_DestroyTexture);
	if (!texture) {
		Utils::outMsg("ImageManipulator::rotozoom, SDL_CreateTextureFromSurface", SDL_GetError());
		return nullptr;
	}

	if (SDL_RenderCopyEx(renderer, texture.get(), nullptr, &dstRect, angle, nullptr, flip)) {
		Utils::outMsg("ImageManipulator::rotozoom, SDL_RenderCopyEx", SDL_GetError());
		return nullptr;
	}

	return res;
}



static inline Uint8 getRed  (const Uint8 *pixel) { return pixel[Rshift / 8]; }
static inline Uint8 getGreen(const Uint8 *pixel) { return pixel[Gshift / 8]; }
static inline Uint8 getBlue (const Uint8 *pixel) { return pixel[Bshift / 8]; }
static inline Uint8 getAlpha(const Uint8 *pixel) { return pixel[Ashift / 8]; }

static inline const Uint8* get1(const Uint8 *a, const Uint8 *) { return a; }
static inline const Uint8* get2(const Uint8 *, const Uint8 *a) { return a; }

template<typename GetChannel, typename CmpFunc, typename GetPixel, typename GetAlpha>
static void maskCycles(const int value,
					   SurfacePtr img, const Uint8 *maskPixels, Uint8 *resPixels,
					   GetChannel getChannel, CmpFunc cmp,
					   GetPixel getPixel, GetAlpha getAlphaFunc)
{
	const Uint8 *imgPixels = (const Uint8*)img->pixels;

	const int w = img->w;
	const int h = img->h;
	const int pitch = img->pitch;

	auto processLine = [&](int y) {
		const Uint8 *mask = maskPixels + y * pitch;
		const Uint8 *src  = imgPixels  + y * pitch;
		Uint8 *dst        = resPixels  + y * pitch;

		const Uint8 *dstEnd = dst + pitch;
		while (dst != dstEnd) {
			const Uint8 channel = getChannel(mask);

			if (cmp(channel, value)) {
				const Uint8 a = getAlphaFunc(getPixel(src, mask));

				if (a) {
					dst[Rshift / 8] = src[Rshift / 8];
					dst[Gshift / 8] = src[Gshift / 8];
					dst[Bshift / 8] = src[Bshift / 8];
					dst[Ashift / 8] = a;
				}else {
					*(Uint32*)dst = 0;
				}
			}else {
				*(Uint32*)dst = 0;
			}

			mask += 4;
			src += 4;
			dst += 4;
		}
	};

	if (smallImage(w, h)) {
		for (int y = 0; y < h; ++y) {
			processLine(y);
		}
	}else {
#pragma omp parallel for
		for (int y = 0; y < h; ++y) {
			processLine(y);
		}
	}
}
template<typename GetChannel, typename GetPixel, typename GetAlpha>
static void maskChooseCmp(const int value, const std::string &cmp,
						  SurfacePtr img, const Uint8 *maskPixels, Uint8 *resPixels,
						  GetChannel getChannel,
						  GetPixel getPixel, GetAlpha getAlphaFunc)
{
	if (cmp == "l") {
		maskCycles(value, img, maskPixels, resPixels, getChannel, std::less<int>(), getPixel, getAlphaFunc);
	}else
	if (cmp == "g") {
		maskCycles(value, img, maskPixels, resPixels, getChannel, std::greater<int>(), getPixel, getAlphaFunc);
	}else
	if (cmp == "e") {
		maskCycles(value, img, maskPixels, resPixels, getChannel, std::equal_to<int>(), getPixel, getAlphaFunc);
	}else
	if (cmp == "ne") {
		maskCycles(value, img, maskPixels, resPixels, getChannel, std::not_equal_to<int>(), getPixel, getAlphaFunc);
	}else
	if (cmp == "le") {
		maskCycles(value, img, maskPixels, resPixels, getChannel, std::less_equal<int>(), getPixel, getAlphaFunc);
	}else {//ge
		maskCycles(value, img, maskPixels, resPixels, getChannel, std::greater_equal<int>(), getPixel, getAlphaFunc);
	}
}
template<typename GetChannel>
static void maskChooseAlpha(const char alphaChannel, bool alphaFromImage,
                            const int value, const std::string &cmp,
							SurfacePtr img, const Uint8 *maskPixels, Uint8 *resPixels,
							GetChannel getChannel)
{
	if (alphaChannel == 'r') {
		if (alphaFromImage) {
			maskChooseCmp(value, cmp, img, maskPixels, resPixels, getChannel, get1, getRed);
		}else {
			maskChooseCmp(value, cmp, img, maskPixels, resPixels, getChannel, get2, getRed);
		}
	}else
	if (alphaChannel == 'g') {
		if (alphaFromImage) {
			maskChooseCmp(value, cmp, img, maskPixels, resPixels, getChannel, get1, getGreen);
		}else {
			maskChooseCmp(value, cmp, img, maskPixels, resPixels, getChannel, get2, getGreen);
		}
	}else
	if (alphaChannel == 'b') {
		if (alphaFromImage) {
			maskChooseCmp(value, cmp, img, maskPixels, resPixels, getChannel, get1, getBlue);
		}else {
			maskChooseCmp(value, cmp, img, maskPixels, resPixels, getChannel, get2, getBlue);
		}
	}else {//a
		if (alphaFromImage) {
			maskChooseCmp(value, cmp, img, maskPixels, resPixels, getChannel, get1, getAlpha);
		}else {
			maskChooseCmp(value, cmp, img, maskPixels, resPixels, getChannel, get2, getAlpha);
		}
	}
}

static SurfacePtr mask(const std::vector<std::string> &args) {
	if (args.size() != 8) {
		Utils::outMsg("ImageManipulator::mask", "Expected 7 arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	SurfacePtr mask = ImageManipulator::getImage(args[2]);
	if (!mask) return nullptr;

	if (img->w != mask->w || img->h != mask->h) {
		const std::string imgSize = std::to_string(img->w) + "x" + std::to_string(img->h);
		const std::string maskSize = std::to_string(mask->w) + "x" + std::to_string(mask->h);
		Utils::outMsg("ImageManipulator::mask", "Mask sizes " + maskSize + " not equal image sizes " + imgSize + ":\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	const std::string channelStr = Algo::clear(args[3]);
	if (channelStr != "r" && channelStr != "g" && channelStr != "b" && channelStr != "a") {
		Utils::outMsg("ImageManipulator::mask", "Unexpected channel <" + channelStr + ">, expected r, g, b or a:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	const char channel = channelStr[0];

	const std::string valueStr = Algo::clear(args[4]);
	if (!String::isNumber(valueStr)) {
		Utils::outMsg("ImageManipulator::mask", "Value <" + valueStr + "> must be number:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	const int value = int(String::toDouble(valueStr));

	const std::string cmp = Algo::clear(args[5]);
	static const std::vector<std::string> cmps = {"l", "g", "e", "ne", "le", "ge"};
	if (!Algo::in(cmp, cmps)) {
		Utils::outMsg("ImageManipulator::mask", "Unexpected compare function <" + cmp + ">, expected l(<), g(>), e(==), ne(!=), le(<=), ge(>=):\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	const std::string alphaStr = Algo::clear(args[6]);
	if (alphaStr != "r" && alphaStr != "g" && alphaStr != "b" && alphaStr != "a") {
		Utils::outMsg("ImageManipulator::mask", "Unexpected channel <" + alphaStr + ">, expected r, g, b or a:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	const char alphaChannel = alphaStr[0];

	const std::string alphaImage = Algo::clear(args[7]);
	if (alphaImage != "1" && alphaImage != "2") {
		Utils::outMsg("ImageManipulator::mask", "Unexpected image num <" + alphaStr + ">, expected 1 (image) or 2 (mask):\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}


	const Uint8 *maskPixels = (Uint8*)mask->pixels;

	SurfacePtr res = ImageManipulator::getNewNotClear(img->w, img->h);
	Uint8 *resPixels = (Uint8*)res->pixels;


	if (channel == 'r') {
		maskChooseAlpha(alphaChannel, alphaImage == "1", value, cmp, img, maskPixels, resPixels, getRed);
	}else
	if (channel == 'g') {
		maskChooseAlpha(alphaChannel, alphaImage == "1", value, cmp, img, maskPixels, resPixels, getGreen);
	}else
	if (channel == 'b') {
		maskChooseAlpha(alphaChannel, alphaImage == "1", value, cmp, img, maskPixels, resPixels, getBlue);
	}else {//a
		maskChooseAlpha(alphaChannel, alphaImage == "1", value, cmp, img, maskPixels, resPixels, getAlpha);
	}

	return res;
}


static SurfacePtr blurH(const std::vector<std::string> &args) {
	if (args.size() != 3) {
		Utils::outMsg("ImageManipulator::blurH", "Expected 2 arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const int w = img->w;
	const int h = img->h;
	const int pitch = img->pitch;
	const Uint8 *pixels = (const Uint8 *)img->pixels;


	const int dist = std::min(String::toInt(args[2]), w);
	if (dist < 0) {
		Utils::outMsg("ImageManipulator::blurH", "Blur distance must be >= 0:\n<" + String::join(args, ", ") + ">");
		return img;
	}
	if (!dist) {
		return img;
	}

	SurfacePtr res = ImageManipulator::getNewNotClear(w, h);
	Uint8 *resPixels = (Uint8 *)res->pixels;

	auto processLine = [&](int y) {
		const Uint8 *src = pixels + y * pitch;
		Uint8 *dst = resPixels + y * pitch;

		for (int x = 0; x < w; ++x) {
			Uint32 r, g, b, a;
			r = g = b = a = 0;

			const Uint32 xStart = Uint32(std::max(0, x - dist));
			const Uint32 xEnd = Uint32(std::min(x + dist + 1, w));
			const Uint32 count = xEnd - xStart;

			const Uint8 *pixel = src + xStart * 4;
			const Uint8 *endPixel = src + (xEnd - (count % 8)) * 4;
			while (pixel != endPixel) {
				r += (pixel[Rshift / 8] + pixel[Rshift / 8 + 4]) + (pixel[Rshift / 8 + 8] + pixel[Rshift / 8 + 12]) + (pixel[Rshift / 8 + 16] + pixel[Rshift / 8 + 20]) + (pixel[Rshift / 8 + 24] + pixel[Rshift / 8 + 28]);
				g += (pixel[Gshift / 8] + pixel[Gshift / 8 + 4]) + (pixel[Gshift / 8 + 8] + pixel[Gshift / 8 + 12]) + (pixel[Gshift / 8 + 16] + pixel[Gshift / 8 + 20]) + (pixel[Gshift / 8 + 24] + pixel[Gshift / 8 + 28]);
				b += (pixel[Bshift / 8] + pixel[Bshift / 8 + 4]) + (pixel[Bshift / 8 + 8] + pixel[Bshift / 8 + 12]) + (pixel[Bshift / 8 + 16] + pixel[Bshift / 8 + 20]) + (pixel[Bshift / 8 + 24] + pixel[Bshift / 8 + 28]);
				a += (pixel[Ashift / 8] + pixel[Ashift / 8 + 4]) + (pixel[Ashift / 8 + 8] + pixel[Ashift / 8 + 12]) + (pixel[Ashift / 8 + 16] + pixel[Ashift / 8 + 20]) + (pixel[Ashift / 8 + 24] + pixel[Ashift / 8 + 28]);

				pixel += 32;
			}
			for (size_t i = 0; i < count % 8; ++i) {
				r += pixel[Rshift / 8];
				g += pixel[Gshift / 8];
				b += pixel[Bshift / 8];
				a += pixel[Ashift / 8];

				pixel += 4;
			}

			if (a) {
				dst[Rshift / 8] = Uint8(r / count);
				dst[Gshift / 8] = Uint8(g / count);
				dst[Bshift / 8] = Uint8(b / count);
				dst[Ashift / 8] = Uint8(a / count);
			}else {
				*(Uint32*)dst = 0;
			}
			dst += 4;
		}
	};

	if (smallImage(w, h)) {
		for (int y = 0; y < h; ++y) {
			processLine(y);
		}
	}else {
#pragma omp parallel for
		for (int y = 0; y < h; ++y) {
			processLine(y);
		}
	}

	return res;
}
static SurfacePtr blurV(const std::vector<std::string> &args) {
	if (args.size() != 3) {
		Utils::outMsg("ImageManipulator::blurV", "Expected 2 arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const int w = img->w;
	const int h = img->h;
	const int pitch = img->pitch;
	const Uint8 *pixels = (const Uint8 *)img->pixels;


	const int dist = std::min(String::toInt(args[2]), w);
	if (dist < 0) {
		Utils::outMsg("ImageManipulator::blurV", "Blur distance must be >= 0:\n<" + String::join(args, ", ") + ">");
		return img;
	}
	if (!dist) {
		return img;
	}

	const int rotateW = h;
	const int rotateH = w;
	const int rotatePitch = rotateW * 4;

	std::unique_ptr<Uint8[]> rotateTo(new Uint8[size_t(w * h * 4)]);

	std::unique_ptr<Uint8[]> rotateFrom(new Uint8[size_t(w * h * 4)]);
	auto lineToRotate = [&](int y) {
		const Uint8 *src = pixels + y * pitch;
		const Uint8 *srcEnd = src + pitch;

		const int rotateX = y;
		Uint8 *dst = rotateFrom.get() + rotateX * 4;

		while (src != srcEnd) {
			dst[Rshift / 8] = src[Rshift / 8];
			dst[Gshift / 8] = src[Gshift / 8];
			dst[Bshift / 8] = src[Bshift / 8];
			dst[Ashift / 8] = src[Ashift / 8];

			src += 4;
			dst += rotatePitch;
		}
	};
	if (smallImage(w, h)) {
		for (int y = 0; y < rotateH; ++y) {
			lineToRotate(y);
		}
	}else {
#pragma omp parallel for
		for (int y = 0; y < rotateH; ++y) {
			lineToRotate(y);
		}
	}

	auto processLine = [&](int y) {
		const Uint8 *src = rotateFrom.get() + y * rotatePitch;
		Uint8 *dst = rotateTo.get() + y * rotatePitch;

		for (int x = 0; x < rotateW; ++x) {
			Uint32 r, g, b, a;
			r = g = b = a = 0;

			const Uint32 xStart = Uint32(std::max(0, x - dist));
			const Uint32 xEnd = Uint32(std::min(x + dist + 1, rotateH));
			const Uint32 count = xEnd - xStart;

			const Uint8 *pixel = src + xStart * 4;
			const Uint8 *endPixel = src + (xEnd - (count % 8)) * 4;
			while (pixel != endPixel) {
				r += (pixel[Rshift / 8] + pixel[Rshift / 8 + 4]) + (pixel[Rshift / 8 + 8] + pixel[Rshift / 8 + 12]) + (pixel[Rshift / 8 + 16] + pixel[Rshift / 8 + 20]) + (pixel[Rshift / 8 + 24] + pixel[Rshift / 8 + 28]);
				g += (pixel[Gshift / 8] + pixel[Gshift / 8 + 4]) + (pixel[Gshift / 8 + 8] + pixel[Gshift / 8 + 12]) + (pixel[Gshift / 8 + 16] + pixel[Gshift / 8 + 20]) + (pixel[Gshift / 8 + 24] + pixel[Gshift / 8 + 28]);
				b += (pixel[Bshift / 8] + pixel[Bshift / 8 + 4]) + (pixel[Bshift / 8 + 8] + pixel[Bshift / 8 + 12]) + (pixel[Bshift / 8 + 16] + pixel[Bshift / 8 + 20]) + (pixel[Bshift / 8 + 24] + pixel[Bshift / 8 + 28]);
				a += (pixel[Ashift / 8] + pixel[Ashift / 8 + 4]) + (pixel[Ashift / 8 + 8] + pixel[Ashift / 8 + 12]) + (pixel[Ashift / 8 + 16] + pixel[Ashift / 8 + 20]) + (pixel[Ashift / 8 + 24] + pixel[Ashift / 8 + 28]);

				pixel += 32;
			}
			for (size_t i = 0; i < count % 8; ++i) {
				r += pixel[Rshift / 8];
				g += pixel[Gshift / 8];
				b += pixel[Bshift / 8];
				a += pixel[Ashift / 8];

				pixel += 4;
			}

			if (a) {
				dst[Rshift / 8] = Uint8(r / count);
				dst[Gshift / 8] = Uint8(g / count);
				dst[Bshift / 8] = Uint8(b / count);
				dst[Ashift / 8] = Uint8(a / count);
			}else {
				*(Uint32*)dst = 0;
			}
			dst += 4;
		}
	};

	if (smallImage(w, h)) {
		for (int y = 0; y < rotateH; ++y) {
			processLine(y);
		}
	}else {
#pragma omp parallel for
		for (int y = 0; y < rotateH; ++y) {
			processLine(y);
		}
	}

	SurfacePtr res = ImageManipulator::getNewNotClear(w, h);
	Uint8 *resPixels = (Uint8 *)res->pixels;

	auto rotateToLine = [&](int rotateY) {
		const Uint8 *src = rotateTo.get() + rotateY * rotatePitch;
		const Uint8 *srcEnd = src + rotatePitch;

		const int x = rotateY;
		Uint8 *dst = resPixels + x * 4;

		while (src != srcEnd) {
			dst[Rshift / 8] = src[Rshift / 8];
			dst[Gshift / 8] = src[Gshift / 8];
			dst[Bshift / 8] = src[Bshift / 8];
			dst[Ashift / 8] = src[Ashift / 8];

			src += 4;
			dst += pitch;
		}
	};
	if (smallImage(w, h)) {
		for (int rotateY = 0; rotateY < rotateH; ++rotateY) {
			rotateToLine(rotateY);
		}
	}else {
#pragma omp parallel for
		for (int rotateY = 0; rotateY < rotateH; ++rotateY) {
			rotateToLine(rotateY);
		}
	}

	return res;
}


static SurfacePtr motionBlur(const std::vector<std::string> &args) {
	if (args.size() != 5) {
		Utils::outMsg("ImageManipulator::motionBlur", "Expected 4 arguments:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const int w = img->w;
	const int h = img->h;
	const Uint8 *pixels = (const Uint8*)img->pixels;
	const int pitch = img->pitch;

	const std::string &strCX = args[2];
	const std::string &strCY = args[3];
	const int cX = Math::inBounds(int(String::toDouble(strCX) * (strCX.find('.') == size_t(-1) ? 1 : w)), 0, w - 1);
	const int cY = Math::inBounds(int(String::toDouble(strCY) * (strCY.find('.') == size_t(-1) ? 1 : h)), 0, h - 1);

	int dist = String::toInt(args[4]);
	if (dist <= 0 || dist > 255) {
		Utils::outMsg("ImageManipulator::motionBlur", "Blur distance must be from 1 to 255:\n<" + String::join(args, ", ") + ">");
		dist = Math::inBounds(dist, 5, 255);
	}
	if (dist == 1) {
		return img;
	}

	const int maxDX = std::max(cX, w - 1 - cX);
	const int maxDY = std::max(cY, h - 1 - cY);
	const float maxDist = std::sqrt(float(maxDX * maxDX + maxDY * maxDY));
	const float distK = std::sqrt(float(dist) / maxDist) / 2;


	SurfacePtr res = ImageManipulator::getNewNotClear(w, h);
	Uint8 *resPixels = (Uint8*)res->pixels;

	auto processLine = [&](int y) {
		Uint8 *resPixel = resPixels + size_t(y * pitch);

		for (int x = 0; x < w; ++x) {
			Uint16 r, g, b, a;
			r = g = b = a = 0;

			const float dX = float(cX - x) * distK;
			const float dY = float(cY - y) * distK;
			const float d = std::trunc(std::sqrt(dX * dX + dY * dY));

			if (d > 0) {
				const float dx = dX / d;
				const float dy = dY / d;

				float ix = float(x);
				float iy = float(y);
				for (int i = 0; i < int(d); ++i) {
					const Uint8 *pixel = pixels + int(iy) * pitch + int(ix) * 4;
					ix += dx;
					iy += dy;

					r += pixel[Rshift / 8];
					g += pixel[Gshift / 8];
					b += pixel[Bshift / 8];
					a += pixel[Ashift / 8];
				}

				if (a) {
					resPixel[Rshift / 8] = Uint8(r / d);
					resPixel[Gshift / 8] = Uint8(g / d);
					resPixel[Bshift / 8] = Uint8(b / d);
					resPixel[Ashift / 8] = Uint8(a / d);
				}else {
					*(Uint32*)resPixel = 0;
				}
			}else {
				*(Uint32*)resPixel = *(const Uint32*)(pixels + y * pitch + x * 4);
			}
			resPixel += 4;
		}
	};

	if (smallImage(w, h)) {
		for (int y = 0; y < h; ++y) {
			processLine(y);
		}
	}else {
#pragma omp parallel for
		for (int y = 0; y < h; ++y) {
			processLine(y);
		}
	}

	return res;
}


void ImageManipulator::save(const std::string &imageStr, const std::string &path, const std::string &width, const std::string &height) {
	SurfacePtr img = getImage(imageStr, false);
	saveSurface(img, path, width, height, true);
}

template<bool isRGBA>
static Uint32 getPixelValue(const Uint8 *pixel) {
	Uint32 res;
	if constexpr (isRGBA) {
		if (pixel[Ashift / 8]) {
			res = *(Uint32*)pixel;
		}else {
			res = 0;
		}
	}else {
		Uint8 *res8 = (Uint8*)(&res);
		res8[Rshift / 8] = pixel[Rshift / 8];
		res8[Gshift / 8] = pixel[Gshift / 8];
		res8[Bshift / 8] = pixel[Bshift / 8];
		res8[Ashift / 8] = 255;
	}

	return res;
}
template<bool isRGBA>
static void imageToPalette(std::map<Uint32, Uint32> &colors, int w, int h, const Uint8* imgPixels, int imgPitch, Uint8* resPixels, int resPitch) {
	constexpr int pixelSize = isRGBA ? 4 : 3;

	//2 passes to set transparent colors at the begining of the palette (optimization of saving png in palette-mode)
	for (bool transparent : {true, false}) {
		std::map<Uint32, Uint32> localColors;
		ScopeExit se([&]() {
			colors.insert(localColors.begin(), localColors.end());
		});

		for (int y = 0; y < h; ++y) {
			const Uint8 *src = imgPixels + y * imgPitch;
			Uint8 *dst = resPixels + y * resPitch;
			Uint8 *dstEnd = dst + w;
			while (dst < dstEnd) {
				ScopeExit se2([&]() {
					dst += 1;
					src += pixelSize;
				});

				Uint32 color = getPixelValue<isRGBA>(src);
				Uint8 *color8 = (Uint8*)(&color);
				if (transparent) {
					if (color8[Ashift / 8] == 255) continue;
				}else {
					if (color8[Ashift / 8] != 255) continue;
				}

				Uint32 index;
				auto it = localColors.find(color);
				if (it != localColors.end()) {
					index = it->second;
				}else {
					index = Uint32(colors.size() + localColors.size());
					localColors[color] = index;
					if (index == 256) return;
				}
				*dst = Uint8(index);
			}
		}
	}
}

static SurfacePtr optimizeSurfaceFormat(const SurfacePtr &img) {
	if (img->format->BytesPerPixel == 1) return img;//optimized

	bool isRGBA = false;
	Uint32 format = img->format->format;
	if (format == SDL_PIXELFORMAT_RGBA32) {
		isRGBA = true;
	}else {
		if (format != SDL_PIXELFORMAT_RGB24) {
			return img;//unusual format, no optimizations
		}
	}

	const int w = img->w;
	const int h = img->h;
	const int imgPitch = img->pitch;
	const Uint8 *imgPixels = (const Uint8*)img->pixels;


	Uint32 resFormat = SDL_PIXELFORMAT_INDEX8;
	int resPitch = w * int(SDL_BITSPERPIXEL(resFormat)) / 8;
	resPitch = (resPitch + 3) & ~3;//align to 4
	Uint8 *resPixels = (Uint8*)SDL_malloc(size_t(h * resPitch));

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormatFrom(resPixels, w, h, SDL_BITSPERPIXEL(resFormat), resPitch, resFormat),
	               SDL_FreeSurface);
	SDL_SetSurfaceBlendMode(res.get(), SDL_BLENDMODE_NONE);
	res->flags &= Uint32(~SDL_PREALLOC);


	std::map<Uint32, Uint32> colors;
	if (isRGBA) {
		imageToPalette<true>(colors, w, h, imgPixels, imgPitch, resPixels, resPitch);
	}else {
		imageToPalette<false>(colors, w, h, imgPixels, imgPitch, resPixels, resPitch);
	}
	if (colors.size() <= 256) {
		Uint32 colorKey;
		bool hasColorKey = (format == SDL_PIXELFORMAT_RGB24) && SDL_HasColorKey(img.get());
		if (hasColorKey) {
			SDL_GetColorKey(img.get(), &colorKey);
		}

		SDL_Palette *palette = res->format->palette;
		palette->ncolors = int(colors.size());
		for (const auto &pair : colors) {
			Uint32 color = pair.first;
			Uint32 index = pair.second;
			if (hasColorKey && color == colorKey) {
				color = 0;
			}

			Uint8 *color8 = (Uint8*)(&color);
			SDL_Color &sdlColor = palette->colors[index];
			sdlColor.r = color8[Rshift / 8];
			sdlColor.g = color8[Gshift / 8];
			sdlColor.b = color8[Bshift / 8];
			sdlColor.a = color8[Ashift / 8];
		}

		return res;
	}

	if (!isRGBA) return img;//no optimizations

	//check rgba -> rgb:
	for (int y = 0; y < h; ++y) {
		const Uint8 *src = imgPixels + y * imgPitch;
		const Uint8 *srcEnd = src + imgPitch;
		while (src < srcEnd) {
			if (src[Ashift / 8] != 255) return img;//no optimizations
			src += 4;
		}
	}

	res = ImageManipulator::getNewNotClear(w, h, SDL_PIXELFORMAT_RGB24);
	SDL_BlitSurface(img.get(), nullptr, res.get(), nullptr);

	return res;
}

//67 bytes
static Uint8 png1x1Black[] = {
    137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 55, 110, 249, 36, 0, 0, 0, 10, 73, 68, 65, 84, 8, 215, 99, 96, 0, 0, 0, 2, 0, 1, 226, 33, 188, 51, 0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130
};
void ImageManipulator::saveSurface(const SurfacePtr &img, const std::string &path, const std::string &width, const std::string &height, bool now) {
	if (!img) return;

	if (!now) {
		std::ofstream os(path, std::ios_base::binary);
		os.write(reinterpret_cast<const char*>(png1x1Black), sizeof(png1x1Black));
		os.close();

		std::lock_guard g(preloadMutex);
		toSaveImages.push_back({img, path, width, height});
		return;
	}

	const std::string widthStr = Algo::clear(width);
	const std::string heightStr = Algo::clear(height);

	const int w = std::max(1, widthStr  == "None" ? img->w : String::toInt(widthStr));
	const int h = std::max(1, heightStr == "None" ? img->h : String::toInt(heightStr));

	SurfacePtr resizedImg;
	if (w != img->w || h != img->h) {
		resizedImg = getNewNotClear(w, h);
		SurfacePtr imgNotPaletted = img->format->BitsPerPixel < 24 ? ImageCaches::convertToRGBA32(img) : img;
		SDL_BlitScaled(imgNotPaletted.get(), nullptr, resizedImg.get(), nullptr);
	}else {
		resizedImg = img;
	}
	SurfacePtr imgToSave = optimizeSurfaceFormat(resizedImg);

	//allow unfinished image only with tmp name

	std::string parentDir = FileSystem::getParentDirectory(path);
	std::string filename = FileSystem::getFileName(path);

	std::string tmpPath;
	size_t i = 0;
	while (true) {
		tmpPath = parentDir + "_" + std::to_string(i++) + filename;
		if (!FileSystem::exists(tmpPath)) break;
	}

	IMG_SavePNG(imgToSave.get(), tmpPath.c_str());
	if (FileSystem::exists(path)) {
		FileSystem::remove(path);
	}
	FileSystem::rename(tmpPath, path);
}
