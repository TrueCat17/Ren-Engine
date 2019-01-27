#include "image_manipulator.h"

#include <thread>
#include <mutex>
#include <algorithm>

#include <SDL2/SDL_image.h>

#include "gv.h"
#include "renderer.h"

#include "utils/algo.h"
#include "utils/image_caches.h"
#include "utils/math.h"
#include "utils/utils.h"


static std::map<String, std::function<SurfacePtr(const std::vector<String>&)>> functions;

static std::deque<String> toLoadImages;
static std::mutex toLoadMutex;


static inline bool smallImage(int w, int h) {
	return h < 32 && w * h < 64 * 32;
}


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

static SurfacePtr blurH(const std::vector<String> &args);
static SurfacePtr blurV(const std::vector<String> &args);
static SurfacePtr motionBlur(const std::vector<String> &args);


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


SurfacePtr ImageManipulator::getNewNotClear(int w, int h) {
	if (w <= 0 || h <= 0) return nullptr;

	int pitch = w * 4;
	void *pixels = SDL_malloc(size_t(h * pitch));

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormatFrom(pixels, w, h, 32, pitch, SDL_PIXELFORMAT_RGBA32),
				   SDL_FreeSurface);
	SDL_SetSurfaceBlendMode(res.get(), SDL_BLENDMODE_NONE);

	res->flags &= Uint32(~SDL_PREALLOC);
	return res;
}


void ImageManipulator::loadImage(const std::string &desc) {
	SurfacePtr image = ImageCaches::getThereIsSurfaceOrNull(desc);
	if (image) return;

	std::lock_guard<std::mutex> g(toLoadMutex);

	if (!Algo::in(desc, toLoadImages)) {
		toLoadImages.push_back(desc);
	}
}
void preloadThread() {
	while (!GV::exit) {
		if (toLoadImages.empty()) {
			Utils::sleep(5, false);
		}else {
			String desc;
			{
				std::lock_guard<std::mutex> g(toLoadMutex);
				desc = toLoadImages.front();
				toLoadImages.pop_front();
			}
			ImageManipulator::getImage(desc);
		}
	}
}


static std::mutex vecMutex;
static std::vector<String> processingImages;

SurfacePtr ImageManipulator::getImage(String desc) {
	desc = Algo::clear(desc);

	SurfacePtr res = ImageCaches::getThereIsSurfaceOrNull(desc);
	if (res) return res;


	bool in = true;
	while (in) {
		{
			std::lock_guard<std::mutex> g(vecMutex);
			in = Algo::in(desc, processingImages);
			if (!in) {
				res = ImageCaches::getThereIsSurfaceOrNull(desc);
				if (res) return res;

				processingImages.push_back(desc);
			}
		}
		if (in) {
			Utils::sleep(10);
		}
	}


	struct Deleter {
		const String &desc;

		Deleter(const String& desc): desc(desc) {}

		~Deleter() {
			std::lock_guard<std::mutex> g(vecMutex);

			for (size_t i = 0; i < processingImages.size(); ++i) {
				if (processingImages[i] == desc) {
					processingImages.erase(processingImages.begin() + int(i));
					return;
				}
			}
		}
	} deleter(desc);


	const std::vector<String> args = Algo::getArgs(desc, '|');

	if (args.empty()) {
		Utils::outMsg("ImageManipulator::getImage", "Список аргументов пуст");
		return nullptr;
	}
	if (args.size() == 1) {
		const String imagePath = Algo::clear(args[0]);

		res = ImageCaches::getSurface(imagePath);
		return res;
	}


	const String &command = args[0];

	auto it = functions.find(command);
	if (it == functions.end()) {
		Utils::outMsg("ImageManipulator::getImage", "Неизвестная команда <" + command + ">");
	}else {
		auto func = it->second;
		res = func(args);
	}

	ImageCaches::setSurface(desc, res);
	return res;
}


