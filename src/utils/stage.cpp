#include "stage.h"

#include "config.h"
#include "renderer.h"

#include "gui/group.h"

#include "media/py_utils.h"

#include "utils/math.h"
#include "utils/message.h"
#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/thread_tasks.h"


int Stage::x, Stage::y, Stage::width, Stage::height;
bool Stage::fullscreen;
bool Stage::needResize = false;
bool Stage::minimized = false;
bool Stage::maximized = false;

SDL_Window *Stage::window = nullptr;
Group *Stage::screens = nullptr;


static SDL_DisplayMode displayModeStruct;

void Stage::updateDisplayMode() {
	displayModeStruct.w = 1920;
	displayModeStruct.h = 1080;

	SDL_DisplayID index = 0;
	if (Stage::window) {
		index = SDL_GetDisplayForWindow(Stage::window);
		if (!index) {
			Message::outMsg("SDL_GetDisplayForWindow", SDL_GetError());
			return;
		}
	}else {
		const SDL_DisplayID *firstDisplayId = SDL_GetDisplays(nullptr);
		if (!firstDisplayId) {
			Message::outMsg("SDL_GetDisplays", SDL_GetError());
			return;
		}
		index = *firstDisplayId;
	}

	const SDL_DisplayMode *dm = SDL_GetDesktopDisplayMode(index);
	if (!dm) {
		Message::outMsg("SDL_GetDesktopDisplayMode", SDL_GetError());
	}else {
		displayModeStruct = *dm;
	}
}

int Stage::getMaxWidth() {
	return displayModeStruct.w;
}
int Stage::getMaxHeight() {
	return displayModeStruct.h;
}

int Stage::getStageWidth() {
	return Stage::width;
}
int Stage::getStageHeight() {
	return Stage::height;
}


static void getUsableBounds(int &usableWidth, int &usableHeight) {
	usableWidth = Stage::getMaxWidth();
	usableHeight = Stage::getStageHeight();

	if (!Stage::fullscreen) {
		SDL_DisplayID index = SDL_GetDisplayForWindow(Stage::window);
		if (!index) {
			Message::outMsg("SDL_GetDisplayForWindow", SDL_GetError());
			return;
		}

		int wTop, wLeft, wBottom, wRight;
		SDL_GetWindowBordersSize(Stage::window, &wTop, &wLeft, &wBottom, &wRight);

		SDL_Rect res;
		SDL_GetDisplayUsableBounds(index, &res);
		usableWidth  = res.w - (wLeft + wRight);
		usableHeight = res.h - (wTop + wBottom);
	}
}


static std::pair<int, int> fixWindowSize(int w, int h, int dX = 1, int dY = 1) {
	double k = String::toDouble(Config::get("window_w_div_h"));
	if (k < 0.1) {
		k = 1.777;
		Message::outError("fixWindowSize",
		                  "Invalid <window_w_div_h> in <params.conf>: <%>",
		                  Config::get("window_w_div_h"));
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

		int usableWidth, usableHeight;
		getUsableBounds(usableWidth, usableHeight);
		if (w > usableWidth) {
			w = usableWidth;
			h = int(w / k);
		}
		if (h > usableHeight) {
			h = usableHeight;
			w = int(h * k);
		}
	}

	return {w, h};
}


static void onResize() {
	Stage::x = 0;
	Stage::y = 0;
	if (Stage::maximized || Stage::fullscreen) {
		int w, h;
		SDL_GetWindowSize(Stage::window, &w, &h);

		Stage::x = std::max(0, w - Stage::width) / 2;
		Stage::y = std::max(0, h - Stage::height) / 2;
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

	Renderer::needToUpdateViewPort = true;

	if (GV::inGame) {
		pyExecFromCpp("signals.send('resized_stage')");
	}
}


void Stage::setStageSize(int width, int height) {
	if (width >= displayModeStruct.w || height >= displayModeStruct.h) {
		Stage::setFullscreen(true);
		return;
	}

	if (width != Stage::width || height != Stage::height) {
		ThreadTasks::main.addAndWait([=]() {
			SDL_RestoreWindow(Stage::window);
			Stage::maximized = false;
			Stage::changeWindowSize(width, height);
		});
	}
}

void Stage::setFullscreen(bool value) {
	if (value == Stage::fullscreen) return;

	ThreadTasks::main.addAndWait([=]() {
		Stage::fullscreen = value;
		Config::set("window_fullscreen", value ? "True" : "False");

		SDL_SetWindowFullscreen(Stage::window, value);

		if (value) {
			SDL_GetWindowSize(Stage::window, &Stage::width, &Stage::height);
		}else {
			Stage::maximized = false;

			int w = String::toInt(Config::get("window_width"));
			int h = String::toInt(Config::get("window_height"));

			Stage::width  = Math::inBounds(w, Stage::MIN_WIDTH,  Stage::getMaxWidth());
			Stage::height = Math::inBounds(h, Stage::MIN_HEIGHT, Stage::getMaxHeight());
		}

		const auto size = fixWindowSize(Stage::width, Stage::height);
		Stage::width = size.first;
		Stage::height = size.second;

		onResize();
	});
}


static std::tuple<int, int, int, int> prevSizes = { 0, 0, 0, 0 };
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
	std::tuple<int, int, int, int> sizes = { startW, startH, prevWindowWidth, prevWindowHeight };
	if (prevSizes == sizes) return;
	prevSizes = sizes;

	int dX = startW - prevWindowWidth;
	int dY = startH - prevWindowHeight;
	if (!dX && !dY && !Stage::needResize) return;

	const auto [w, h] = fixWindowSize(startW, startH, dX, dY);

	if (w != startW || w != Stage::width || h != startH || h != Stage::height || Stage::needResize) {
		if (GV::inGame && Stage::fullscreen) {
			Stage::fullscreen = false;
			Config::set("window_fullscreen", "False");

			SDL_SetWindowFullscreen(Stage::window, false);
		}
		Stage::width = w;
		Stage::height = h;

		onResize();
	}
}
