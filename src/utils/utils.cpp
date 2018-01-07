#include "utils.h"

#include <iostream>
#include <thread>

#include <boost/filesystem.hpp>
#include <boost/python.hpp>
#include <Python.h>

#include "gv.h"
#include "config.h"
#include "logger.h"
#include "renderer.h"

#include "media/image.h"
#include "media/py_utils.h"

#include "parser/node.h"

#include "game.h"


String Utils::ROOT;
String Utils::FONTS;

std::map<String, TTF_Font*> Utils::fonts;

std::map<String, String> Utils::images;
std::map<String, Node*> Utils::declAts;

std::vector<std::pair<SurfacePtr, TexturePtr>> Utils::textures;

std::mutex Utils::surfaceMutex;
std::vector<std::pair<String, SurfacePtr>> Utils::surfaces;

double* Utils::sins = new double[360];
double* Utils::coss = new double[360];

std::chrono::system_clock::time_point Utils::startTime = std::chrono::system_clock::now();


std::vector<String> Utils::getFileNames(const std::string &path) {
	std::vector<String> res;

	namespace fs = boost::filesystem;

	if (!fs::exists(path)) {
		outMsg("Директории <" + path + "> не существует");
		static const std::string mainMenu = "../resources/mods/main_menu/";
		if (path != mainMenu) {
			return getFileNames(mainMenu);
		}
		return res;
	}

	for (fs::recursive_directory_iterator it(path), end; it != end; ++it) {
		fs::path filePath(it->path());
		if (fs::is_regular_file(filePath)) {
			res.push_back(filePath.string());
		}
	}

	return res;
}

