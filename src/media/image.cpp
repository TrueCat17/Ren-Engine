#include "image.h"

#include <thread>
#include <mutex>
#include <algorithm>

#include <SDL2/SDL_image.h>

#include "gv.h"


std::map<String, std::function<SurfacePtr(const std::vector<String>&)>> Image::functions;

std::deque<String> Image::toLoadImages;
std::mutex Image::toLoadMutex;

size_t Image::countThreads;
std::deque<std::pair<size_t, std::function<void(size_t)>>> Image::partsToProcessing;
std::mutex Image::processingMutex;


#if SDL_BYTEORDER == SDL_BIG_ENDIAN
static const Uint32 Rmask = 0xFF000000;
static const Uint32 Gmask = 0x00FF0000;
static const Uint32 Bmask = 0x0000FF00;
static const Uint32 Amask = 0x000000FF;

static const Uint8 Rshift = 24;
static const Uint8 Gshift = 16;
static const Uint8 Bshift = 8;
static const Uint8 Ashift = 0;
#else
static const Uint32 Rmask = 0x000000FF;
static const Uint32 Gmask = 0x0000FF00;
static const Uint32 Bmask = 0x00FF0000;
static const Uint32 Amask = 0xFF000000;

static const Uint8 Rshift = 0;
static const Uint8 Gshift = 8;
static const Uint8 Bshift = 16;
static const Uint8 Ashift = 24;
#endif


static inline Uint8 getRed  (Uint32 pixel) { return (pixel & Rmask) >> Rshift; }
static inline Uint8 getGreen(Uint32 pixel) { return (pixel & Gmask) >> Gshift; }
static inline Uint8 getBlue (Uint32 pixel) { return (pixel & Bmask) >> Bshift; }
static inline Uint8 getAlpha(Uint32 pixel) { return (pixel & Amask) >> Ashift; }

static inline Uint32 getPixelColor(const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a) {
	return  (r << Rshift) | (g << Gshift) | (b << Bshift) | (a << Ashift);
}


void Image::init() {
	functions["Scale"] = scale;
	functions["FactorScale"] = factorScale;
	functions["Crop"] = crop;
	functions["Composite"] = composite;
	functions["Flip"] = flip;
	functions["MatrixColor"] = matrixColor;
	functions["ReColor"] = reColor;
	functions["Rotozoom"] = rotozoom;
	functions["Mask"] = mask;

	std::thread(preloadThread).detach();

	countThreads = Utils::inBounds(size_t(SDL_GetCPUCount()) * 2, 2, 16);
	for (size_t i = 0; i < countThreads; ++i) {
		std::thread(processingThread).detach();
	}
}

void Image::processingThread() {
	bool empty;
	std::pair<size_t, std::function<void(size_t)>> p;

	while (!GV::exit) {
		{
			std::lock_guard<std::mutex> g(processingMutex);
			empty = partsToProcessing.empty();
			if (!empty) {
				p = partsToProcessing.front();
				partsToProcessing.pop_front();
			}
		}

		if (empty) {
			Utils::sleep(1, false);
		}else {
			p.second(p.first);
		}
	}
}

void Image::loadImage(const std::string &desc) {
	SurfacePtr image = Utils::getThereIsSurfaceOrNull(desc);
	if (image) return;

	std::lock_guard<std::mutex> g(toLoadMutex);

	if (!Utils::in(desc, toLoadImages)) {
		toLoadImages.push_back(desc);
	}
}
void Image::preloadThread() {
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
			getImage(desc);
		}
	}
}


