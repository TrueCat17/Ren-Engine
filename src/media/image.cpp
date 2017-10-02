#include "image.h"

#include <thread>
#include <mutex>

#include <SDL2/SDL_image.h>



void Image::loadImage(const std::string &desc) {
	auto f = [](const std::string desc) {
		static std::mutex m;
		std::lock_guard<std::mutex> g(m);
		getImage(desc);
	};
	std::thread(f, desc).detach();
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


	struct D {
		const String &desc;

		D(const String& desc): desc(desc) {}

		~D() {
			std::lock_guard<std::mutex> g(vecMutex);

			for (size_t i = 0; i < processingImages.size(); ++i) {
				if (processingImages.at(i) == desc) {
					processingImages.erase(processingImages.begin() + i, processingImages.begin() + i + 1);
					return;
				}
			}
		}
	};
	D d(desc);


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

	if (command == "Scale") {
		SurfacePtr img = getImage(args[1]);
		if (!img) return nullptr;

		SDL_Rect imgRect = { 0, 0, img->w, img->h };

		SDL_Rect resRect = { 0, 0, Utils::clear(args[2]).toInt(), Utils::clear(args[3]).toInt() };
		res.reset(SDL_CreateRGBSurface(img->flags, resRect.w, resRect.h, 32,
									   img->format->Rmask, img->format->Gmask, img->format->Bmask, img->format->Amask),
				  SDL_FreeSurface);

		SDL_BlitScaled(img.get(), &imgRect, res.get(), &resRect);
	}else

	if (command == "FactorScale") {
		SurfacePtr img = getImage(args[1]);
		if (!img) return nullptr;

		SDL_Rect imgRect = { 0, 0, img->w, img->h };
		double k = Utils::clear(args[2]).toDouble();
		SDL_Rect resRect = { 0, 0, int(img->w * k), int(img->h * k) };

		res.reset(SDL_CreateRGBSurface(img->flags, resRect.w, resRect.h, 32,
									   img->format->Rmask, img->format->Gmask, img->format->Bmask, img->format->Amask),
				  SDL_FreeSurface);

		SDL_BlitScaled(img.get(), &imgRect, res.get(), &resRect);
	}else

	if (command == "Crop") {
		SurfacePtr img = getImage(args[1]);
		if (!img) return nullptr;

		String rectStr = Utils::clear(args[2]);
		std::vector<String> rectVec = rectStr.split(' ');

		SDL_Rect imgRect = { rectVec[0].toInt(), rectVec[1].toInt(), rectVec[2].toInt(), rectVec[3].toInt() };
		SDL_Rect resRect = { 0, 0, imgRect.w, imgRect.h };

		res.reset(SDL_CreateRGBSurface(img->flags, resRect.w, resRect.h, 32,
									   img->format->Rmask, img->format->Gmask, img->format->Bmask, img->format->Amask),
				  SDL_FreeSurface);

		SDL_BlitScaled(img.get(), &imgRect, res.get(), &resRect);
	}else

	if (command == "Composite") {
		String sizeStr = Utils::clear(args[1]);
		std::vector<String> sizeVec = sizeStr.split(' ');

		SDL_Rect resRect = { 0, 0, sizeVec[0].toInt(), sizeVec[1].toInt() };
		res.reset(SDL_CreateRGBSurface(0, resRect.w, resRect.h, 32,
									   0xFF << 24, 0xFF << 16, 0xFF << 8, 0xFF),
				  SDL_FreeSurface);

		for (size_t i = 2; i < args.size(); i += 2) {
			String posStr = Utils::clear(args[i]);
			std::vector<String> posVec = posStr.split(' ');

			String imgStr = Utils::clear(args[i + 1]);
			SurfacePtr img = getImage(imgStr);
			if (!img) {
				Utils::outMsg("Image::getImage, Composite", "Не удалось получить изображение <" + imgStr + ">");
				continue;
			}

			SDL_Rect fromRect = { 0, 0, img->w, img->h };
			SDL_Rect toRect = { posVec[0].toInt(), posVec[1].toInt(), img->w, img->h };

			SDL_BlitScaled(img.get(), &fromRect, res.get(), &toRect);
		}
	}else

	if (command == "Flip") {
		SurfacePtr img = getImage(args[1]);
		if (!img) return nullptr;

		bool horizontal = Utils::clear(args[2]) == "True";
		bool vertical = Utils::clear(args[3]) == "True";

		Uint8 *imgPixels = (Uint8*)img->pixels;
		SDL_PixelFormat *imgPixelFormat = img->format;
		Uint32 rMask = imgPixelFormat->Rmask;
		Uint32 gMask = imgPixelFormat->Gmask;
		Uint32 bMask = imgPixelFormat->Bmask;
		Uint32 aMask = imgPixelFormat->Amask;

		const int w = img->w;
		const int h = img->h;
		const int imgPitch = img->pitch;
		const int imgBpp = img->format->BytesPerPixel;

		res.reset(SDL_CreateRGBSurface(img->flags, w, h, 32, rMask, gMask, bMask, aMask),
				  SDL_FreeSurface);

		SDL_PixelFormat *resPixelFormat = res->format;
		const int resPitch = res->pitch;
		const int resBpp = res->format->BytesPerPixel;
		Uint8 *resPixels = (Uint8*)res->pixels;


		size_t countThreads = std::min(8, h);
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

			std::lock_guard<std::mutex> g(guard);
			++countFinished;
		};

		for (size_t i = 0; i < countThreads; ++i) {
			std::thread(f, i).detach();
		}
		while (countFinished != countThreads) {
			Utils::sleep(1);
		}
	}else

	if (command == "MatrixColor") {
		SurfacePtr img = getImage(args[1]);
		if (!img) return nullptr;

		String matrixStr = Utils::clear(args[2]);
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

		const int w = img->w;
		const int h = img->h;
		const int imgPitch = img->pitch;
		const int imgBpp = img->format->BytesPerPixel;

		res.reset(SDL_CreateRGBSurface(img->flags, w, h, 32, rMask, gMask, bMask, aMask),
				  SDL_FreeSurface);

		SDL_PixelFormat *resPixelFormat = res->format;
		const int resPitch = res->pitch;
		const int resBpp = res->format->BytesPerPixel;
		Uint8 *resPixels = (Uint8*)res->pixels;


		const size_t countThreads = std::min(8, h);
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

					Uint8 newA = Utils::inBounds(matrix[15] * oldR + matrix[16] * oldG + matrix[17] * oldB + matrix[18] * oldA + matrix[19] * 255, 0.0, 255.0);
					if (newA) {
						Uint8 newR = Utils::inBounds(matrix[0] * oldR + matrix[1] * oldG + matrix[2] * oldB + matrix[3] * oldA + matrix[4] * 255, 0.0, 255.0);
						Uint8 newG = Utils::inBounds(matrix[5] * oldR + matrix[6] * oldG + matrix[7] * oldB + matrix[8] * oldA + matrix[9] * 255, 0.0, 255.0);
						Uint8 newB = Utils::inBounds(matrix[10] * oldR + matrix[11] * oldG + matrix[12] * oldB + matrix[13] * oldA + matrix[14] * 255, 0.0, 255.0);
						Uint32 resPixel = SDL_MapRGBA(resPixelFormat, newR, newG, newB, newA);
						*(Uint32*)(resPixels + y * resPitch + x * resBpp) = resPixel;
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
	}else

	if (command == "ReColor") {
		SurfacePtr img = getImage(args[1]);
		if (!img) return nullptr;

		String colorsStr = Utils::clear(args[2]);
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

		const int w = img->w;
		const int h = img->h;
		const int imgPitch = img->pitch;
		const int imgBpp = img->format->BytesPerPixel;

		res.reset(SDL_CreateRGBSurface(img->flags, w, h, 32, rMask, gMask, bMask, aMask),
				  SDL_FreeSurface);

		SDL_PixelFormat *resPixelFormat = res->format;
		const int resPitch = res->pitch;
		const int resBpp = res->format->BytesPerPixel;
		Uint8 *resPixels = (Uint8*)res->pixels;


		const size_t countThreads = std::min(8, h);
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

					Uint32 resPixel;
					Uint8 newA = oldA * colors[3] / 255;
					if (newA) {
						Uint8 newR = oldR * colors[0] / 255;
						Uint8 newG = oldG * colors[1] / 255;
						Uint8 newB = oldB * colors[2] / 255;
						resPixel = SDL_MapRGBA(resPixelFormat, newR, newG, newB, newA);
					}else {
						resPixel = 0;
					}
					*(Uint32*)(resPixels + y * resPitch + x * resBpp) = resPixel;
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
	}else 
		
	if (command == "Rotozoom") {
		SurfacePtr img = getImage(args[1]);
		if (!img) return nullptr;
		

		int angle = -Utils::clear(args[2]).toDouble();
		double zoom = Utils::clear(args[3]).toDouble();
		if (!zoom) {
			Utils::outMsg("Image::getImage, Rotozoom", "zoom не должен быть равен 0");
			zoom = 1;
		}
		SDL_RendererFlip flip = SDL_FLIP_NONE;
		if (zoom < 0) {
			zoom = -zoom;
			flip = SDL_RendererFlip(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
		}

		SDL_PixelFormat *imgPixelFormat = img->format;
		Uint32 rMask = imgPixelFormat->Rmask;
		Uint32 gMask = imgPixelFormat->Gmask;
		Uint32 bMask = imgPixelFormat->Bmask;
		Uint32 aMask = imgPixelFormat->Amask;

		int w = std::max(img->w * zoom, 1.0);
		int h = std::max(img->h * zoom, 1.0);

		double radians = angle * M_PI / 180;
		double sinA = std::abs(std::sin(radians));
		double cosA = std::abs(std::cos(radians));
		int resW = w * cosA + h * sinA;
		int resH = w * sinA + h * cosA;

		SDL_Rect srcRect = {0, 0, img->w, img->h};
		SDL_Rect dstRect = {(resW - w) / 2, (resH - h) / 2, w, h};

		res.reset(SDL_CreateRGBSurface(img->flags, resW, resH, 32, rMask, gMask, bMask, aMask),
				  SDL_FreeSurface);


		String error;

		SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(res.get());

		if (!renderer) {
			error = "Image::getImage, Rotozoom, SDL_CreateSoftwareRenderer";
		}else {
			TexturePtr texture(SDL_CreateTextureFromSurface(renderer, img.get()), SDL_DestroyTexture);
			if (!texture) {
				error = "Image::getImage, Rotozoom, SDL_CreateTextureFromSurface";
			}else {
				if (SDL_RenderCopyEx(renderer, texture.get(), &srcRect, &dstRect, angle, nullptr, flip)) {
					error = "Image::getImage, Rotozoom, SDL_RenderCopyEx";
				}
			}

			SDL_DestroyRenderer(renderer);
		}
		if (error) {
			Utils::outMsg(error, SDL_GetError());
			res = nullptr;
		}
	}

	else {
		Utils::outMsg("Image::getImage", "Неизвестная команда <" + command + ">");
	}

	Utils::setSurface(desc, res);
	return res;
}