void Utils::init() {
	char *charsPath = SDL_GetBasePath();
	String basePath = charsPath ? charsPath : "";
	SDL_free(charsPath);
	basePath.replaceAll('\\', '/');

	ROOT = basePath + "../resources/";
	FONTS = ROOT + "fonts/";

	for (size_t i = 0; i < 360; ++i) {
		sins[i] = std::sin(i * M_PI / 180);
		coss[i] = std::cos(i * M_PI / 180);
	}
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
void Utils::sleepMicroSeconds(int ms) {
	std::this_thread::sleep_for(std::chrono::microseconds(ms));
}

void Utils::outMsg(std::string msg, const std::string &err) {
	static std::map<std::string, bool> errors;

	if (err.size()) {
		msg += " Error:\n" + err;

		if (errors[msg]) return;
		errors[msg] = true;
	}
	if (msg.size()) {
		static std::mutex msgGuard;
		std::lock_guard<std::mutex> g(msgGuard);

		Logger::log(msg + "\n\n");
		if (SDL_ShowSimpleMessageBox(err.size() ? SDL_MESSAGEBOX_ERROR : SDL_MESSAGEBOX_WARNING, "Message", msg.c_str(), GV::mainWindow)) {
			std::cout << msg << '\n';
		}
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
	fonts.clear();
}


size_t Utils::getStartArg(const String &args, size_t end, const char separator) {
	size_t res = end + 1;
	while (res < args.size() && args[res] == separator) {
		++res;
	}
	if (res >= args.size()) {
		return -1;
	}
	return res;
}
size_t Utils::getEndArg(const String &args, size_t start, const char separator) {
	bool wasSeparator = false;
	int b1 = 0;//open (
	int b2 = 0;//open [
	bool q1 = false;//open '
	bool q2 = false;//open "

	size_t i = start;
	for (; i < args.size(); ++i) {
		const char c = args[i];
		wasSeparator = wasSeparator || (c == separator);

		if (wasSeparator && !b1 && !b2 && !q1 && !q2) break;

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
	if (start || end != s.size()) {
		s = s.substr(start, end - start);
	}

	size_t n = 0;
	while (n < s.size() && s[n] == '(') {
		if (s[s.size() - n - 1] == ')') {
			++n;
		}else{
			Utils::outMsg("Utils::clear", "Путаница в скобках:\n<" + s + ">");
			return nullptr;
		}
	}

	if (n) {
		start = n;
		end = s.size() - n;
		s = s.substr(start, end - start);
	}

	return s;
}

std::vector<String> Utils::getArgs(String args, const char separator) {
	size_t start = args.find_first_not_of(' ');
	size_t end = args.find_last_not_of(' ') + 1;

	if (start == size_t(-1)) start = 0;
	if (!end) end = args.size();
	if (start || end != args.size()) {
		args = args.substr(start, end - start);
	}

	std::vector<String> res;

	start = 0;
	while (start != size_t(-1)) {
		end = Utils::getEndArg(args, start, separator);

		String t = args.substr(start, end - start);
		if (t) {
			res.push_back(t);
		}

		start = Utils::getStartArg(args, end, separator);
	}
	return res;
}


void Utils::trimTexturesCache(const SurfacePtr &last) {
	/* Если размер кэша текстур превышает желаемый, то удалять неиспользуемые сейчас текстуры до тех пор,
	 * пока кэш не уменьшится до нужных размеров
	 * Текстуры удаляются в порядке последнего использования (сначала те, что использовались давно)
	 * Может оказаться так, что все текстуры нужны, тогда никакая из них не удаляется и кэш превышает желаемый размер
	 * После всего этого новая текстура добавляется в конец кэша
	 */

	const size_t MAX_SIZE = Config::get("max_size_textures_cache").toInt() * (1 << 20);//в МБ

	size_t cacheSize = last->w * last->h * 4;
	for (const std::pair<SurfacePtr, TexturePtr> &p : textures) {
		const SurfacePtr &surface = p.first;
		cacheSize += surface->w * surface->h * 4;
	}

	if (cacheSize > MAX_SIZE) {
		for (size_t i = 0; i < textures.size(); ++i) {
			const SurfacePtr &surface = textures[i].first;
			if (surface.use_count() == 1) {
				cacheSize -= surface->w * surface->h * 4;

				textures.erase(textures.begin() + i, textures.begin() + i + 1);
				--i;
			}
		}
	}

	size_t i = 0;
	while (cacheSize > MAX_SIZE) {
		while (i < textures.size() && textures[i].second.use_count() > 1) {//1 - value in textures
			++i;
		}
		if (i == textures.size()) break;

		const std::pair<SurfacePtr, TexturePtr> &p = textures[i];
		const SurfacePtr &surface = p.first;

		cacheSize -= surface->w * surface->h * 4;

		textures.erase(textures.begin() + i, textures.begin() + i + 1);
	}
}

TexturePtr Utils::getTexture(const SurfacePtr &surface) {
	if (!surface) return nullptr;

	const int N = 20;
	for (size_t i = textures.size() - 1; i != size_t(-1); --i) {
		if (textures[i].first == surface) {
			if (textures.size() > N && i < textures.size() - N) {
				std::pair<SurfacePtr, TexturePtr> t = textures[i];

				textures.erase(textures.begin() + i, textures.begin() + i + 1);
				textures.push_back(t);

				i = textures.size() - 1;
			}
			return textures[i].second;
		}
	}

	TexturePtr texture(SDL_CreateTextureFromSurface(GV::mainRenderer, surface.get()), SDL_DestroyTexture);

	if (texture) {
		trimTexturesCache(surface);
		textures.push_back(std::make_pair(surface, texture));
		return texture;
	}

	outMsg("SDL_CreateTextureFromSurface", SDL_GetError());
	return nullptr;
}

void Utils::trimSurfacesCache(const SurfacePtr &last) {
	const size_t MAX_SIZE = Config::get("max_size_surfaces_cache").toInt() * (1 << 20);

	size_t cacheSize = last->w * last->h * 4;

	std::map<SDL_Surface*, long> countSurfaces;
	for (const std::pair<String, SurfacePtr> &p : surfaces) {
		const SurfacePtr &surface = p.second;
		if (countSurfaces.find(surface.get()) == countSurfaces.end()) {
			cacheSize += surface->w * surface->h * 4;
		}
		countSurfaces[surface.get()] += 1;
	}
	std::map<SDL_Surface*, long> countSurfacesStart = countSurfaces;

	std::vector<SurfacePtr> toDelete;
	size_t i = 0;
	while (cacheSize > MAX_SIZE) {
		while (i < surfaces.size() &&
			   surfaces[i].second.use_count() > countSurfacesStart[surfaces[i].second.get()]
		) {
			++i;
		}
		if (i == surfaces.size()) break;

		const std::pair<String, SurfacePtr> &p = surfaces[i];
		const SurfacePtr &surface = p.second;

		if (!(countSurfaces[surface.get()] -= 1)) {
			cacheSize -= surface->w * surface->h * 4;
			toDelete.push_back(surface);
		}
		++i;
	}

	for (const SurfacePtr &s : toDelete) {
		for (i = 0; i < surfaces.size(); ++i) {
			if (surfaces[i].second == s) {
				surfaces.erase(surfaces.begin() + i, surfaces.begin() + i + 1);
				--i;
			}
		}
	}
}
SurfacePtr Utils::getThereIsSurfaceOrNull(const String &path) {
	std::lock_guard<std::mutex> g(surfaceMutex);

	const size_t N = 15;
	for (size_t i = surfaces.size() - 1; i != size_t(-1); --i) {
		if (surfaces[i].first == path) {
			if (surfaces.size() > N && i < surfaces.size() - N) {
				std::pair<String, SurfacePtr> t = surfaces[i];

				surfaces.erase(surfaces.begin() + i, surfaces.begin() + i + 1);
				surfaces.push_back(t);

				i = surfaces.size() - 1;
			}
			return surfaces[i].second;
		}
	}
	return nullptr;
}

SurfacePtr Utils::getSurface(const String &path) {
	if (!path) return nullptr;

	SurfacePtr t = getThereIsSurfaceOrNull(path);
	if (t) return t;

	static std::mutex mutex;
	std::lock_guard<std::mutex> g(mutex);

	t = getThereIsSurfaceOrNull(path);
	if (t) return t;


	String fullPath = Utils::ROOT + path;
	size_t end = fullPath.find('?');
	if (end != size_t(-1)) {
		fullPath.erase(fullPath.begin() + end, fullPath.end());
	}
	for (size_t i = 0; i < fullPath.size(); ++i) {
		if (fullPath[i] == '\\') {
			fullPath[i] = '/';
		}
	}

	SurfacePtr surface(IMG_Load(fullPath.c_str()), SDL_FreeSurface);
	if (surface) {
		if (surface->format->format != SDL_PIXELFORMAT_RGBA32) {
			SurfacePtr newSurface(SDL_CreateRGBSurfaceWithFormat(0, surface->w, surface->h, 32, SDL_PIXELFORMAT_RGBA32),
								  SDL_FreeSurface);
			if (newSurface) {
				SDL_Rect rect = { 0, 0, surface->w, surface->h };
				SDL_BlitSurface(surface.get(), &rect, newSurface.get(), &rect);
				surface = newSurface;
			}else{
				Utils::outMsg("SDL_CreateRGBSurfaceWithFormat", SDL_GetError());
			}
		}

		std::lock_guard<std::mutex> g2(surfaceMutex);
		trimSurfacesCache(surface);
		surfaces.push_back(std::make_pair(path, surface));

		return surface;
	}

	outMsg("IMG_Load", IMG_GetError());
	return nullptr;
}

void Utils::setSurface(const String &path, const SurfacePtr &surface) {
	if (!surface) return;

	std::lock_guard<std::mutex> g(surfaceMutex);
	for (std::pair<String, SurfacePtr> &p : surfaces) {
		const String &pPath = p.first;
		if (pPath == path) {
			p.second = surface;
			return;
		}
	}

	trimSurfacesCache(surface);
	surfaces.push_back(std::make_pair(path, surface));
}

Uint32 Utils::getPixel(const SurfacePtr surface, const SDL_Rect &draw, const SDL_Rect &crop) {
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

	const int bpp = 4;
	Uint32 *pixel = (Uint32*)((Uint8*)(surface->pixels) + y * surface->pitch + x * bpp);

	Uint8 r, g, b, a;
	SDL_GetRGBA(*pixel, surface->format, &r, &g, &b, &a);
	return (r << 24) + (g << 16) + (b << 8) + a;
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
		std::lock_guard<std::mutex> g(PyUtils::pyExecMutex);

		py::dict globals = py::extract<py::dict>(GV::pyUtils->pythonGlobal);
		if (!globals.has_key("default_decl_at")) return true;

		py::object defaultDeclAt = globals["default_decl_at"];
		if (!PyUtils::isTuple(defaultDeclAt) && !PyUtils::isList(defaultDeclAt)) return true;

		size_t sizeDefaultDeclAt = Py_SIZE(defaultDeclAt.ptr());
		for (size_t i = 0; i < sizeDefaultDeclAt; ++i) {
			Node *node = new Node("some assign default_decl_at", 0);
			node->params = py::extract<const std::string>(py::str(defaultDeclAt[i]));
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
		Utils::outMsg("Utils::getImageCode", "Изображение <" + name + "> не зарегистрировано");
		return "";
	}

	return images[name];
}
py::list Utils::getImageDeclAt(const std::string &name) {
	if (declAts.find(name) == declAts.end()) {
		Utils::outMsg("Utils::getImageDeclAt", "Изображение <" + name + "> не зарегистрировано");
		return py::list();
	}

	const Node *node = declAts[name];
	if (!node) return py::list();

	return node->getPyList();
}
std::vector<String> Utils::getVectorImageDeclAt(const std::string &name) {
	if (declAts.find(name) == declAts.end()) {
		Utils::outMsg("Utils::getVectorImageDeclAt", "Изображение <" + name + "> не зарегистрировано");
		return std::vector<String>();
	}

	const Node *node = declAts[name];
	if (!node) return std::vector<String>();

	return node->getImageChildren();
}

std::pair<String, size_t> Utils::getImagePlace(const std::string &name) {
	if (declAts.find(name) == declAts.end()) {
		Utils::outMsg("Utils::getImage", "Изображение <" + name + "> не зарегистрировано");
		return std::make_pair("NoFile", 0);
	}

	const Node *node = declAts[name];
	if (!node) return std::make_pair("NoFile", 0);

	return std::make_pair(node->getFileName(), node->getNumLine());
}