SurfacePtr Image::getImage(String desc) {
	desc = Utils::clear(desc);

	SurfacePtr res = Utils::getThereIsSurfaceOrNull(desc);
	if (res) return res;


	static std::mutex vecMutex;
	static std::vector<String> processingImages;

	bool in = true;
	while (in) {
		{
			std::lock_guard<std::mutex> g(vecMutex);
			in = Utils::in(desc, processingImages);
			if (!in) {
				res = Utils::getThereIsSurfaceOrNull(desc);
				if (res) {
					return res;
				}else {
					processingImages.push_back(desc);
				}
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
					processingImages.erase(processingImages.begin() + i, processingImages.begin() + i + 1);
					return;
				}
			}
		}
	} deleter(desc);


	const std::vector<String> args = Utils::getArgs(desc, '|');

	if (args.empty()) {
		Utils::outMsg("Image::getImage", "Список аргументов пуст");
		return nullptr;
	}
	if (args.size() == 1) {
		const String imagePath = Utils::clear(args[0]);

		res = Utils::getSurface(imagePath);
		return res;
	}


	const String &command = args[0];

	auto it = functions.find(command);
	if (it == functions.end()) {
		Utils::outMsg("Image::getImage", "Неизвестная команда <" + command + ">");
	}else {
		auto func = it->second;
		res = func(args);
	}

	Utils::setSurface(desc, res);
	return res;
}


