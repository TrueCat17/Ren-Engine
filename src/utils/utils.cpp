#include "utils.h"

#include <iostream>
#include <thread>
#include <time.h>

#include <boost/filesystem.hpp>
#include <Python.h>

#include "gv.h"
#include "config.h"

#include "gui/display_object.h"

#include "media/image.h"
#include "media/py_utils.h"

#include "parser/node.h"

#include "game.h"


String Utils::ROOT;
String Utils::FONTS;

std::map<String, TTF_Font*> Utils::fonts;

std::map<String, String> Utils::images;
std::map<String, Node*> Utils::declAts;

std::vector<std::pair<String, SDL_Texture*>> Utils::textures;
std::map<const SDL_Texture*, SDL_Surface*> Utils::textureSurfaces;

std::mutex Utils::surfaceGuard;
std::vector<std::pair<String, SDL_Surface*>> Utils::surfaces;

std::chrono::system_clock::time_point Utils::startTime = std::chrono::system_clock::now();


std::vector<String> Utils::getFileNames(String path) {
	std::vector<String> res;

	namespace fs = boost::filesystem;

	if (!fs::exists(path.c_str())) {
		outMsg("Директории <" + path + "> не существует");
		static const String mainMenu = "../resources/mods/main_menu/";
		if (path != mainMenu) {
			return getFileNames(mainMenu);
		}
		return res;
	}
	if (!fs::is_directory(path)) {
		outMsg("<" + path + "> - это файл, а не директория");
		return res;
	}

	fs::path dir(path.c_str());
	for (fs::recursive_directory_iterator it(dir), end; it != end; ++it) {
		fs::path filePath(it->path());
		if (fs::is_regular_file(filePath)) {
			res.push_back(filePath.string());
		}
	}

	return res;
}

void Utils::init() {
	ROOT = "../resources/";
	FONTS = ROOT + "fonts/";
}

bool Utils::isFirstByte(char c) {
	//2-й и последующие байты в 2-чном представлении в UTF-8 начинаются с 10 (0b10xxxxxx)
	return !(c & 0b10000000) || c & 0b01000000;
}

int Utils::getTimer() {
	auto now = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
	return duration.count();
}
void Utils::sleep(int ms, bool checkInGame) {
	const int MIN_SLEEP = Game::getFrameTime();

	while ((GV::inGame || !checkInGame) && ms >= MIN_SLEEP) {
		std::this_thread::sleep_for(std::chrono::milliseconds(MIN_SLEEP));
		ms -= MIN_SLEEP;
	}
	if (ms <= 0) return;

	if (GV::inGame || !checkInGame) {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}
}

void Utils::outMsg(std::string msg, const std::string& err) {
	static std::map<std::string, bool> errors;

	if (err.size()) {
		msg += " Error:\n" + err;

		if (errors[msg]) return;
		errors[msg] = true;
	}
	if (msg.size()) {
		static std::mutex msgGuard;

		msgGuard.lock();
		if (SDL_ShowSimpleMessageBox(err.size() ? SDL_MESSAGEBOX_ERROR : SDL_MESSAGEBOX_WARNING, "Message", msg.c_str(), GV::mainWindow)) {
			std::cout << msg << '\n';
		}
		msgGuard.unlock();
	}
}


TTF_Font* Utils::getFont(const String &path, int size) {
	const String t = path + "/" + size;

	auto it = fonts.find(t);
	if (it != fonts.end()) {
		return it->second;
	}

	TTF_Font *res = TTF_OpenFont(path.c_str(), size);
	if (res) {
		fonts[t] = res;
	}
	return res;
}
void Utils::destroyAllFonts() {
	for (auto i = fonts.begin(); i != fonts.end(); ++i) {
		TTF_Font *font = i->second;
		TTF_CloseFont(font);
	}
}


