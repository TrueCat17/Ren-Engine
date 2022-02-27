#include "utils.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <set>
#include <list>
#include <map>
#include <pthread.h>

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_clipboard.h>

#include "gv.h"
#include "logger.h"

#include "media/image_manipulator.h"
#include "media/py_utils.h"

#include "parser/node.h"

#include "utils/algo.h"
#include "utils/file_system.h"
#include "utils/game.h"
#include "utils/stage.h"
#include "utils/string.h"


static std::map<std::string, Node*> declAts;


void Utils::setThreadName(std::string name) {
#ifndef __WIN32__ //suspend on wine
	size_t maxNameSize = 14;//if more - no change in task manager

	if (name.empty()) {
		name = "empty_name";
	}else
	if (name.size() > maxNameSize) {
		std::string ending = "..";
		name.erase(maxNameSize - ending.size());

		if (!Algo::isFirstByte(name.back())) {
			while (!Algo::isFirstByte(name.back())) {
				name.pop_back();
			}
			name.pop_back();
		}
		while (name.back() == ' ') {
			name.pop_back();
		}
		name += ending;
	}

	if (pthread_setname_np(pthread_self(), name.c_str())) {
		outMsg("pthread_setname_np", "Error on set process name <" + name + ">");
	}
#endif
}

std::string Utils::getClipboardText() {
	const char *tmp = SDL_GetClipboardText();
	std::string res = tmp;
	SDL_free((void*)tmp);
	if (res.empty()) {
		std::string error = SDL_GetError();
		if (!error.empty()) {
			Utils::outMsg("Utils::getClipboardText", error);
		}
	}
	return res;
}

bool Utils::setClipboardText(const std::string &text) {
	if (SDL_SetClipboardText(text.c_str()) < 0) {
		Utils::outMsg("Utils::setClipboardText", SDL_GetError());
		return false;
	}
	return true;
}

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

double Utils::getTimer() {
	static auto startTime = std::chrono::system_clock::now();

	auto now = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - startTime);
	return double(duration.count()) / 1e9;
}
void Utils::sleep(double sec, bool checkInGame) {
	const double MIN_SLEEP = Game::getFrameTime();

	while ((GV::inGame || !checkInGame) && sec >= MIN_SLEEP) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(size_t(MIN_SLEEP * 1e9)));
		sec -= MIN_SLEEP;
	}
	if (sec <= 0) return;

	if (GV::inGame || !checkInGame) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(size_t(sec * 1e9)));
	}
}


struct Message {
	uint8_t id;
	bool isError;

	bool _padding[sizeof(void*)-2];
	std::string str;

	bool operator==(const Message& msg) const { return id == msg.id; }
};

static std::list<Message> messagesToOut;
static bool msgCloseAll = false;
static std::mutex msgGuard;

bool Utils::realOutMsg() {
	if (messagesToOut.empty() || msgCloseAll) return false;

	Message msg;
	{
		std::lock_guard g(msgGuard);
		msg = messagesToOut.front();
		messagesToOut.pop_front();
	}

	static const SDL_MessageBoxButtonData buttons[] = {
	    {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Ok"},
	    {0, 1, "Close All"}
	};

	static SDL_MessageBoxData data;
	data.flags = msg.isError ? SDL_MESSAGEBOX_ERROR : SDL_MESSAGEBOX_WARNING;
	data.window = GV::messageThreadId == std::this_thread::get_id() ? Stage::window : nullptr;
	data.title = "Message";
	data.message = msg.str.c_str();
	data.numbuttons = 2;
	data.buttons = buttons;
	data.colorScheme = nullptr;

	int res = 0;
	if (SDL_ShowMessageBox(&data, &res)) {
		std::cout << msg.str << '\n';
		std::cout << SDL_GetError() << '\n';
	}

	std::lock_guard g(msgGuard);
	if (res == 1) {
		msgCloseAll = true;
		messagesToOut.clear();
	}

	return !messagesToOut.empty();
}

void Utils::outMsg(std::string msg, const std::string &err) {
	{
		std::lock_guard g(msgGuard);

		if (!err.empty()) {
			msg += " Error:\n" + err;

			static std::set<std::string> msgErrors;
			if (msgErrors.count(msg)) return;
			msgErrors.insert(msg);
		}
		if (msg.empty()) return;

		Logger::log(msg + "\n\n");
	}

	if (msgCloseAll) return;

	static uint8_t nextId = 0;
	Message message;
	message.id = nextId++;
	message.isError = !err.empty();
	message.str = msg;
	{
		std::lock_guard g(msgGuard);
		messagesToOut.push_back(message);
	}

	if (GV::messageThreadId == std::this_thread::get_id() || GV::messageThreadId == std::thread::id()) {
		while (Utils::realOutMsg()) {}
	}else {
		while (true) {
			sleep(0.010, false);

			std::lock_guard g(msgGuard);
			if (!Algo::in(message, messagesToOut)) break;
		}
	}
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
