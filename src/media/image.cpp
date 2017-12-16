#include "image.h"

#include <thread>
#include <mutex>

#include <SDL2/SDL_image.h>


std::map<String, std::function<SurfacePtr(const std::vector<String>&)>> Image::functions;
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
}


void Image::loadImage(const std::string &desc) {
	std::thread(getImage, desc).detach();
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


	const std::vector<String> args = Utils::getArgs(desc);

	if (args.empty()) {
		Utils::outMsg("Image::getImage", "Список аргументов пуст");
		return nullptr;
	}
	if (args.size() == 1) {
		const String imagePath = Utils::clear(args[0]);

		res = Utils::getSurface(imagePath);
		return res;
	}


	const String command = args[0];

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
	SDL_Rect resRect = { 0, 0, int(img->w * k), int(img->h * k) };

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


	size_t countThreads = std::min(8, h);
	size_t countFinished = 0;
	std::mutex guard;

	auto f = [&](const int num) {
		const int yStart = h * (double(num) / countThreads);
		const int yEnd = h * (double(num + 1) / countThreads);
		for (int y = yStart; y < yEnd; ++y) {
			for (int x = 0; x < w; ++x) {
				const int _y = vertical ? h - y - 1 : y;
				const int _x = horizontal ? w - x - 1 : x;

				const Uint32 imgPixel = *(const Uint32*)(imgPixels + _y * imgPitch + _x * imgBpp);
				Uint8 r, g, b, a;
				SDL_GetRGBA(imgPixel, img->format, &r, &g, &b, &a);

				const Uint32 resPixel = SDL_MapRGBA(img->format, r, g, b, a);
				*(Uint32*)(resPixels + y * imgPitch + x * imgBpp) = resPixel;
			}
		}

		std::lock_guard<std::mutex> g(guard);
		++countFinished;
	};

	for (size_t i = 0; i < countThreads; ++i) {
		std::thread(f, i).detach();
	}
	while (countFinished != countThreads) {
		Utils::sleep(1);
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
	const int imgBpp = 4;

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(img->flags, w, h, 32, img->format->format),
				   SDL_FreeSurface);

	Uint8 *resPixels = (Uint8*)res->pixels;


	const size_t countThreads = std::min(8, h);
	size_t countFinished = 0;
	std::mutex guard;

	auto f = [&](const int num) {
		const int yStart = h * (double(num) / countThreads);
		const int yEnd = h * (double(num + 1) / countThreads);
		for (int y = yStart; y < yEnd; ++y) {
			for (int x = 0; x < w; ++x) {
				const Uint32 imgPixel = *(const Uint32*)(imgPixels + y * imgPitch + x * imgBpp);
				Uint8 oldR, oldG, oldB, oldA;
				SDL_GetRGBA(imgPixel, img->format, &oldR, &oldG, &oldB, &oldA);

				const Uint8 newA = Utils::inBounds(matrix[15] * oldR + matrix[16] * oldG + matrix[17] * oldB + matrix[18] * oldA + matrix[19] * 255, 0, 255);
				if (newA) {
					const Uint8 newR = Utils::inBounds(matrix[0] * oldR + matrix[1] * oldG + matrix[2] * oldB + matrix[3] * oldA + matrix[4] * 255, 0, 255);
					const Uint8 newG = Utils::inBounds(matrix[5] * oldR + matrix[6] * oldG + matrix[7] * oldB + matrix[8] * oldA + matrix[9] * 255, 0, 255);
					const Uint8 newB = Utils::inBounds(matrix[10] * oldR + matrix[11] * oldG + matrix[12] * oldB + matrix[13] * oldA + matrix[14] * 255, 0, 255);
					const Uint32 resPixel = SDL_MapRGBA(img->format, newR, newG, newB, newA);
					*(Uint32*)(resPixels + y * imgPitch + x * imgBpp) = resPixel;
				}
			}
		}

		std::lock_guard<std::mutex> g(guard);
		++countFinished;
	};

	for (size_t i = 0; i < countThreads; ++i) {
		std::thread(f, i).detach();
	}
	while (countFinished != countThreads) {
		Utils::sleep(1);
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

	if (r == 1 && g == 1 && b == 1 && a == 1) {
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


	const size_t countThreads = std::min(8, h);
	size_t countFinished = 0;
	std::mutex guard;

	auto f = [&](const int num) {
		const int yStart = h * (double(num) / countThreads);
		const int yEnd = h * (double(num + 1) / countThreads);
		for (int y = yStart; y < yEnd; ++y) {
			for (int x = 0; x < w; ++x) {
				const Uint32 imgPixel = *(const Uint32*)(imgPixels + y * imgPitch + x * imgBpp);
				Uint8 oldR, oldG, oldB, oldA;
				SDL_GetRGBA(imgPixel, img->format, &oldR, &oldG, &oldB, &oldA);

				const Uint8 newA = Utils::inBounds(oldA * a / 255, 0, 255);
				if (newA) {
					const Uint8 newR = Utils::inBounds(oldR * r / 255, 0, 255);
					const Uint8 newG = Utils::inBounds(oldG * g / 255, 0, 255);
					const Uint8 newB = Utils::inBounds(oldB * b / 255, 0, 255);
					const Uint32 resPixel = SDL_MapRGBA(img->format, newR, newG, newB, newA);
					*(Uint32*)(resPixels + y * imgPitch + x * imgBpp) = resPixel;
				}
			}
		}

		std::lock_guard<std::mutex> g(guard);
		++countFinished;
	};

	for (size_t i = 0; i < countThreads; ++i) {
		std::thread(f, i).detach();
	}
	while (countFinished != countThreads) {
		Utils::sleep(1);
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

	const double absSin = std::abs(Utils::getSin(angle));
	const double absCos = std::abs(Utils::getCos(angle));
	const int resW = w * absCos + h * absSin;
	const int resH = w * absSin + h * absCos;

	const SDL_Rect srcRect = {0, 0, img->w, img->h};
	const SDL_Rect dstRect = {(resW - w) / 2, (resH - h) / 2, w, h};

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(img->flags, resW, resH, 32, img->format->format), SDL_FreeSurface);

	SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(res.get());
	if (!renderer) {
		Utils::outMsg("Image::rotozoom, SDL_CreateSoftwareRenderer", SDL_GetError());
		return nullptr;
	}else {
		TexturePtr texture(SDL_CreateTextureFromSurface(renderer, img.get()), SDL_DestroyTexture);
		if (!texture) {
			Utils::outMsg("Image::rotozoom, SDL_CreateTextureFromSurface", SDL_GetError());
			return nullptr;
		}else {
			if (SDL_RenderCopyEx(renderer, texture.get(), &srcRect, &dstRect, angle, nullptr, flip)) {
				Utils::outMsg("Image::rotozoom, SDL_RenderCopyEx", SDL_GetError());
				return nullptr;
			}
		}
		SDL_DestroyRenderer(renderer);
	}

	return res;
}


template<typename T1, typename T2>
static void maskCycles(std::mutex &guard,
					   const int value,
					   const int num,
					   const size_t countThreads, size_t &countFinished,
					   SurfacePtr img, const Uint8 *maskPixels, Uint8 *resPixels,
					   T1 getChannel,
					   T2 cmp)
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
			Uint8 channel = getChannel(maskPixel, img->format);

			if (cmp(channel, value)) {
				const Uint32 imgPixel = *(const Uint32*)(imgPixels + y * imgPitch + x * imgBpp);
				Uint8 r, g, b, a;
				SDL_GetRGBA(imgPixel, img->format, &r, &g, &b, &a);

				if (a) {
					const Uint32 resPixel = SDL_MapRGBA(img->format, r, g, b, a);
					*(Uint32*)(resPixels + y * imgPitch + x * imgBpp) = resPixel;
				}
			}
		}
	}

	std::lock_guard<std::mutex> g(guard);
	++countFinished;
}
template<typename GetColor>
static void maskChooseCmp(std::mutex &guard,
						  const int value, const String &cmp,
						  const int num,
						  const size_t countThreads, size_t &countFinished,
						  SurfacePtr img, const Uint8 *maskPixels, Uint8 *resPixels,
						  GetColor getColor)
{
	if (cmp == "l") {
		maskCycles(guard, value, num, countThreads, countFinished,
			  img, maskPixels, resPixels, getColor, std::less<int>());
	}else
	if (cmp == "g") {
		maskCycles(guard, value, num, countThreads, countFinished,
			  img, maskPixels, resPixels, getColor, std::greater<int>());
	}else
	if (cmp == "e") {
		maskCycles(guard, value, num, countThreads, countFinished,
			  img, maskPixels, resPixels, getColor, std::equal_to<int>());
	}else
	if (cmp == "ne") {
		maskCycles(guard, value, num, countThreads, countFinished,
			  img, maskPixels, resPixels, getColor, std::not_equal_to<int>());
	}else
	if (cmp == "le") {
		maskCycles(guard, value, num, countThreads, countFinished,
			  img, maskPixels, resPixels, getColor, std::less_equal<int>());
	}else {//ge
		maskCycles(guard, value, num, countThreads, countFinished,
			  img, maskPixels, resPixels, getColor, std::greater_equal<int>());
	}
}