size_t Utils::getStartArg(const String& args, size_t end) {
	size_t res = end + 1;
	while (res < args.size() && args[res] == ' ') {
		++res;
	}
	if (res >= args.size()) {
		return -1;
	}
	return res;
}
size_t Utils::getEndArg(const String& args, size_t start) {
	bool wasSpace = false;
	int b1 = 0;//open (
	int b2 = 0;//open [
	bool q1 = false;//open '
	bool q2 = false;//open "

	size_t i = start;
	for (; i < args.size(); ++i) {
		char c = args[i];
		wasSpace = wasSpace || (c == ' ');

		if (wasSpace && !b1 && !b2 && !q1 && !q2) break;

		if (c == '\'' && !q2) {
			q1 = !q1;
		}else
			if (c == '"' && !q1) {
				q2 = !q2;
			}else
				if (!q1 && !q2) {
					if (c == '(') ++b1;
					else if (c == ')') --b1;
					else if (c == '[') ++b2;
					else if (c == ']') --b2;
				}
	}
	if (i == args.size() && (b1 || b2 || q1 || q2)) {
		String error = "Не закрыта " +
							((b1 || b2)
							 ? (String(b1 ? "круглая" : "квадратная")) + " скобка"
							 : (String(q1 ? "одинарная" : "двойная") + " кавычка")
						);
		Utils::outMsg("Utils::getEndArg", error);
	}
	return i;
}

String Utils::clear(String s) {
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
			Utils::outMsg("Utils::clear", "Путаница в скобках:\n<" + s + ">");
			return nullptr;
		}
	}
	start = n;
	end = s.size() - n;
	s = s.substr(start, end - start);

	return s;
}

std::vector<String> Utils::getArgs(String args) {
	size_t start = args.find_first_not_of(' ');
	size_t end = args.find_last_not_of(' ') + 1;

	if (start == size_t(-1)) start = 0;
	if (!end) end = args.size();
	args = args.substr(start, end - start);

	std::vector<String> res;

	start = 0;
	while (start != size_t(-1)) {
		end = Utils::getEndArg(args, start);

		String t = args.substr(start, end - start);
		if (t) {
			res.push_back(t);
		}

		start = Utils::getStartArg(args, end);
	}
	return res;
}


size_t Utils::getTextureWidth(SDL_Texture *texture) {
	const SDL_Surface *surface = textureSurfaces[texture];
	if (surface) {
		return surface->w;
	}
	Utils::outMsg("Utils::getTextureWidth", "surface == nullptr");
	return 0;
}
size_t Utils::getTextureHeight(SDL_Texture *texture) {
	const SDL_Surface *surface = textureSurfaces[texture];
	if (surface) {
		return surface->h;
	}
	Utils::outMsg("Utils::getTextureHeight", "surface == nullptr");
	return 0;
}


void Utils::trimTexturesCache(const SDL_Surface *last) {
	/* Если размер кэша текстур превышает желаемый, то удалять неиспользуемые сейчас текстуры до тех пор,
	 * пока кэш не уменьшится до нужных размеров
	 * Текстуры удаляются в порядке последнего использования (сначала те, что использовались давно)
	 * Может оказаться так, что все текстуры нужны, тогда никакая из них не удаляется и кэш превышает желаемый размер
	 * После всего этого новая текстура добавляется в конец кэша
	 */

	const size_t MAX_SIZE = Config::get("max_size_textures_cache").toInt() * (1 << 20);//в МБ

	size_t cacheSize = last->w * last->h * 4;
	for (auto &p : textures) {
		const SDL_Texture *texture = p.second;
		const SDL_Surface *surface = textureSurfaces[texture];
		cacheSize += surface->w * surface->h * 4;
	}

	size_t i = 0;
	while (cacheSize > MAX_SIZE) {
		while (i < textures.size() && DisplayObject::useTexture(textures[i].second)) {
			++i;
		}
		if (i == textures.size()) break;

		SDL_Texture *texture = textures[i].second;
		const SDL_Surface *surface = textureSurfaces[texture];
		cacheSize -= surface->w * surface->h * 4;

		SDL_DestroyTexture(texture);

		textureSurfaces.erase(textureSurfaces.find(texture));
		textures.erase(textures.begin() + i, textures.begin() + i + 1);
	}
}