SurfacePtr Image::scale(const std::vector<String> &args) {
	if (args.size() != 4) {
		Utils::outMsg("Image::scale", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = getImage(args[1]);
	if (!img) return nullptr;

	const SDL_Rect imgRect = { 0, 0, img->w, img->h };
	SDL_Rect resRect = { 0, 0, Utils::clear(args[2]).toInt(), Utils::clear(args[3]).toInt() };
	if (resRect.w <= 0 || resRect.h <= 0) {
		Utils::outMsg("Image::scale", "Некорректные размеры:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	if (imgRect.w == resRect.w && imgRect.h == resRect.h) {
		return img;
	}

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(img->flags, resRect.w, resRect.h, 32, img->format->format),
				   SDL_FreeSurface);
	SDL_BlitScaled(img.get(), &imgRect, res.get(), &resRect);

	return res;
}

SurfacePtr Image::factorScale(const std::vector<String> &args) {
	if (args.size() != 3) {
		Utils::outMsg("Image::factorScale", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = getImage(args[1]);
	if (!img) return nullptr;

	const double k = Utils::clear(args[2]).toDouble();
	if (k == 1) {
		return img;
	}

	const SDL_Rect imgRect = { 0, 0, img->w, img->h };
	SDL_Rect resRect = { 0, 0, std::max(int(img->w * k), 1), std::max(int(img->h * k), 1) };

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(img->flags, resRect.w, resRect.h, 32, img->format->format),
				   SDL_FreeSurface);
	SDL_BlitScaled(img.get(), &imgRect, res.get(), &resRect);

	return res;
}

SurfacePtr Image::crop(const std::vector<String> &args) {
	if (args.size() != 3) {
		Utils::outMsg("Image::crop", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	SurfacePtr img = getImage(args[1]);
	if (!img) return nullptr;

	const String rectStr = Utils::clear(args[2]);
	const std::vector<String> rectVec = rectStr.split(' ');

	const SDL_Rect imgRect = { rectVec[0].toInt(), rectVec[1].toInt(), rectVec[2].toInt(), rectVec[3].toInt() };
	SDL_Rect resRect = { 0, 0, imgRect.w, imgRect.h };
	if (resRect.w <= 0 || resRect.h <= 0) {
		Utils::outMsg("Image::crop", "Некорректные размеры:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	if (!imgRect.x && !imgRect.y && imgRect.w == img->w && imgRect.h == img->h) {
		return img;
	}

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(img->flags, resRect.w, resRect.h, 32, img->format->format),
				   SDL_FreeSurface);
	SDL_BlitScaled(img.get(), &imgRect, res.get(), &resRect);

	return res;
}

SurfacePtr Image::composite(const std::vector<String> &args) {
	if (args.size() % 2) {
		Utils::outMsg("Image::composite", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	const String sizeStr = Utils::clear(args[1]);
	const std::vector<String> sizeVec = sizeStr.split(' ');
	if (sizeVec.size() != 2) {
		Utils::outMsg("Image::composite", "Некорректные размеры <" + sizeStr + ">");
		return nullptr;
	}

	std::map<String, size_t> images;
	for (size_t i = 2; i < args.size(); i += 2) {
		const String image = Utils::clear(args[i + 1]);

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
			if (!Utils::in(desc, toLoadImages) && !Utils::getThereIsSurfaceOrNull(desc)) {
				toLoadImages.push_back(desc);
			}
		}

	}

	const SDL_Rect resRect = { 0, 0, sizeVec[0].toInt(), sizeVec[1].toInt() };
	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(0, resRect.w, resRect.h, 32, SDL_PIXELFORMAT_RGBA32),
				   SDL_FreeSurface);

	for (size_t i = 2; i < args.size(); i += 2) {
		const String posStr = Utils::clear(args[i]);
		const std::vector<String> posVec = posStr.split(' ');

		const String imgStr = Utils::clear(args[i + 1]);
		SurfacePtr img = getImage(imgStr);
		if (!img) {
			Utils::outMsg("Image::composite", "Не удалось получить изображение <" + imgStr + ">");
			continue;
		}

		const SDL_Rect fromRect = { 0, 0, img->w, img->h };
		SDL_Rect toRect = { posVec[0].toInt(), posVec[1].toInt(), img->w, img->h };
		if (toRect.w <= 0 || toRect.h <= 0) {
			Utils::outMsg("Image::composite", "Некорректные размеры:\n<" + imgStr + ">");
			continue;
		}

		SDL_BlitScaled(img.get(), &fromRect, res.get(), &toRect);
	}
	return res;
}

SurfacePtr Image::flip(const std::vector<String> &args) {
	if (args.size() != 4) {
		Utils::outMsg("Image::flip", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = getImage(args[1]);
	if (!img) return nullptr;

	const bool horizontal = Utils::clear(args[2]) == "True";
	const bool vertical = Utils::clear(args[3]) == "True";
	if (!horizontal && !vertical) {
		return img;
	}

	const Uint8 *imgPixels = (const Uint8*)img->pixels;

	const int w = img->w;
	const int h = img->h;
	const int imgPitch = img->pitch;
	const int imgBpp = 4;

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(img->flags, w, h, 32, img->format->format),
				   SDL_FreeSurface);

	Uint8 *resPixels = (Uint8*)res->pixels;

	const size_t countThreads = std::min(Image::countThreads * partsOnThreads, size_t(h));
	size_t countFinished = 0;
	std::mutex guard;


	auto f = [&](const size_t num) {
		const int yStart = h * (double(num) / countThreads);
		const int yEnd = h * (double(num + 1) / countThreads);

		for (int y = yStart; y < yEnd; ++y) {
			for (int x = 0; x < w; ++x) {
				const int _y = vertical ? h - y - 1 : y;
				const int _x = horizontal ? w - 1 : 0;

				const Uint32 imgPixel = *(const Uint32*)(imgPixels + _y * imgPitch + _x * imgBpp);
				*(Uint32*)(resPixels + y * imgPitch + x * imgBpp) = imgPixel;
			}
		}

		std::lock_guard<std::mutex> g(guard);
		++countFinished;
	};

	if (countThreads != 1) {
		{
			std::lock_guard<std::mutex> g(processingMutex);
			for (size_t i = 0; i < countThreads; ++i) {
				partsToProcessing.push_back(std::make_pair(i, f));
			}
		}
		while (countFinished != countThreads) {
			Utils::sleep(1);
		}
	}else {
		f(0);
	}

	return res;
}

SurfacePtr Image::matrixColor(const std::vector<String> &args) {
	if (args.size() != 3) {
		Utils::outMsg("Image::matrixColor", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = getImage(args[1]);
	if (!img) return nullptr;

	const String matrixStr = Utils::clear(args[2]);
	const std::vector<String> matrixVec = matrixStr.split(' ');
	if (matrixVec.size() != 25) {
		Utils::outMsg("Image::matrixColor",
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

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(img->flags, w, h, 32, img->format->format),
				   SDL_FreeSurface);
	if (!matrix[15] && !matrix[16] && !matrix[17] && !matrix[18] && !matrix[19]) return res;

	Uint8 *resPixels = (Uint8*)res->pixels;


	const size_t countThreads = std::min(Image::countThreads * partsOnThreads, size_t(h));
	size_t countFinished = 0;
	std::mutex guard;

	auto f = [&](const size_t num) {
		const double extraR = matrix[ 4] * 255;
		const double extraG = matrix[ 9] * 255;
		const double extraB = matrix[14] * 255;
		const double extraA = matrix[19] * 255;

		const int yStart = h * (double(num) / countThreads);
		const int yEnd = h * (double(num + 1) / countThreads);

		const Uint8 *src = imgPixels + yStart * imgPitch;
		Uint8 *dst = resPixels + yStart * imgPitch;

		for (int y = yStart; y < yEnd; ++y) {
			for (int x = 0; x < w; ++x) {
				const Uint8 oldR = src[Rshift / 8];
				const Uint8 oldG = src[Gshift / 8];
				const Uint8 oldB = src[Bshift / 8];
				const Uint8 oldA = src[Ashift / 8];

				const Uint8 newA = Utils::inBounds(matrix[15] * oldR + matrix[16] * oldG + matrix[17] * oldB + matrix[18] * oldA + extraA, 0, 255);
				if (newA) {
					const Uint8 newR = Utils::inBounds(matrix[ 0] * oldR + matrix[ 1] * oldG + matrix[ 2] * oldB + matrix[ 3] * oldA + extraR, 0, 255);
					const Uint8 newG = Utils::inBounds(matrix[ 5] * oldR + matrix[ 6] * oldG + matrix[ 7] * oldB + matrix[ 8] * oldA + extraG, 0, 255);
					const Uint8 newB = Utils::inBounds(matrix[10] * oldR + matrix[11] * oldG + matrix[12] * oldB + matrix[13] * oldA + extraB, 0, 255);

					dst[Rshift / 8] = newR;
					dst[Gshift / 8] = newG;
					dst[Bshift / 8] = newB;
					dst[Ashift / 8] = newA;
				}

				src += 4;
				dst += 4;
			}
		}

		std::lock_guard<std::mutex> g(guard);
		++countFinished;
	};

	if (countThreads != 1) {
		{
			std::lock_guard<std::mutex> g(processingMutex);
			for (size_t i = 0; i < countThreads; ++i) {
				partsToProcessing.push_back(std::make_pair(i, f));
			}
		}
		while (countFinished != countThreads) {
			Utils::sleep(1);
		}
	}else {
		f(0);
	}

	return res;
}

SurfacePtr Image::reColor(const std::vector<String> &args) {
	if (args.size() != 3) {
		Utils::outMsg("Image::reColor", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = getImage(args[1]);
	if (!img) return nullptr;

	const String colorsStr = Utils::clear(args[2]);
	const std::vector<String> colorsVec = colorsStr.split(' ');
	if (colorsVec.size() != 4) {
		Utils::outMsg("Image::reColor", "Неверное количество цветов:\n<" + colorsStr + ">");
		return img;
	}

	const int r = colorsVec[0].toDouble();
	const int g = colorsVec[1].toDouble();
	const int b = colorsVec[2].toDouble();
	const int a = colorsVec[3].toDouble();

	if (r == 256 && g == 256 && b == 256 && a == 256) {
		return img;
	}

	const Uint8 *imgPixels = (const Uint8*)img->pixels;

	const int w = img->w;
	const int h = img->h;
	const int imgPitch = img->pitch;

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(img->flags, w, h, 32, img->format->format),
				   SDL_FreeSurface);
	if (!a) return res;

	Uint8 *resPixels = (Uint8*)res->pixels;


	const size_t countThreads = std::min(Image::countThreads * partsOnThreads, size_t(h));
	size_t countFinished = 0;
	std::mutex guard;

	auto f = [&](const size_t num) {
		const int yStart = h * (double(num) / countThreads);
		const int yEnd = h * (double(num + 1) / countThreads);

		const Uint8 *src = imgPixels + yStart * imgPitch;
		Uint8 *dst = resPixels + yStart * imgPitch;

		for (int y = yStart; y < yEnd; ++y) {
			for (int x = 0; x < w; ++x) {
				if (src[Ashift / 8]) {
					dst[Rshift / 8] = src[Rshift / 8] * r / 256;
					dst[Gshift / 8] = src[Gshift / 8] * g / 256;
					dst[Bshift / 8] = src[Bshift / 8] * b / 256;
					dst[Ashift / 8] = src[Ashift / 8] * a / 256;
				}

				src += 4;
				dst += 4;
			}

		}

		std::lock_guard<std::mutex> g(guard);
		++countFinished;
	};

	if (countThreads != 1) {
		{
			std::lock_guard<std::mutex> g(processingMutex);
			for (size_t i = 0; i < countThreads; ++i) {
				partsToProcessing.push_back(std::make_pair(i, f));
			}
		}
		while (countFinished != countThreads) {
			Utils::sleep(1);
		}
	}else {
		f(0);
	}

	return res;
}

SurfacePtr Image::rotozoom(const std::vector<String> &args) {
	if (args.size() != 4) {
		Utils::outMsg("Image::rotozoom", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = getImage(args[1]);
	if (!img) return nullptr;

	const int angle = -Utils::clear(args[2]).toDouble();
	double zoom = Utils::clear(args[3]).toDouble();
	if (!zoom) {
		Utils::outMsg("Image::rotozoom", "zoom не должен быть равен 0");
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

	const double absSin = abs(Utils::getSin(angle));
	const double absCos = abs(Utils::getCos(angle));
	const int resW = w * absCos + h * absSin;
	const int resH = w * absSin + h * absCos;

	const SDL_Rect srcRect = {0, 0, img->w, img->h};
	const SDL_Rect dstRect = {(resW - w) / 2, (resH - h) / 2, w, h};

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(img->flags, resW, resH, 32, img->format->format), SDL_FreeSurface);

	std::shared_ptr<SDL_Renderer> renderer(SDL_CreateSoftwareRenderer(res.get()), SDL_DestroyRenderer);

	if (!renderer) {
		Utils::outMsg("Image::rotozoom, SDL_CreateSoftwareRenderer", SDL_GetError());
		return nullptr;
	}

	TexturePtr texture(SDL_CreateTextureFromSurface(renderer.get(), img.get()), SDL_DestroyTexture);
	if (!texture) {
		Utils::outMsg("Image::rotozoom, SDL_CreateTextureFromSurface", SDL_GetError());
		return nullptr;
	}

	if (SDL_RenderCopyEx(renderer.get(), texture.get(), &srcRect, &dstRect, angle, nullptr, flip)) {
		Utils::outMsg("Image::rotozoom, SDL_RenderCopyEx", SDL_GetError());
		return nullptr;
	}

	return res;
}




static inline Uint32 get1(Uint32 a, Uint32) { return a; }
static inline Uint32 get2(Uint32, Uint32 a) { return a; }

template<typename GetChannel, typename CmpFunc, typename GetPixel, typename GetAlpha>
static void maskCycles(std::mutex &guard,
					   const int value,
					   const size_t num,
					   const size_t countThreads, size_t &countFinished,
					   SurfacePtr img, const Uint8 *maskPixels, Uint8 *resPixels,
					   GetChannel getChannel, CmpFunc cmp,
					   GetPixel getPixel, GetAlpha getAlphaFunc)
{
	const Uint8 *imgPixels = (const Uint8*)img->pixels;

	const int w = img->w;
	const int h = img->h;
	const int imgPitch = img->pitch;
	const int imgBpp = 4;

	const int yStart = h * (double(num) / countThreads);
	const int yEnd = h * (double(num + 1) / countThreads);
	for (int y = yStart; y < yEnd; ++y) {
		for (int x = 0; x < w; ++x) {
			const Uint32 maskPixel = *(const Uint32*)(maskPixels + y * imgPitch + x * imgBpp);
			const Uint8 channel = getChannel(maskPixel);

			if (cmp(channel, value)) {
				const Uint32 imgPixel = *(const Uint32*)(imgPixels + y * imgPitch + x * imgBpp);
				const Uint8 a = getAlphaFunc(getPixel(imgPixel, maskPixel));

				if (a) {
					const Uint8 r = getRed(imgPixel);
					const Uint8 g = getGreen(imgPixel);
					const Uint8 b = getBlue(imgPixel);

					const Uint32 resPixel = getPixelColor(r, g, b, a);
					*(Uint32*)(resPixels + y * imgPitch + x * imgBpp) = resPixel;
				}
			}
		}
	}

	std::lock_guard<std::mutex> g(guard);
	++countFinished;
}
template<typename GetChannel, typename GetPixel, typename GetAlpha>
static void maskChooseCmp(std::mutex &guard,
						  const int value, const String &cmp,
						  const size_t num,
						  const size_t countThreads, size_t &countFinished,
						  SurfacePtr img, const Uint8 *maskPixels, Uint8 *resPixels,
						  GetChannel getChannel,
						  GetPixel getPixel, GetAlpha getAlphaFunc)
{
	if (cmp == "l") {
		maskCycles(guard, value, num, countThreads, countFinished,
				   img, maskPixels, resPixels, getChannel, std::less<int>(), getPixel, getAlphaFunc);
	}else
	if (cmp == "g") {
		maskCycles(guard, value, num, countThreads, countFinished,
				   img, maskPixels, resPixels, getChannel, std::greater<int>(), getPixel, getAlphaFunc);
	}else
	if (cmp == "e") {
		maskCycles(guard, value, num, countThreads, countFinished,
				   img, maskPixels, resPixels, getChannel, std::equal_to<int>(), getPixel, getAlphaFunc);
	}else
	if (cmp == "ne") {
		maskCycles(guard, value, num, countThreads, countFinished,
				   img, maskPixels, resPixels, getChannel, std::not_equal_to<int>(), getPixel, getAlphaFunc);
	}else
	if (cmp == "le") {
		maskCycles(guard, value, num, countThreads, countFinished,
				   img, maskPixels, resPixels, getChannel, std::less_equal<int>(), getPixel, getAlphaFunc);
	}else {//ge
		maskCycles(guard, value, num, countThreads, countFinished,
				   img, maskPixels, resPixels, getChannel, std::greater_equal<int>(), getPixel, getAlphaFunc);
	}
}
template<typename GetChannel>
static void maskChooseAlpha(std::mutex &guard,
							const char alphaChannel, bool alphaFromImage,
							const int value, const String &cmp,
							const size_t num,
							const size_t countThreads, size_t &countFinished,
							SurfacePtr img, const Uint8 *maskPixels, Uint8 *resPixels,
							GetChannel getChannel)
{
	if (alphaChannel == 'r') {
		if (alphaFromImage) {
			maskChooseCmp(guard, value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getChannel, get1, getRed);
		}else {
			maskChooseCmp(guard, value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getChannel, get2, getRed);
		}
	}else
	if (alphaChannel == 'g') {
		if (alphaFromImage) {
			maskChooseCmp(guard, value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getChannel, get1, getGreen);
		}else {
			maskChooseCmp(guard, value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getChannel, get2, getGreen);
		}
	}else
	if (alphaChannel == 'b') {
		if (alphaFromImage) {
			maskChooseCmp(guard, value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getChannel, get1, getBlue);
		}else {
			maskChooseCmp(guard, value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getChannel, get2, getBlue);
		}
	}else {//a
		if (alphaFromImage) {
			maskChooseCmp(guard, value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getChannel, get1, getAlpha);
		}else {
			maskChooseCmp(guard, value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getChannel, get2, getAlpha);
		}
	}
}

SurfacePtr Image::mask(const std::vector<String> &args) {
	if (args.size() != 8) {
		Utils::outMsg("Image::mask", "Неверное количество аргументов:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	SurfacePtr img = getImage(args[1]);
	if (!img) return nullptr;

	SurfacePtr mask = getImage(args[2]);
	if (!mask) return nullptr;

	if (img->w != mask->w || img->h != mask->h) {
		const String imgSize = String(img->w) + "x" + String(img->h);
		const String maskSize = String(mask->w) + "x" + String(mask->h);
		Utils::outMsg("Image::mask", "Размеры маски " + imgSize + " не соответствуют размерам изображения " + maskSize + ":\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	const String channelStr = Utils::clear(args[3]);
	if (channelStr != "r" && channelStr != "g" && channelStr != "b" && channelStr != "a") {
		Utils::outMsg("Image::mask", "Некорректный канал <" + channelStr + ">, ожидалось r, g, b или a:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	const char channel = channelStr[0];

	const String valueStr = Utils::clear(args[4]);
	if (!valueStr.isNumber()) {
		Utils::outMsg("Image::mask", "Значение <" + valueStr + "> должно являться числом:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	const int value = valueStr.toDouble();

	const String cmp = Utils::clear(args[5]);
	static const std::vector<String> cmps = {"l", "g", "e", "ne", "le", "ge"};
	if (!Utils::in(cmp, cmps)) {
		Utils::outMsg("Image::mask", "Некорректная функция сравнения <" + cmp + ">, ожидалось l(<), g(>), e(==), ne(!=), le(<=), ge(>=):\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}

	const String alphaStr = Utils::clear(args[6]);
	if (alphaStr != "r" && alphaStr != "g" && alphaStr != "b" && alphaStr != "a") {
		Utils::outMsg("Image::mask", "Некорректный канал <" + alphaStr + ">, ожидалось r, g, b или a:\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}
	const char alphaChannel = alphaStr[0];

	const String alphaImage = Utils::clear(args[7]);
	if (alphaImage != "1" && alphaImage != "2") {
		Utils::outMsg("Image::mask", "Некорректный номер изображения <" + alphaStr + ">, ожидалось 1 (основа) или 2 (маска):\n<" + String::join(args, ", ") + ">");
		return nullptr;
	}


	const Uint8 *maskPixels = (Uint8*)mask->pixels;

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(img->flags, img->w, img->h, 32, img->format->format),
				   SDL_FreeSurface);
	Uint8 *resPixels = (Uint8*)res->pixels;

	const size_t countThreads = std::min(Image::countThreads * partsOnThreads, size_t(img->h));
	size_t countFinished = 0;
	std::mutex guard;


	auto chooseChannel = [&](const size_t num) {
		if (channel == 'r') {
			maskChooseAlpha(guard, alphaChannel, alphaImage == "1", value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getRed);
		}else
		if (channel == 'g') {
			maskChooseAlpha(guard, alphaChannel, alphaImage == "1", value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getGreen);
		}else
		if (channel == 'b') {
			maskChooseAlpha(guard, alphaChannel, alphaImage == "1", value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getBlue);
		}else {//a
			maskChooseAlpha(guard, alphaChannel, alphaImage == "1", value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getAlpha);
		}
	};

	if (countThreads != 1) {
		{
			std::lock_guard<std::mutex> g(processingMutex);
			for (size_t i = 0; i < countThreads; ++i) {
				partsToProcessing.push_back(std::make_pair(i, chooseChannel));
			}
		}
		while (countFinished != countThreads) {
			Utils::sleep(1);
		}
	}else {
		chooseChannel(0);
	}

	return res;
}
