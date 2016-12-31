#include "utils.h"

#include <iostream>
#include <thread>
#include <time.h>

#include <boost/filesystem.hpp>
#include <Python.h>

#include "gv.h"
#include "config.h"

#include "gui/display_object.h"

#include "parser/py_guard.h"

#include "game.h"
#include "image.h"


String Utils::ROOT;
String Utils::FONTS;

std::map<String, TTF_Font*> Utils::fonts;
std::map<String, String> Utils::images;

std::vector<std::pair<String, SDL_Texture*>> Utils::textures;
std::map<SDL_Texture*, SDL_Surface*> Utils::textureSurfaces;

std::vector<std::pair<String, SDL_Surface*>> Utils::surfaces;

std::chrono::system_clock::time_point Utils::startTime = std::chrono::system_clock::now();


std::vector<String> Utils::getFileNames(String path) {
	std::vector<String> res;

	namespace fs = boost::filesystem;
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

			/* Если размер кэша текстур превышает желаемый, то удалять неиспользуемые сейчас текстуры до тех пор,
			 * пока кэш не уменьшится до нужных размеров
			 * Текстуры удаляются в порядке последнего использования (сначала те, что использовались давно)
			 * Может оказаться так, что все текстуры нужны, тогда никакая из них не удаляется и кэш превышает желаемый размер
			 * После всего этого новая текстура добавляется в конец кэша
			 */

			const size_t MAX_SIZE = Config::get("max_size_textures_cache").toInt();

			size_t i = 0;
			while (textures.size() >= MAX_SIZE) {
				SDL_Texture *_texture = textures[i].second;
				while (i < textures.size() && DisplayObject::useTexture(_texture)) {
					++i;
					_texture = textures[i].second;
				}
				if (i == textures.size()) break;

				SDL_DestroyTexture(_texture);

				textureSurfaces.erase(textureSurfaces.find(_texture));
				textures.erase(textures.begin() + i, textures.begin() + i + 1);
			}
			textures.push_back(std::pair<String, SDL_Texture*>(path, texture));
			return texture;
		}

		outMsg("SDL_CreateTextureFromSurface", SDL_GetError());
	}else {
		outMsg("IMG_Load", IMG_GetError());
	}

	return nullptr;
}
void Utils::destroyAllTextures() {
	for (size_t i = 0; i < textures.size(); ++i) {
		SDL_Texture *texture = textures[i].second;
		if (!DisplayObject::useTexture(texture)) {
			SDL_DestroyTexture(texture);

			textures.erase(textures.begin() + i, textures.begin() + i + 1);
			textureSurfaces.erase(textureSurfaces.find(texture));
			--i;
		}
	}
}

SDL_Surface* Utils::getSurface(const String &path) {
	if (!path) return nullptr;

	for (size_t i = surfaces.size() - 1; i != size_t(-1); --i) {
		std::pair<String, SDL_Surface*> t = surfaces[i];
		if (t.first == path) {
			if (i < surfaces.size() - 10) {
				surfaces.erase(surfaces.begin() + i, surfaces.begin() + i + 1);
				surfaces.push_back(t);
			}
			return t.second;
		}
	}

	SDL_Surface *surface = IMG_Load((Utils::ROOT + path).c_str());
	if (surface) {
		SDL_Surface *newSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
		if (newSurface) {
			SDL_FreeSurface(surface);
			surface = newSurface;
		}else{
			Utils::outMsg("SDL_ConvertSurfaceFormat", SDL_GetError());
		}

		const size_t MAX_SIZE = Config::get("max_size_surfaces_cache").toInt();

		auto usedSomeTexture = [&](const SDL_Surface *s) -> bool {
			for (auto i : textureSurfaces) {
				if (i.second == s) {
					return true;
				}
			}
			return false;
		};

		size_t i = 0;
		while (surfaces.size() >= MAX_SIZE) {
			SDL_Surface *_surface = surfaces[i].second;
			while (i < surfaces.size() && usedSomeTexture(_surface)) {
				++i;
				_surface = surfaces[i].second;
			}
			if (i == surfaces.size()) break;

			SDL_FreeSurface(_surface);
			surfaces.erase(surfaces.begin() + i, surfaces.begin() + i + 1);
		}
		surfaces.push_back(std::pair<String, SDL_Surface*>(path, surface));
		return surface;
	}

	outMsg("IMG_Load", IMG_GetError());
	return nullptr;
}
void Utils::destroyAllSurfaces() {
	for (auto i : surfaces) {
		SDL_Surface *surface = i.second;
		SDL_FreeSurface(surface);
	}
	surfaces.clear();
}