SDL_Texture* Utils::getTexture(const String &path) {
	if (!path) return nullptr;

	for (size_t i = textures.size() - 1; i != size_t(-1); --i) {
		std::pair<String, SDL_Texture*> t = textures[i];
		if (t.first == path) {
			if (i < textures.size() - 10) {
				textures.erase(textures.begin() + i, textures.begin() + i + 1);
				textures.push_back(t);
			}
			return t.second;
		}
	}

	SDL_Surface *surface = Image::getImage(path);
	if (surface) {
		GV::renderGuard.lock();
		SDL_Texture *texture = SDL_CreateTextureFromSurface(GV::mainRenderer, surface);
		GV::renderGuard.unlock();

		if (texture) {
			textureSurfaces[texture] = surface;

			trimTexturesCache(surface);
			textures.push_back(std::pair<String, SDL_Texture*>(path, texture));
			return texture;
		}

		outMsg("SDL_CreateTextureFromSurface", SDL_GetError());
	}else {
		outMsg("IMG_Load", IMG_GetError());
	}

	return nullptr;
}

void Utils::trimSurfacesCache(const SDL_Surface* last) {
	surfaceGuard.lock();

	const size_t MAX_SIZE = Config::get("max_size_surfaces_cache").toInt() * (1 << 20);

	auto usedSomeTexture = [&](const SDL_Surface *s) -> bool {
		for (auto i : textureSurfaces) {
			if (i.second == s) {
				return true;
			}
		}
		return false;
	};

	size_t cacheSize = last->w * last->h * 4;
	for (auto &p : surfaces) {
		const SDL_Surface *surface = p.second;
		cacheSize += surface->w * surface->h * 4;
	}

	size_t i = 0;
	while (cacheSize > MAX_SIZE) {
		SDL_Surface *surface = nullptr;
		while (i < surfaces.size() && usedSomeTexture(surface = surfaces[i].second)) {
			++i;
		}
		if (i == surfaces.size()) break;

		cacheSize -= surface->w * surface->h * 4;

		SDL_FreeSurface(surface);
		surfaces.erase(surfaces.begin() + i, surfaces.begin() + i + 1);
	}
	surfaceGuard.unlock();
}
SDL_Surface* Utils::getThereIsSurfaceOrNull(const String &path) {
	surfaceGuard.lock();
	for (size_t i = surfaces.size() - 1; i != size_t(-1); --i) {
		std::pair<String, SDL_Surface*> t = surfaces[i];
		if (t.first == path) {
			if (i < surfaces.size() - 10) {
				surfaces.erase(surfaces.begin() + i, surfaces.begin() + i + 1);
				surfaces.push_back(t);
			}
			surfaceGuard.unlock();
			return t.second;
		}
	}
	surfaceGuard.unlock();
	return nullptr;
}

void Utils::setSurface(const String &path, SDL_Surface *surface) {
	if (!surface) return;

	surfaceGuard.lock();
	for (auto &p : surfaces) {
		String &pPath = p.first;
		if (pPath == path) {
			p.second = surface;
			surfaceGuard.unlock();
			return;
		}
	}
	surfaceGuard.unlock();

	trimSurfacesCache(surface);

	surfaceGuard.lock();
	surfaces.push_back(std::pair<String, SDL_Surface*>(path, surface));
	surfaceGuard.unlock();
}

SDL_Surface* Utils::getSurface(const String &path) {
	if (!path) return nullptr;

	SDL_Surface *t = getThereIsSurfaceOrNull(path);
	if (t) return t;

	static std::mutex mutex;
	mutex.lock();

	t = getThereIsSurfaceOrNull(path);
	if (t)  {
		mutex.unlock();
		return t;
	}

	SDL_Surface *surface = IMG_Load((Utils::ROOT + path).c_str());
	if (surface) {
		/*
		 * Вот это вот - это костыль
		 *
		 * Текстура от результата от SDL_ConvertSurfaceFormat почему-то не хочет менять прозрачность
		 * Поэтому для непрозрачных картинок (все jpg, например) используется SDL_CreateRGBSurface и SDL_BlitSurface
		 *
		 * Ну а SDL_BlitSurface почему-то искажает _полупрозрачные_ пиксели (например, красные -> чёрные)
		 * Поэтому для поверхностей с прозрачностью используется SDL_ConvertSurfaceFormat
		 *
		 * Как сделать нормально - я не знаю
		 */
		if (surface->format->Amask) {
			SDL_Surface *newSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
			if (newSurface) {
				SDL_FreeSurface(surface);
				surface = newSurface;
			}else{
				Utils::outMsg("SDL_ConvertSurfaceFormat", SDL_GetError());
			}
		}else {
			SDL_Rect imgRect = { 0, 0, surface->w, surface->h };
			SDL_Surface *newSurface = SDL_CreateRGBSurface(0, imgRect.w, imgRect.h, 32,
														   0xFF << 24, 0xFF << 16, 0xFF << 8, 0xFF);
			if (newSurface) {
				SDL_BlitSurface(surface, &imgRect, newSurface, &imgRect);
				SDL_FreeSurface(surface);
				surface = newSurface;
			}else{
				Utils::outMsg("SDL_CreateRGBSurface", SDL_GetError());
			}
		}

		trimSurfacesCache(surface);

		surfaceGuard.lock();
		surfaces.push_back(std::pair<String, SDL_Surface*>(path, surface));
		surfaceGuard.unlock();

		mutex.unlock();
		return surface;
	}

	outMsg("IMG_Load", IMG_GetError());
	mutex.unlock();
	return nullptr;
}

