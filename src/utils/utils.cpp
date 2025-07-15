#include "utils.h"

#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <set>
#include <pthread.h>

extern "C" {
#include <libavutil/md5.h>
}

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_clipboard.h>

#include "gv.h"
#include "logger.h"

#include "gui/screen/key.h"

#include "utils/algo.h"
#include "utils/game.h"
#include "utils/stage.h"
#include "utils/string.h"


static std::string versionStr;
std::string Utils::getVersion() {
	if (versionStr.empty()) {
		std::ifstream is("../Ren-Engine/version", std::ios_base::binary);
		while (is.is_open() && !is.eof()) {
			std::getline(is, versionStr);
			if (!versionStr.empty()) break;
		}

		if (versionStr.empty()) {
			versionStr = "None";
		}
	}
	return versionStr;
}

void Utils::setThreadName([[maybe_unused]] std::string name) {
#ifndef __WIN32__ //suspend on wine
	size_t maxNameSize = 14;//doc. say "16 with null terminator" (15), but this does not work

	if (name.empty()) {
		name = "empty_name";
	}

	if (name.size() > maxNameSize) {
		std::string ending = "..";
		size_t i = String::getCorrectCropIndex(name, maxNameSize - ending.size());
		while (i > 0 && name[i - 1] == ' ') {
			--i;
		}
		name.erase(i);
		name += ending;
	}

	if (pthread_setname_np(pthread_self(), name.c_str())) {
		outError("pthread_setname_np", "Error on set process name <%>", name);
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

static std::vector<Message> messagesToOut;
static bool msgCloseAll = false;
static std::mutex msgGuard;

bool Utils::realOutMsg() {
	if (messagesToOut.empty() || msgCloseAll) return false;

	Message msg;
	{
		std::lock_guard g(msgGuard);
		msg = messagesToOut.front();
		messagesToOut.erase(messagesToOut.begin());
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

	Key::resetPressed();

	int res = 0;
	if (SDL_ShowMessageBox(&data, &res)) {
		std::cout << msg.str << '\n';
		std::cout << SDL_GetError() << '\n';
	}

	Key::resetPressed();

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
		}

		const size_t maxLineSize = 100;
		const size_t maxLineCount = 30;
		std::vector<std::string> lines = String::split(msg, "\n");
		for (size_t i = 0; i < lines.size() && i < maxLineCount; ++i) {
			std::string &line = lines[i];
			if (line.size() <= maxLineSize) continue;

			size_t space = line.find_last_of(' ', maxLineSize - 1);
			if (space <= maxLineSize / 2 || space == size_t(-1)) {//small string or no spaces?
				space = maxLineSize;//word-wrap in the middle of word
			}

			lines.insert(lines.begin() + long(i) + 1, line.substr(space));
			lines[i].erase(space);//no just "line" (vector mb reallocated on prev line)
		}
		if (lines.size() > maxLineCount) {
			lines.erase(lines.begin() + maxLineCount - 1, lines.end());
			lines.push_back("..");
		}
		msg = String::join(lines, "\n");

		static std::set<std::string> msgErrors;
		if (msg.empty() || msgErrors.count(msg)) return;

		Logger::log(msg + "\n\n");

		msgErrors.insert(msg);
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

std::string Utils::formatImpl(std::string_view format, const std::string *strs, size_t n) {
	std::string msg;
	size_t msgSize = format.size() - n;
	for (size_t i = 0; i < n; ++i) {
		msgSize += strs[i].size();
	}
	msg.reserve(msgSize);

	size_t start = 0;
	size_t index = format.find('%');
	size_t strIndex = 0;
	while (index != size_t(-1)) {
		msg += format.substr(start, index - start);
		start = index + 1;
		index = format.find('%', start);

		if (strIndex < n) {
			msg += strs[strIndex++];
		}else {
			Utils::outMsg("Utils::formatImpl", "Need more args to format msg");
			break;
		}
	}
	msg += format.substr(start);

	if (strIndex != n) {
		Utils::outMsg("Utils::formatImpl", "Need more placeholders to format msg");
		msg += "\n(";
		while (strIndex < n) {
			msg += strs[strIndex++];
			if (strIndex != n) {
				msg += ", ";
			}
		}
		msg += ')';
	}

	return msg;
}


Uint32 Utils::getPixel(const SurfacePtr &surface, const SDL_Rect &draw, const SDL_Rect &crop) {
	if (!surface) {
		Utils::outMsg("Utils::getPixel", "surface == nullptr");
		return 0;
	}

	int x = crop.x + draw.x * crop.w / draw.w;
	int y = crop.y + draw.y * crop.h / draw.h;

	if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
		Utils::outError("Utils::getPixel",
		                "Point (%, %) is invalid for image with size %x%",
		                x, y, surface->w, surface->h);
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


static std::mutex md5_mutex;
static const char *md5_table16 = "0123456789abcdef";
std::string Utils::md5(const std::string &str) {
	std::lock_guard g(md5_mutex);

	uint8_t data[16];
	av_md5_sum(data, reinterpret_cast<const uint8_t*>(str.c_str()), str.size());

	std::string res;
	res.resize(32);
	for (size_t i = 0; i < 16; ++i) {
		uint8_t v = data[i];
		res[i * 2 + 0] = md5_table16[v >> 4];
		res[i * 2 + 1] = md5_table16[v & 15];
	}
	return res;
}