Uint32 Utils::getPixel(const SDL_Surface *surface, int x, int y, int drawW, int drawH) {
	if (!surface) {
		Utils::outMsg("Utils::getPixel", "surface == nullptr");
		return 0;
	}

	int w = surface->w;
	int h = surface->h;

	if (drawW && drawH) {
		x = double(x) * w / drawW;
		y = double(y) * h / drawH;
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
Uint32 Utils::getPixel(SDL_Texture *texture, int x, int y, int drawW, int drawH) {
	const SDL_Surface *surface = textureSurfaces[texture];
	return getPixel(surface, x, y, drawW, drawH);
}

void Utils::registerImage(const String &desc) {
	size_t i = desc.find("=");
	if (i == size_t(-1)) {
		outMsg("Неверное объявление изображения (нет знака '='):\n" + desc);
		return;
	}
	bool spaceL = desc[i - 1] == ' ';
	bool spaceR = desc[i + 1] == ' ';

	const String name = desc.substr(0, i - spaceL);
	const String path = desc.substr(i + spaceR + 1);
	images[name] = path;
}
std::string Utils::getImageCode(const std::string &name) {
	if (images.find(name) == images.end()) {
		Utils::outMsg("Utils::getImage", "Изображение <" + name + "> не зарегистрировано");
		return "";
	}

	return images[name];
}

String Utils::execPython(String code, bool retRes) {
	if (!code) return "";

	if (code.isDouble()) {
		return code;
	}
	if (code.isSimpleString()) {
		return code.substr(1, code.size() - 2);
	}
	if (code == "True" || code == "False") {
		return code;
	}


	String res = "empty";
	if (retRes) {
		code = "res = str(" + code + ")";
	}

	static std::mutex pyExecGuard;

	pyExecGuard.lock();
	try {
		PyCodeObject *co = PyGuard::getCompileObject(code);
		if (co) {
			if (!PyEval_EvalCode(co, GV::pyGuard->pythonGlobal.ptr(), nullptr)) {
				throw py::error_already_set();
			}

			if (retRes) {
				py::object resObj = GV::pyGuard->pythonGlobal["res"];
				const std::string resStr = py::extract<const std::string>(resObj);
				res = resStr;
			}
		}else {
			throw py::error_already_set();
		}
	}catch (py::error_already_set) {
		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
		PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);

		py::handle<> htype(ptype);
		std::string excType = py::extract<std::string>(py::str(htype));
		if (excType == "<type 'exceptions.StopIteration'>") {
			pyExecGuard.unlock();
			Py_DecRef(pvalue);
			Py_DecRef(ptraceback);
			throw StopException();
		}

		py::handle<> hvalue(py::allow_null(pvalue));
		std::string excValue = py::extract<std::string>(py::str(hvalue));

		std::string traceback;
		if (!String(excValue).startsWith("invalid syntax")) {
			py::handle<> htraceback(py::allow_null(ptraceback));
			GV::pyGuard->pythonGlobal["traceback"] = htraceback;

			String code = "traceback_str = get_traceback(traceback)";
			try {
				py::exec(code.c_str(), GV::pyGuard->pythonGlobal);
				traceback = py::extract<std::string>(GV::pyGuard->pythonGlobal["traceback_str"]);
			}catch (py::error_already_set) {
				traceback = "Error on call get_traceback\n";
			}
		}

		std::cout << "Python Error (" + excType + "):\n"
				  << '\t' << excValue << '\n';

		if (traceback.size()) {
			std::cout << "Traceback:\n"
					  << '\t' << traceback;
		}

		std::cout << "Code:\n"
				  << code << "\n\n";
	}catch (...) {
		std::cout << "Python Unknown Error\n";
		std::cout << "Code:\n" << code << '\n';
	}
	pyExecGuard.unlock();

	return res;
}