SurfacePtr Image::mask(const std::vector<String> &args) {
	if (args.size() != 6) {
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

	const Uint8 *maskPixels = (Uint8*)mask->pixels;

	SurfacePtr res(SDL_CreateRGBSurfaceWithFormat(img->flags, img->w, img->h, 32, img->format->format),
				   SDL_FreeSurface);
	Uint8 *resPixels = (Uint8*)res->pixels;

	const size_t countThreads = std::min(8, img->h);
	size_t countFinished = 0;
	std::mutex guard;


	for (size_t i = 0; i < countThreads; ++i) {
		auto getRed   = [](Uint32 pixel, const SDL_PixelFormat *format) -> Uint8 { return (pixel & format->Rmask) >> format->Rshift; };
		auto getGreen = [](Uint32 pixel, const SDL_PixelFormat *format) -> Uint8 { return (pixel & format->Gmask) >> format->Gshift; };
		auto getBlue  = [](Uint32 pixel, const SDL_PixelFormat *format) -> Uint8 { return (pixel & format->Bmask) >> format->Bshift; };
		auto getAlpha = [](Uint32 pixel, const SDL_PixelFormat *format) -> Uint8 { return (pixel & format->Amask) >> format->Ashift; };

		auto chooseChannel = [&](const int num) {
			if (channel == 'r') {
				maskChooseCmp(guard, value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getRed);
			}else
			if (channel == 'g') {
				maskChooseCmp(guard, value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getGreen);
			}else
			if (channel == 'b') {
				maskChooseCmp(guard, value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getBlue);
			}else {//a
				maskChooseCmp(guard, value, cmp, num, countThreads, countFinished, img, maskPixels, resPixels, getAlpha);
			}
		};
		std::thread(chooseChannel, i).detach();
	}
	while (countFinished != countThreads) {
		Utils::sleep(1);
	}

	return res;
}
