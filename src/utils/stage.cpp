#include "stage.h"

#include <SDL2/SDL.h>

#include "config.h"
#include "gv.h"
#include "renderer.h"

#include "gui/group.h"

#include "media/py_utils.h"

#include "utils/math.h"
#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/utils.h"


int Stage::x, Stage::y, Stage::width, Stage::height;
bool Stage::fullscreen;
bool Stage::needResize = false;
bool Stage::minimized = false;
bool Stage::maximized = false;

SDL_Window *Stage::window = nullptr;
Group *Stage::screens = nullptr;


static SDL_DisplayMode getDisplayMode() {
	SDL_DisplayMode res;
	int index = 0;
	if (Stage::window) {
		index = SDL_GetWindowDisplayIndex(Stage::window);
		if (index < 0) {
			Utils::outMsg("SDL_GetWindowDisplayIndex", SDL_GetError());
			index = 0;
		}
	}
	if (SDL_GetDesktopDisplayMode(index, &res)) {
		Utils::outMsg("SDL_GetDesktopDisplayMode", SDL_GetError());
		res.w = 1920;
		res.h = 1080;
	}
	return res;
}

int Stage::getMaxWidth() {
	return getDisplayMode().w;
}
int Stage::getMaxHeight() {
	return getDisplayMode().h;
}

int Stage::getStageWidth() {
	return Stage::width;
}
int Stage::getStageHeight() {
	return Stage::height;
}


static SDL_Rect getUsableBounds() {
	SDL_Rect res;
	if (Stage::fullscreen) {
		res = {0, 0, Stage::getMaxWidth(), Stage::getMaxHeight()};
	}else {
		int index = SDL_GetWindowDisplayIndex(Stage::window);
		if (index < 0) {
			Utils::outMsg("SDL_GetWindowDisplayIndex", SDL_GetError());
			res = {0, 0, Stage::getMaxWidth(), Stage::getMaxHeight()};
		}else {
			int wTop, wLeft, wBottom, wRight;
			SDL_GetWindowBordersSize(Stage::window, &wTop, &wLeft, &wBottom, &wRight);

			SDL_GetDisplayUsableBounds(index, &res);
			res.w -= wLeft + wRight;
			res.h -= wTop + wBottom;
		}
	}
	return res;
}


static std::pair<int, int> fixWindowSize(int w, int h, int dX = 1, int dY = 1) {
	double k = String::toDouble(Config::get("window_w_div_h"));
	if (k < 0.1) {
		k = 1.777;
		Utils::outMsg("fixWindowSize",
		              "Invalid <window_w_div_h> in <params.conf>:\n"
		              "<" + Config::get("window_w_div_h") + ">");
	}

	if (Stage::maximized || Stage::fullscreen) {//can only scale down
		double curK = double(w) / h;
		if (curK > k + 0.01) {
			w = int(h * k);
		}else
		if (curK < k - 0.01) {
			h = int(w / k);
		}
	}else {
		if (std::abs(dX) >= std::abs(dY)) {
			h = int(w / k);
		}else {
			w = int(h * k);
		}

		if (w < Stage::MIN_WIDTH || h < Stage::MIN_WIDTH / k) {
			w = Stage::MIN_WIDTH;
			h = int(Stage::MIN_WIDTH / k);
		}

		SDL_Rect usableBounds = getUsableBounds();
		if (w > usableBounds.w) {
			w = usableBounds.w;
			h = int(w / k);
		}
		if (h > usableBounds.h) {
			h = usableBounds.h;
			w = int(h * k);
		}
	}

	return {w, h};
}


static std::mutex stageMutex;
static int stageWidth = 0;
static int stageHeight = 0;
static bool stageFullscreen = false;
static bool stageApplied = true;

