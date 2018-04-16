#include "utils.h"

#include <iostream>
#include <chrono>
#include <thread>

#include <boost/filesystem.hpp>
#include <boost/python.hpp>
#include <Python.h>

#include "gv.h"
#include "config.h"
#include "logger.h"

#include "media/image.h"
#include "media/py_utils.h"

#include "parser/node.h"

#include "utils/game.h"


std::map<String, TTF_Font*> Utils::fonts;

std::map<String, String> Utils::images;
std::map<String, Node*> Utils::declAts;


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

int Utils::getTimer() {
	static auto startTime = std::chrono::system_clock::now();

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


TTF_Font* Utils::getFont(const String &name, int size) {
	const String t = name + "|" + size;

	auto it = fonts.find(t);
	if (it != fonts.end()) {
		return it->second;
	}

	const String path = "fonts/" + name + ".ttf";
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


Uint32 Utils::getPixel(const SurfacePtr &surface, const SDL_Rect &draw, const SDL_Rect &crop) {
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

	const Uint8 *pixel = ((const Uint8 *)surface->pixels + y * surface->pitch + x * 4);
	const Uint8 r = pixel[Rshift / 8];
	const Uint8 g = pixel[Gshift / 8];
	const Uint8 b = pixel[Bshift / 8];
	const Uint8 a = pixel[Ashift / 8];

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

			PyObject *elem = PySequence_Fast_GET_ITEM(defaultDeclAt.ptr(), i);
			if (PyString_CheckExact(elem)) {
				node->params = PyString_AS_STRING(elem);
			}else {
				try {
					PyObject *str = PyObject_Str(elem);
					if (str) {
						node->params = PyString_AS_STRING(str);
					}
					Py_DecRef(str);
				}catch (py::error_already_set) {
					PyUtils::errorProcessing("str(elem)");
				}
			}

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
