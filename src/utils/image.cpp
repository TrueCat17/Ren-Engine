#include "image.h"

#include <iostream>
#include <thread>
#include <mutex>

#include <SDL2/SDL_image.h>

#include "utils/utils.h"

String Image::clear(String s) {
	size_t start = s.find_first_not_of(' ');
	size_t end = s.find_last_not_of(' ') + 1;

	if (start == size_t(-1)) start = 0;
	if (!end) end = s.size();
	s = s.substr(start, end - start);


	size_t n = 0;
	while (n < s.size() && s[n] == '(') {
		if (s[s.size() - n - 1] == ')') {
			++n;
		}else{
			Utils::outMsg("Image::clear", "Путаница в скобках:\n<" + s + ">");
			return nullptr;
		}
	}
	start = n;
	end = s.size() - n;
	s = s.substr(start, end - start);

	return s;
}

SDL_Surface* Image::getImage(String desc) {
	SDL_Surface *res = nullptr;

	desc = clear(desc);
	std::vector<String> args = Utils::getArgs(desc);

	if (args.empty()) {
		Utils::outMsg("Image::getImage", "Список аргументов пуст");
		return res;
	}
	if (args.size() == 1) {
		String imagePath = clear(args[0]);

		res = Utils::getSurface(imagePath);
		return res;
	}


	String command = args[0];

	if (command == "Scale") {
		SDL_Surface *img = getImage(args[1]);
		if (!img) return nullptr;

		SDL_Rect imgRect = { 0, 0, img->w, img->h };

		SDL_Rect resRect = { 0, 0, clear(args[2]).toInt(), clear(args[3]).toInt() };
		res = SDL_CreateRGBSurface(img->flags, resRect.w, resRect.h, 32,
								   img->format->Rmask, img->format->Gmask, img->format->Bmask, img->format->Amask);
		SDL_BlitScaled(img, &imgRect, res, &resRect);
	}else

	if (command == "FactorScale") {
		SDL_Surface *img = getImage(args[1]);
		if (!img) return nullptr;

		SDL_Rect imgRect = { 0, 0, img->w, img->h };
		double k = clear(args[2]).toDouble();
		SDL_Rect resRect = { 0, 0, int(img->w * k), int(img->h * k) };
		res = SDL_CreateRGBSurface(img->flags, resRect.w, resRect.h, 32,
								   img->format->Rmask, img->format->Gmask, img->format->Bmask, img->format->Amask);
		SDL_BlitScaled(img, &imgRect, res, &resRect);
	}else

	if (command == "Crop") {
		SDL_Surface *img = getImage(args[1]);
		if (!img) return nullptr;

		String rectStr = clear(args[2]);
		std::vector<String> rectVec = rectStr.split(' ');

		SDL_Rect imgRect = { rectVec[0].toInt(), rectVec[1].toInt(), rectVec[2].toInt(), rectVec[3].toInt() };
		SDL_Rect resRect = { 0, 0, imgRect.w, imgRect.h };

		res = SDL_CreateRGBSurface(img->flags, resRect.w, resRect.h, 32,
								   img->format->Rmask, img->format->Gmask, img->format->Bmask, img->format->Amask);
		SDL_BlitScaled(img, &imgRect, res, &resRect);
	}else

	if (command == "Composite") {
		String sizeStr = clear(args[1]);
		std::vector<String> sizeVec = sizeStr.split(' ');

		SDL_Rect resRect = { 0, 0, sizeVec[0].toInt(), sizeVec[1].toInt() };
		res = SDL_CreateRGBSurface(0, resRect.w, resRect.h, 32,
								   0xFF << 24, 0xFF << 16, 0xFF << 8, 0xFF);

		for (size_t i = 2; i < args.size(); i += 2) {
			String posStr = clear(args[i]);
			std::vector<String> posVec = posStr.split(' ');

			String imgStr = clear(args[i + 1]);
			SDL_Surface *img = getImage(imgStr);
			if (!img) return nullptr;

			if (!img) {
				Utils::outMsg("Image::getImage, Composite", "Не удалось получить изображение <" + imgStr + ">");
				continue;
			}

			SDL_Rect fromRect = { 0, 0, img->w, img->h };
			SDL_Rect toRect = { posVec[0].toInt(), posVec[1].toInt(), img->w, img->h };
			SDL_BlitScaled(img, &fromRect, res, &toRect);
		}
	}else

	if (command == "Flip") {
		SDL_Surface *img = getImage(args[1]);
		if (!img) return nullptr;

		bool horizontal = clear(args[2]) == "True";
		bool vertical = clear(args[3]) == "True";

		Uint8 *imgPixels = (Uint8*)img->pixels;
		SDL_PixelFormat *imgPixelFormat = img->format;
		Uint32 rMask = imgPixelFormat->Rmask;
		Uint32 gMask = imgPixelFormat->Gmask;
		Uint32 bMask = imgPixelFormat->Bmask;
		Uint32 aMask = imgPixelFormat->Amask;

		int w = img->w;
		int h = img->h;
		int imgPitch = img->pitch;
		int imgBpp = img->format->BytesPerPixel;

		res = SDL_CreateRGBSurface(img->flags, img->w, img->h, 32, rMask, gMask, bMask, aMask);
		SDL_PixelFormat *resPixelFormat = res->format;
		int resPitch = res->pitch;
		int resBpp = res->format->BytesPerPixel;
		Uint8 *resPixels = (Uint8*)res->pixels;


		size_t countThreads = 8;
		size_t countFinished = 0;
		std::mutex guard;

		auto f = [&](int num) {
			int yStart = h * (double(num) / countThreads);
			int yEnd = h * (double(num + 1) / countThreads);
			for (int y = yStart; y < yEnd; ++y) {
				for (int x = 0; x < w; ++x) {
					int _y = vertical ? h - y - 1 : y;
					int _x = horizontal ? w - x - 1 : x;

					Uint32 imgPixel = *(Uint32*)(imgPixels + _y * imgPitch + _x * imgBpp);
					Uint8 r, g, b, a;
					SDL_GetRGBA(imgPixel, imgPixelFormat, &r, &g, &b, &a);

					Uint32 resPixel = SDL_MapRGBA(resPixelFormat, r, g, b, a);
					*(Uint32*)(resPixels + y * resPitch + x * resBpp) = resPixel;
				}
			}

			guard.lock();
			++countFinished;
			guard.unlock();
		};

		for (size_t i = 0; i < countThreads; ++i) {
			std::thread(f, i).detach();
		}
		while (countFinished != countThreads) {
			Utils::sleep(1);
		}
	}else

	if (command == "MatrixColor") {
		SDL_Surface *img = getImage(args[1]);
		if (!img) return nullptr;

		String matrixStr = clear(args[2]);
		std::vector<String> matrixVec = matrixStr.split(' ');
		if (matrixVec.size() != 25) {
			Utils::outMsg("Image::getImage, MatrixColor",
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

		Uint8 *imgPixels = (Uint8*)img->pixels;
		SDL_PixelFormat *imgPixelFormat = img->format;
		Uint32 rMask = imgPixelFormat->Rmask;
		Uint32 gMask = imgPixelFormat->Gmask;
		Uint32 bMask = imgPixelFormat->Bmask;
		Uint32 aMask = imgPixelFormat->Amask;

		int w = img->w;
		int h = img->h;
		int imgPitch = img->pitch;
		int imgBpp = img->format->BytesPerPixel;

		res = SDL_CreateRGBSurface(img->flags, img->w, img->h, 32, rMask, gMask, bMask, aMask);
		SDL_PixelFormat *resPixelFormat = res->format;
		int resPitch = res->pitch;
		int resBpp = res->format->BytesPerPixel;
		Uint8 *resPixels = (Uint8*)res->pixels;


		size_t countThreads = 8;
		size_t countFinished = 0;
		std::mutex guard;

		auto f = [&](int num) {
			int yStart = h * (double(num) / countThreads);
			int yEnd = h * (double(num + 1) / countThreads);
			for (int y = yStart; y < yEnd; ++y) {
				for (int x = 0; x < w; ++x) {
					Uint32 imgPixel = *(Uint32*)(imgPixels + y * imgPitch + x * imgBpp);
					Uint8 oldR, oldG, oldB, oldA;
					SDL_GetRGBA(imgPixel, imgPixelFormat, &oldR, &oldG, &oldB, &oldA);

					Uint8 newR = Utils::inBounds(matrix[0] * oldR + matrix[1] * oldG + matrix[2] * oldB + matrix[3] * oldA + matrix[4] * 255, 0.0, 255.0);
					Uint8 newG = Utils::inBounds(matrix[5] * oldR + matrix[6] * oldG + matrix[7] * oldB + matrix[8] * oldA + matrix[9] * 255, 0.0, 255.0);
					Uint8 newB = Utils::inBounds(matrix[10] * oldR + matrix[11] * oldG + matrix[12] * oldB + matrix[13] * oldA + matrix[14] * 255, 0.0, 255.0);
					Uint8 newA = Utils::inBounds(matrix[15] * oldR + matrix[16] * oldG + matrix[17] * oldB + matrix[18] * oldA + matrix[19] * 255, 0.0, 255.0);

					Uint32 resPixel = SDL_MapRGBA(resPixelFormat, newR, newG, newB, newA);
					*(Uint32*)(resPixels + y * resPitch + x * resBpp) = resPixel;
				}
			}

			guard.lock();
			++countFinished;
			guard.unlock();
		};

		for (size_t i = 0; i < countThreads; ++i) {
			std::thread(f, i).detach();
		}
		while (countFinished != countThreads) {
			Utils::sleep(1);
		}

		return res;
	}else

	if (command == "ReColor") {
		SDL_Surface *img = getImage(args[1]);
		if (!img) return nullptr;

		String colorsStr = clear(args[2]);
		std::vector<String> colorsVec = colorsStr.split(' ');

		std::vector<double> colors;
		const int colorsSizeToUse = 4;
		colors.resize(colorsSizeToUse);
		for (size_t i = 0; i < colorsSizeToUse; ++i) {
			colors[i] = colorsVec[i].toDouble();
		}

		Uint8 *imgPixels = (Uint8*)img->pixels;
		SDL_PixelFormat *imgPixelFormat = img->format;
		Uint32 rMask = imgPixelFormat->Rmask;
		Uint32 gMask = imgPixelFormat->Gmask;
		Uint32 bMask = imgPixelFormat->Bmask;
		Uint32 aMask = imgPixelFormat->Amask;

		int w = img->w;
		int h = img->h;
		int imgPitch = img->pitch;
		int imgBpp = img->format->BytesPerPixel;

		res = SDL_CreateRGBSurface(img->flags, img->w, img->h, 32, rMask, gMask, bMask, aMask);
		SDL_PixelFormat *resPixelFormat = res->format;
		int resPitch = res->pitch;
		int resBpp = res->format->BytesPerPixel;
		Uint8 *resPixels = (Uint8*)res->pixels;


		size_t countThreads = 8;
		size_t countFinished = 0;
		std::mutex guard;

		auto f = [&](int num) {
			int yStart = h * (double(num) / countThreads);
			int yEnd = h * (double(num + 1) / countThreads);
			for (int y = yStart; y < yEnd; ++y) {
				for (int x = 0; x < w; ++x) {
					Uint32 imgPixel = *(Uint32*)(imgPixels + y * imgPitch + x * imgBpp);
					Uint8 oldR, oldG, oldB, oldA;
					SDL_GetRGBA(imgPixel, imgPixelFormat, &oldR, &oldG, &oldB, &oldA);

					Uint8 newR = oldR * colors[0] / 255;
					Uint8 newG = oldG * colors[1] / 255;
					Uint8 newB = oldB * colors[2] / 255;
					Uint8 newA = oldA * colors[3] / 255;

					Uint32 resPixel = SDL_MapRGBA(resPixelFormat, newR, newG, newB, newA);
					*(Uint32*)(resPixels + y * resPitch + x * resBpp) = resPixel;
				}
			}

			guard.lock();
			++countFinished;
			guard.unlock();
		};

		for (size_t i = 0; i < countThreads; ++i) {
			std::thread(f, i).detach();
		}
		while (countFinished != countThreads) {
			Utils::sleep(1);
		}

		return res;
	}

	else {
		Utils::outMsg("Image::getImage", "Неизвестная команда <" + command + ">");
	}

	return res;
}