void Stage::applyChanges() {
	if (stageApplied) return;

	std::lock_guard g(Renderer::renderMutex);

	if (Stage::fullscreen != stageFullscreen) {//need only change fullscreen mode?
		Stage::fullscreen = stageFullscreen;
		Config::set("window_fullscreen", stageFullscreen ? "True" : "False");
		SDL_SetWindowFullscreen(Stage::window, SDL_WINDOW_FULLSCREEN_DESKTOP * Stage::fullscreen);
		if (Stage::fullscreen) {
			SDL_GetWindowSize(Stage::window, &Stage::width, &Stage::height);
		}else {
			Stage::maximized = false;
			Stage::width = Math::inBounds(String::toInt(Config::get("window_width")), Stage::MIN_WIDTH, Stage::getMaxWidth());
			Stage::height = Math::inBounds(String::toInt(Config::get("window_height")), Stage::MIN_HEIGHT, Stage::getMaxHeight());
		}

		auto size = fixWindowSize(Stage::width, Stage::height);
		Stage::width = size.first;
		Stage::height = size.second;
	}else {                                    //need change window size
		if (GV::inGame && Stage::fullscreen) {
			Stage::fullscreen = false;
			Config::set("window_fullscreen", "False");

			SDL_SetWindowFullscreen(Stage::window, 0);
		}
		Stage::width = stageWidth;
		Stage::height = stageHeight;
	}

	Stage::x = 0;
	Stage::y = 0;
	if (Stage::maximized || Stage::fullscreen) {
		int windowW, windowH;
		SDL_GetWindowSize(Stage::window, &windowW, &windowH);

		Stage::x = std::max(0, windowW - Stage::width) / 2;
		Stage::y = std::max(0, windowH - Stage::height) / 2;
	}

	if (Stage::screens) {
		Stage::screens->setX(float(Stage::x));
		Stage::screens->setY(float(Stage::y));
		Stage::screens->setWidth(float(Stage::width));
		Stage::screens->setHeight(float(Stage::height));
		Stage::screens->updateGlobal();
	}

	if (!Stage::fullscreen) {
		Config::set("window_width", std::to_string(Stage::width));
		Config::set("window_height", std::to_string(Stage::height));
		SDL_SetWindowSize(Stage::window, Stage::width + Stage::x * 2, Stage::height + Stage::y * 2);
	}

	stageApplied = true;
	Renderer::needToUpdateViewPort = true;
}


static void wait() {
	stageApplied = false;

	if (GV::messageThreadId == std::this_thread::get_id()) {
		Stage::applyChanges();
	}else {
		while (!stageApplied) {
			Utils::sleep(0.001);
		}
	}

	if (GV::inGame) {
		PyUtils::exec("CPP_EMBED: stage.cpp", __LINE__, "if 'signals' in globals(): signals.send('resized_stage')");
	}
}

void Stage::setStageSize(int width, int height) {
	if (width != Stage::width || height != Stage::height) {
		SDL_RestoreWindow(Stage::window);
		Stage::maximized = false;
		Stage::changeWindowSize(width, height);
	}
}

void Stage::setFullscreen(bool value) {
	if (value == Stage::fullscreen) return;

	std::lock_guard g(stageMutex);
	stageFullscreen = value;
	Stage::maximized = false;
	wait();
}


static std::tuple<int, int, int, int> prevSizes = {0, 0, 0, 0};
void Stage::changeWindowSize(int startW, int startH, int prevWindowWidth, int prevWindowHeight) {
	ScopeExit se([]() {
		Stage::needResize = false;
	});

	if (startW <= 0 || startH <= 0) {
		SDL_GetWindowSize(Stage::window, &startW, &startH);
	}
	if (prevWindowWidth <= 0 || prevWindowHeight <= 0) {
		prevWindowWidth = Stage::width;
		prevWindowHeight = Stage::height;
	}
	std::tuple<int, int, int, int> sizes = {startW, startH, prevWindowWidth, prevWindowHeight};
	if (prevSizes == sizes) return;
	prevSizes = sizes;

	int dX = startW - prevWindowWidth;
	int dY = startH - prevWindowHeight;
	if (!dX && !dY && !Stage::needResize) return;

	auto [w, h] = fixWindowSize(startW, startH, dX, dY);
	if (w != startW || w != Stage::width || h != startH || h != Stage::height || Stage::needResize) {
		std::lock_guard g(stageMutex);
		stageWidth = w;
		stageHeight = h;
		stageFullscreen = Stage::fullscreen;
		wait();
	}
}