static SurfacePtr scale(const std::vector<String> &args) {
	if (args.size() != 4) {
		Utils::outMsg("ImageManipulator::scale", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const int w = Algo::clear(args[2]).toInt();
	const int h = Algo::clear(args[3]).toInt();
	if (w <= 0 || h <= 0) {
		Utils::outMsg("ImageManipulator::scale", "Некорректные размеры:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	if (w == img->w && h == img->h) {
		return img;
	}

	const SDL_Rect imgRect = { 0, 0, img->w, img->h };
	SDL_Rect resRect = { 0, 0, w, h };

	SurfacePtr res = ImageManipulator::getNewNotClear(w, h);
	SDL_BlitScaled(img.get(), &imgRect, res.get(), &resRect);

	return res;
}

static SurfacePtr factorScale(const std::vector<String> &args) {
	if (args.size() != 3) {
		Utils::outMsg("ImageManipulator::factorScale", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const double k = Algo::clear(args[2]).toDouble();
	if (k <= 0) {
		Utils::outMsg("ImageManipulator::factorScale", "Коэффициент масштабирования должен быть > 0:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	if (k == 1) {
		return img;
	}

	const SDL_Rect imgRect = { 0, 0, img->w, img->h };
	SDL_Rect resRect = { 0, 0, std::max(int(img->w * k), 1), std::max(int(img->h * k), 1) };

	SurfacePtr res = ImageManipulator::getNewNotClear(resRect.w, resRect.h);
	SDL_BlitScaled(img.get(), &imgRect, res.get(), &resRect);

	return res;
}
static SurfacePtr rendererScale(const std::vector<String> &args) {
	if (args.size() != 4) {
		Utils::outMsg("ImageManipulator::rendererScale", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const int w = Algo::clear(args[2]).toInt();
	const int h = Algo::clear(args[3]).toInt();
	if (w <= 0 || h <= 0) {
		Utils::outMsg("ImageManipulator::rendererScale", "Некорректные размеры:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	if (w == img->w && h == img->h) {
		return img;
	}

	if (w > Renderer::maxTextureWidth || h > Renderer::maxTextureHeight) {
		return scale(args);
	}

	return Renderer::getScaled(img, w, h);
}

static SurfacePtr crop(const std::vector<String> &args) {
	if (args.size() != 3) {
		Utils::outMsg("ImageManipulator::crop", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const String rectStr = Algo::clear(args[2]);
	const std::vector<String> rectVec = rectStr.split(' ');

	const int x = rectVec[0].toInt();
	const int y = rectVec[1].toInt();
	const int w = rectVec[2].toInt();
	const int h = rectVec[3].toInt();

	if (w <= 0 || h <= 0) {
		Utils::outMsg("ImageManipulator::crop", "Некорректные размеры:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	if (x + w > img->w || y + h > img->h) {
		Utils::outMsg("ImageManipulator::crop", "Область вырезки выходит за пределы изображения:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	if (!x && !y && w == img->w && h == img->h) {
		return img;
	}

	SDL_Rect imgRect = {x, y, w, h};
	SDL_Rect resRect = {0, 0, w, h};

	SurfacePtr res = ImageManipulator::getNewNotClear(w, h);
	SDL_BlitScaled(img.get(), &imgRect, res.get(), &resRect);

	return res;
}

static SurfacePtr composite(const std::vector<String> &args) {
	if (args.size() % 2) {
		Utils::outMsg("ImageManipulator::composite", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	const String sizeStr = Algo::clear(args[1]);
	const std::vector<String> sizeVec = sizeStr.split(' ');

	const int resW = sizeVec.size() == 2 ? sizeVec[0].toInt() : 0;
	const int resH = sizeVec.size() == 2 ? sizeVec[1].toInt() : 0;
	if (sizeVec.size() != 2 || resW <= 0 || resH <= 0) {
		Utils::outMsg("ImageManipulator::composite", "Некорректные размеры <" + sizeStr + ">");
		return nullptr;
	}

	if (args.size() == 4) {
		const String posStr = Algo::clear(args[2]);
		const String imagePath = Algo::clear(args[3]);
		SurfacePtr image = ImageManipulator::getImage(imagePath);

		if (!image) {
			Utils::outMsg("ImageManipulator::composite", "Не удалось получить изображение <" + imagePath + ">");
		}else {
			const String imSizeStr = String(image->w) + ' ' + image->h;
			if (posStr == "0 0" && imSizeStr == sizeStr) {
				return image;
			}
		}
	}


	std::map<String, size_t> images;
	for (size_t i = 2; i < args.size(); i += 2) {
		const String image = Algo::clear(args[i + 1]);

		if (!images.count(image)) {
			images[image] = i;
		}
	}
	if (images.size()) {
		typedef std::pair<String, size_t> P;
		std::vector<P> imagesPairs(images.begin(), images.end());

		std::sort(imagesPairs.begin(), imagesPairs.end(),
				  [](const P &a, const P &b) { return a.second < b.second; });

		std::lock_guard<std::mutex> g(toLoadMutex);

		for (const P &p : imagesPairs) {
			const String &desc = p.first;
			if (!Algo::in(desc, toLoadImages) && !ImageCaches::getThereIsSurfaceOrNull(desc)) {
				toLoadImages.push_back(desc);
			}
		}

	}

	SurfacePtr res = ImageManipulator::getNewNotClear(resW, resH);
	Uint8 *resPixels = (Uint8*)res->pixels;
	const int resPitch = res->pitch;


	const String firstImgStr = args.size() > 2 ? Algo::clear(args[3]) : "";
	SurfacePtr firstImg = firstImgStr ? ImageManipulator::getImage(firstImgStr) : nullptr;
	if (!firstImg) {
		if (args.size() > 2) {
			Utils::outMsg("ImageManipulator::composite", "Не удалось получить изображение <" + firstImgStr + ">");
		}
		::memset(resPixels, 0, size_t(resH * resPitch));
	}else {
		const String firstPosStr = Algo::clear(args[2]);
		const std::vector<String> firstPosVec = firstPosStr.split(' ');

		const int xOn = firstPosVec[0].toInt();
		const int yOn = firstPosVec[1].toInt();
		const int w = firstImg->w;
		const int h = firstImg->h;

		if (yOn > 0) {
			::memset(resPixels, 0, size_t(yOn * resPitch));//up
		}

		const int left = xOn * 4;
		const int center = firstImg->pitch;
		const int right = res->pitch - (left + center);

		//image - center
		SDL_Rect to = {xOn, yOn, w, h};
		SDL_BlitScaled(firstImg.get(), nullptr, res.get(), &to);

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
		const String posStr = Algo::clear(args[i]);
		const std::vector<String> posVec = posStr.split(' ');

		const String imgStr = Algo::clear(args[i + 1]);
		SurfacePtr img = ImageManipulator::getImage(imgStr);
		if (!img) {
			Utils::outMsg("ImageManipulator::composite", "Не удалось получить изображение <" + imgStr + ">");
			continue;
		}

		int xOn = posVec[0].toInt();
		int yOn = posVec[1].toInt();
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
			for (; dst < dstEnd; src += 4, dst += 4) {
				const Uint16 srcA = src[Ashift / 8];
				if (srcA == 255) {
					*(Uint32*)dst = *(Uint32*)src;
				}else if (srcA) {
					const Uint16 dstA = dst[Ashift / 8];

					if (!dstA) {
						*(Uint32*)dst = *(Uint32*)src;
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

						dst[Rshift / 8] = (srcR * sA + dstR * dA + 128) / 256;
						dst[Gshift / 8] = (srcG * sA + dstG * dA + 128) / 256;
						dst[Bshift / 8] = (srcB * sA + dstB * dA + 128) / 256;
						dst[Ashift / 8] = Uint8(outA);
					}
				}
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

static SurfacePtr flip(const std::vector<String> &args) {
	if (args.size() != 4) {
		Utils::outMsg("ImageManipulator::flip", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
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

static SurfacePtr matrixColor(const std::vector<String> &args) {
	if (args.size() != 3) {
		Utils::outMsg("ImageManipulator::matrixColor", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const String matrixStr = Algo::clear(args[2]);
	const std::vector<String> matrixVec = matrixStr.split(' ');
	if (matrixVec.size() != 25) {
		Utils::outMsg("ImageManipulator::matrixColor",
					  "Размер матрицы должен быть равен 25,\n"
					  "Матрица: <" + matrixStr + ">");
		return img;
	}

	std::vector<double> matrix;
	const int matrixSizeToUse = 20;
	matrix.resize(matrixSizeToUse);
	for (size_t i = 0; i < matrixSizeToUse; ++i) {//Используется 20 из 25 значений
		matrix[i] = matrixVec[i].toDouble();
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
	if (!matrix[15] && !matrix[16] && !matrix[17] && !matrix[18] && !matrix[19]) {
		::memset(resPixels, 0, size_t(w * h * 4));
		return res;
	}

	const double extraR = matrix[ 4] * 255;
	const double extraG = matrix[ 9] * 255;
	const double extraB = matrix[14] * 255;
	const double extraA = matrix[19] * 255;

	auto processLine = [&](int y) {
		const Uint8 *src = imgPixels + y * imgPitch;
		Uint8 *dst = resPixels + y * imgPitch;

		const Uint8 *dstEnd = dst + imgPitch;
		while (dst != dstEnd) {
			if (*(const Uint32*)(src) || extraA) {
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

static SurfacePtr reColor(const std::vector<String> &args) {
	if (args.size() != 3) {
		Utils::outMsg("ImageManipulator::reColor", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const String colorsStr = Algo::clear(args[2]);
	const std::vector<String> colorsVec = colorsStr.split(' ');
	if (colorsVec.size() != 4) {
		Utils::outMsg("ImageManipulator::reColor", "Неверное количество цветов:\n<" + colorsStr + ">");
		return img;
	}

	const Uint16 r = Uint16(colorsVec[0].toDouble());
	const Uint16 g = Uint16(colorsVec[1].toDouble());
	const Uint16 b = Uint16(colorsVec[2].toDouble());
	const Uint16 a = Uint16(colorsVec[3].toDouble());

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
				dst[Rshift / 8] = src[Rshift / 8] * r / 256;
				dst[Gshift / 8] = src[Gshift / 8] * g / 256;
				dst[Bshift / 8] = src[Bshift / 8] * b / 256;
				dst[Ashift / 8] = src[Ashift / 8] * a / 256;
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

static SurfacePtr rotozoom(const std::vector<String> &args) {
	if (args.size() != 4) {
		Utils::outMsg("ImageManipulator::rotozoom", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const int angle = int(-Algo::clear(args[2]).toDouble());
	double zoom = Algo::clear(args[3]).toDouble();
	if (!zoom) {
		Utils::outMsg("ImageManipulator::rotozoom", "zoom не должен быть равен 0");
		zoom = 1;
	}
	if (zoom == 1 && angle == 0) {
		return img;
	}

	SDL_RendererFlip flip = SDL_FLIP_NONE;
	if (zoom < 0) {
		zoom = -zoom;
		flip = SDL_RendererFlip(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
	}

	const int w = std::max(int(img->w * zoom), 1);
	const int h = std::max(int(img->h * zoom), 1);

	auto abs = [](double d) { return d < 0 ? -d : d; };

	const double absSin = abs(Math::getSin(angle));
	const double absCos = abs(Math::getCos(angle));
	const int resW = int(w * absCos + h * absSin);
	const int resH = int(w * absSin + h * absCos);

	const SDL_Rect srcRect = {0, 0, img->w, img->h};
	const SDL_Rect dstRect = {(resW - w) / 2, (resH - h) / 2, w, h};

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(0, resW, resH, 32, SDL_PIXELFORMAT_RGBA32),
				   SDL_FreeSurface);

	std::shared_ptr<SDL_Renderer> renderer(SDL_CreateSoftwareRenderer(res.get()), SDL_DestroyRenderer);

	if (!renderer) {
		Utils::outMsg("ImageManipulator::rotozoom, SDL_CreateSoftwareRenderer", SDL_GetError());
		return nullptr;
	}

	TexturePtr texture(SDL_CreateTextureFromSurface(renderer.get(), img.get()), SDL_DestroyTexture);
	if (!texture) {
		Utils::outMsg("ImageManipulator::rotozoom, SDL_CreateTextureFromSurface", SDL_GetError());
		return nullptr;
	}

	if (SDL_RenderCopyEx(renderer.get(), texture.get(), &srcRect, &dstRect, angle, nullptr, flip)) {
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
static void maskChooseCmp(const int value, const String &cmp,
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
							const int value, const String &cmp,
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

static SurfacePtr mask(const std::vector<String> &args) {
	if (args.size() != 8) {
		Utils::outMsg("ImageManipulator::mask", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	SurfacePtr mask = ImageManipulator::getImage(args[2]);
	if (!mask) return nullptr;

	if (img->w != mask->w || img->h != mask->h) {
		const String imgSize = String(img->w) + "x" + String(img->h);
		const String maskSize = String(mask->w) + "x" + String(mask->h);
		Utils::outMsg("ImageManipulator::mask", "Размеры маски " + imgSize + " не соответствуют размерам изображения " + maskSize + ":\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	const String channelStr = Algo::clear(args[3]);
	if (channelStr != "r" && channelStr != "g" && channelStr != "b" && channelStr != "a") {
		Utils::outMsg("ImageManipulator::mask", "Некорректный канал <" + channelStr + ">, ожидалось r, g, b или a:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	const char channel = channelStr[0];

	const String valueStr = Algo::clear(args[4]);
	if (!valueStr.isNumber()) {
		Utils::outMsg("ImageManipulator::mask", "Значение <" + valueStr + "> должно являться числом:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	const int value = int(valueStr.toDouble());

	const String cmp = Algo::clear(args[5]);
	static const std::vector<String> cmps = {"l", "g", "e", "ne", "le", "ge"};
	if (!Algo::in(cmp, cmps)) {
		Utils::outMsg("ImageManipulator::mask", "Некорректная функция сравнения <" + cmp + ">, ожидалось l(<), g(>), e(==), ne(!=), le(<=), ge(>=):\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	const String alphaStr = Algo::clear(args[6]);
	if (alphaStr != "r" && alphaStr != "g" && alphaStr != "b" && alphaStr != "a") {
		Utils::outMsg("ImageManipulator::mask", "Некорректный канал <" + alphaStr + ">, ожидалось r, g, b или a:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	const char alphaChannel = alphaStr[0];

	const String alphaImage = Algo::clear(args[7]);
	if (alphaImage != "1" && alphaImage != "2") {
		Utils::outMsg("ImageManipulator::mask", "Некорректный номер изображения <" + alphaStr + ">, ожидалось 1 (основа) или 2 (маска):\n<" + String::join(args, ", ") + ">");
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


static SurfacePtr blurH(const std::vector<String> &args) {
	if (args.size() != 3) {
		Utils::outMsg("ImageManipulator::blurH", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const int w = img->w;
	const int h = img->h;
	const int pitch = img->pitch;
	const Uint8 *pixels = (const Uint8 *)img->pixels;


	const int dist = std::min(args[2].toInt(), w);
	if (dist < 0) {
		Utils::outMsg("ImageManipulator::blurH", "Расстояние размытия меньше 0:\n<" + String::join(args, ", ") + ">");
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
static SurfacePtr blurV(const std::vector<String> &args) {
	if (args.size() != 3) {
		Utils::outMsg("ImageManipulator::blurV", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const int w = img->w;
	const int h = img->h;
	const int pitch = img->pitch;
	const Uint8 *pixels = (const Uint8 *)img->pixels;


	const int dist = std::min(args[2].toInt(), w);
	if (dist < 0) {
		Utils::outMsg("ImageManipulator::blurV", "Расстояние размытия меньше 0:\n<" + String::join(args, ", ") + ">");
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


static SurfacePtr motionBlur(const std::vector<String> &args) {
	if (args.size() != 5) {
		Utils::outMsg("ImageManipulator::motionBlur", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = ImageManipulator::getImage(args[1]);
	if (!img) return nullptr;

	const int w = img->w;
	const int h = img->h;
	const Uint8 *pixels = (const Uint8*)img->pixels;
	const int pitch = img->pitch;

	const String &strCX = args[2];
	const String &strCY = args[3];
	const int cX = Math::inBounds(int(strCX.toDouble() * (strCX.find('.') == size_t(-1) ? 1 : w)), 0, w - 1);
	const int cY = Math::inBounds(int(strCY.toDouble() * (strCY.find('.') == size_t(-1) ? 1 : h)), 0, h - 1);

	int dist = args[4].toInt();
	if (dist <= 0 || dist > 255) {
		Utils::outMsg("ImageManipulator::motionBlur", "Расстояние размывания должно быть от 1 до 255:\n<" + String::join(args, ", ") + ">");
		dist = Math::inBounds(dist, 5, 255);
	}
	if (dist == 1) {
		return img;
	}

	const int maxDX = std::max(cX, w - 1 - cX);
	const int maxDY = std::max(cY, h - 1 - cY);
	const float maxDist = std::sqrt(float(maxDX * maxDX + maxDY * maxDY));
	const float distK = std::sqrt(dist / maxDist) / 2;


	SurfacePtr res = ImageManipulator::getNewNotClear(w, h);
	Uint8 *resPixels = (Uint8*)res->pixels;

	auto processLine = [&](int y) {
		Uint8 *resPixel = resPixels + size_t(y * pitch);

		for (int x = 0; x < w; ++x) {
			Uint16 r, g, b, a;
			r = g = b = a = 0;

			const float dX = (cX - x) * distK;
			const float dY = (cY - y) * distK;
			const int d = int(std::sqrt(dX * dX + dY * dY));

			if (d) {
				const float dx = dX / d;
				const float dy = dY / d;

				float ix = x;
				float iy = y;
				for (int i = 0; i < d; ++i) {
					const Uint8 *pixel = pixels + size_t(iy * pitch + ix * 4);
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
				*(Uint32*)resPixel = *(Uint32*)(pixels + y * pitch + x * 4);
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
	SurfacePtr img = getImage(imageStr);
	if (!img) return;

	const String widthStr = Algo::clear(width);
	const String heightStr = Algo::clear(height);

	const int w = Math::inBounds(widthStr == "None"  ? img->w : widthStr.toInt() , 1, 2400);
	const int h = Math::inBounds(heightStr == "None" ? img->h : heightStr.toInt(), 1, 1350);

	const SDL_Rect from = {0, 0, img->w, img->h};
	SDL_Rect to = {0, 0, w, h};

	SurfacePtr saveImg = getNewNotClear(w, h);
	SDL_BlitScaled(img.get(), &from, saveImg.get(), &to);

	IMG_SavePNG(saveImg.get(), path.c_str());
}
