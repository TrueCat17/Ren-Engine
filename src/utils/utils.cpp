#include "utils.h"

#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <pthread.h>

extern "C" {
#include <libavutil/md5.h>
}

#include "utils/game.h"
#include "utils/message.h"
#include "utils/string.h"


static std::string versionStr;
const std::string& Utils::getVersion() {
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
#ifndef __CYGWIN__ //suspend on wine
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
		Message::outError("pthread_setname_np", "Error on set process name <%>", name);
	}
#endif
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


Uint32 Utils::getPixel(const SurfacePtr &surface, const SDL_Rect &draw, const SDL_Rect &crop) {
	if (!surface) {
		Message::outMsg("Utils::getPixel", "surface == nullptr");
		return 0;
	}

	int x = crop.x + draw.x * crop.w / draw.w;
	int y = crop.y + draw.y * crop.h / draw.h;

	if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
		Message::outError("Utils::getPixel",
		                  "Point (%, %) is invalid for image with size %x%",
		                  x, y, surface->w, surface->h);
		return 0;
	}

	Uint8 r, g, b, a;
	if (!SDL_ReadSurfacePixel(surface.get(), x, y, &r, &g, &b, &a)) {
		Message::outMsg("Utils::getPixel, SDL_ReadSurfacePixel", SDL_GetError());
		return 0;
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
