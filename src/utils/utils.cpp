#include "utils.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <set>
#include <map>

#include <SDL2/SDL_ttf.h>

#include "gv.h"
#include "logger.h"

#include "media/image_manipulator.h"
#include "media/py_utils.h"
#include "parser/node.h"
#include "utils/file_system.h"
#include "utils/game.h"


static std::map<std::string, Node*> declAts;


std::vector<std::string> Utils::getFileNames(const std::string &path) {
	if (!FileSystem::exists(path)) {
		outMsg("Utils::getFileNames", "Directory <" + path + "> not found");
		static const std::string mainMenu = "mods/main_menu";
		if (path != mainMenu) {
			return getFileNames(mainMenu);
		}
		return {};
	}

	return FileSystem::getFilesRecursive(path);
}

int Utils::getTimer() {
	static auto startTime = std::chrono::system_clock::now();

	auto now = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
	return int(duration.count());
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


bool Utils::msgOuted = true;
static std::mutex msgGuard;

void Utils::outMsg(std::string msg, const std::string &err) {
	if (err.size()) {
		msg += " Error:\n" + err;

		static std::set<std::string> msgErrors;
		if (msgErrors.count(msg)) return;
		msgErrors.insert(msg);
	}
	if (msg.empty()) return;

	std::lock_guard g(msgGuard);

	Logger::log(msg + "\n\n");

	static bool msgCloseAll = false;
	if (msgCloseAll) return;

	static const SDL_MessageBoxButtonData buttons[] = {
	    {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Ok"},
	    {0, 1, "Close All"}
	};

	static SDL_MessageBoxData data;
	data.flags = err.empty() ? SDL_MESSAGEBOX_WARNING : SDL_MESSAGEBOX_ERROR;
	data.window = GV::mainWindow;
	data.title = "Message";
	data.message = msg.c_str();
	data.numbuttons = 2;
	data.buttons = buttons;
	data.colorScheme = nullptr;

	int res = 0;
	msgOuted = false;
	if (SDL_ShowMessageBox(&data, &res)) {
		std::cout << msg << '\n';
		std::cout << SDL_GetError() << '\n';
	}

	if (res == 1) {
		msgCloseAll = true;
	}
	msgOuted = true;
}
bool Utils::confirm(const char *title, const char *question, const char *yes, const char *no) {
	std::lock_guard g(msgGuard);

	static const SDL_MessageBoxButtonData buttons[] = {
	    {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, yes},
	    {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, no}
	};

	static SDL_MessageBoxData data;
	data.flags = SDL_MESSAGEBOX_INFORMATION;
	data.window = GV::mainWindow;
	data.title = title;
	data.message = question;
	data.numbuttons = 2;
	data.buttons = buttons;
	data.colorScheme = nullptr;

	int res = 0;
	if (SDL_ShowMessageBox(&data, &res)) {
		std::cout << SDL_GetError() << '\n';
	}
	return res == 0;
}


Uint32 Utils::getPixel(const SurfacePtr &surface, const SDL_Rect &draw, const SDL_Rect &crop) {
	if (!surface) {
		Utils::outMsg("Utils::getPixel", "surface == nullptr");
		return 0;
	}

	int x = crop.x + draw.x * crop.w / draw.w;
	int y = crop.y + draw.y * crop.h / draw.h;

	if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
		Utils::outMsg("Utils::getPixel",
		              "Point (" +
		                  std::to_string(x) + ", " + std::to_string(y) +
		              ") is invalid for image with size " +
		                  std::to_string(surface->w) + "x" + std::to_string(surface->h));
		return 0;
	}

	const Uint8 *pixel = (const Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
	Uint8 r, g, b, a;
	if (surface->format->palette) {
		SDL_Color &c = surface->format->palette->colors[*pixel];
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
	}else {
		SDL_GetRGBA(*reinterpret_cast<const Uint32*>(pixel), surface->format, &r, &g, &b, &a);
	}
	return (Uint32(r) << 24) + (Uint32(g) << 16) + (Uint32(b) << 8) + a;
}

void Utils::registerImage(Node *imageNode) {
	const std::string &desc = imageNode->params;
	std::string name;
	std::string path;

	size_t i = desc.find("=");
	if (i == size_t(-1)) {
		name = desc;
		path = "";
	}else {
		name = desc.substr(0, i - 1);
		path = desc.substr(i + 1);
	}

	auto clear = [](const std::string &str) -> std::string {
		size_t start = str.find_first_not_of(' ');
		size_t end = str.find_last_not_of(' ');
		if (start == size_t(-1) || end == size_t(-1)) {
			return "";
		}
		return str.substr(start, end - start + 1);
	};

	name = clear(name);
	if (name.empty()) {
		outMsg("Utils::registerImage",
		       "Empty name\n" +
		       imageNode->getPlace());
		return;
	}
	declAts[name] = imageNode;

	path = clear(path);

	if (path.empty()) {
		if (!imageNode->children.empty()) return;
	}else {
		Node *first = Node::getNewNode(imageNode->getFileName(), imageNode->getNumLine());
		first->params = path;

		if (imageNode->children.empty()) {
			imageNode->children.push_back(first);
		}else {
			imageNode->children.insert(imageNode->children.begin(), first);
			return;
		}
	}



	std::lock_guard g(PyUtils::pyExecMutex);

	PyObject *defaultDeclAt = PyDict_GetItemString(PyUtils::global, "default_decl_at");
	if (!defaultDeclAt) {
		outMsg("Utils::registerImage",
		       "default_decl_at not defined\n" +
		       imageNode->getPlace());
		return;
	}
	if (!PyTuple_CheckExact(defaultDeclAt) && !PyList_CheckExact(defaultDeclAt)) {
		outMsg("Utils::registerImage",
		       "default_decl_at is not list or tuple\n" +
		       imageNode->getPlace());
		return;
	}

	size_t sizeDefaultDeclAt = size_t(Py_SIZE(defaultDeclAt));
	for (size_t i = 0; i < sizeDefaultDeclAt; ++i) {
		PyObject *elem = PySequence_Fast_GET_ITEM(defaultDeclAt, i);
		if (!PyString_CheckExact(elem)) {
			outMsg("Utils::registerImage",
			       "default_decl_at[" + std::to_string(i) + "] is not str\n" +
			       imageNode->getPlace());
			continue;
		}

		Node *node = Node::getNewNode(imageNode->getFileName(), imageNode->getNumLine());
		node->params = PyString_AS_STRING(elem);
		imageNode->children.push_back(node);
	}
}

bool Utils::imageWasRegistered(const std::string &name) {
	return declAts.find(name) != declAts.end();
}
void Utils::clearImages() {
	declAts.clear();
}

PyObject* Utils::getImageDeclAt(const std::string &name) {
	auto it = declAts.find(name);
	if (it != declAts.end()) {
		const Node *node = it->second;
		return node->getPyList();
	}

	Utils::outMsg("Utils::getImageDeclAt", "Image <" + name + "> not registered");
	return PyList_New(0);
}
std::vector<std::string> Utils::getVectorImageDeclAt(const std::string &name) {
	auto it = declAts.find(name);
	if (it != declAts.end()) {
		const Node *node = it->second;
		return node->getImageChildren();
	}

	Utils::outMsg("Utils::getVectorImageDeclAt", "Image <" + name + "> not registered");
	return {};
}