Uint32 Utils::getPixel(const SDL_Surface *surface, const SDL_Rect &draw, const SDL_Rect &crop) {
	if (!surface) {
		Utils::outMsg("Utils::getPixel", "surface == nullptr");
		return 0;
	}

	int x = crop.x + draw.x * crop.w / draw.w;
	int y = crop.y + draw.y * crop.h / draw.h;

	if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
		Utils::outMsg("Utils::getPixel", String() +
					  "Попытка получить цвет точки (" + x + ", " + y + ") у картинки размера " + surface->w + "x" + surface->h);
		return 0;
	}

	const int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
	case 4:
		return *(Uint32*)p;
	case 3:
		if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		return p[0] | p[1] << 8 | p[2] << 16;
	case 2:
		return *(Uint16*)p;
	case 1:
		return *p;
	default:
		return 0;       /* shouldn't happen, but avoids warnings */
	}
}
Uint32 Utils::getPixel(SDL_Texture *texture, const SDL_Rect &draw, const SDL_Rect &crop) {
	const SDL_Surface *surface = textureSurfaces[texture];
	return getPixel(surface, draw, crop);
}

bool Utils::registerImage(const String &desc, Node *declAt) {
	String name;
	String path;

	size_t i = desc.find("=");
	if (i == size_t(-1)) {
		name = desc;
		path = "";
	}else {
		name = desc.substr(0, i - 1);
		path = desc.substr(i + 1);
	}

	auto clear = [](String str) -> String {
		size_t start = str.find_first_not_of(' ');
		size_t end = str.find_last_not_of(' ');
		if (start == size_t(-1) || end == size_t(-1)) {
			return "";
		}
		return str.substr(start, end - start + 1);
	};

	name = clear(name);
	if (!name) {
		return false;
	}

	path = clear(path);

	images[name] = path;
	declAts[name] = declAt;

	if (declAt->children.empty()) {
		PyUtils::pyExecGuard.lock();
		py::list defaultDeclAt = py::list(GV::pyUtils->pythonGlobal["default_decl_at"]);
		PyUtils::pyExecGuard.unlock();

		size_t sizeDefaultDeclAt = py::len(defaultDeclAt);
		for (size_t i = 0; i < sizeDefaultDeclAt; ++i) {
			py::str params = py::str(defaultDeclAt[i]);

			Node *node = new Node("some assign default_decl_at", 0);
			node->params = std::string(py::extract<std::string>(params));
			declAt->children.push_back(node);
		}
	}

	return true;
}

bool Utils::imageWasRegistered(const std::string &name) {
	return images.find(name) != images.end();
}

std::string Utils::getImageCode(const std::string &name) {
	if (images.find(name) == images.end()) {
		Utils::outMsg("Utils::getImage", "Изображение <" + name + "> не зарегистрировано");
		return "";
	}

	return images[name];
}
py::list Utils::getImageDeclAt(const std::string &name) {
	if (declAts.find(name) == declAts.end()) {
		Utils::outMsg("Utils::getImage", "Изображение <" + name + "> не зарегистрировано");
		return py::list();
	}

	Node *node = declAts[name];
	if (!node) return py::list();

	return node->getPyList();
}
