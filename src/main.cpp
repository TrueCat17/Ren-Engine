#include <iostream>

#include <SDL2/SDL.h>
#undef main //on Windows: int main(int argc, char **argv), but need: int main()

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "gv.h"
#include "config.h"

#include "gui/gui.h"
#include "gui/display_object.h"
#include "gui/screen/screen_child.h"
#include "gui/screen/screen_key.h"

#include "media/music.h"
#include "media/py_utils.h"

#include "parser/syntax_checker.h"

#include "utils/btn_rect.h"
#include "utils/game.h"
#include "utils/mouse.h"
#include "utils/utils.h"


SDL_DisplayMode displayMode;
SDL_Window *mainWindow;
SDL_Renderer *mainRenderer;


//before windowSize-changes
int startWindowWidth = 0;
int startWindowHeight = 0;

void changeWindowSize() {
	int w, h;
	SDL_GetWindowSize(GV::mainWindow, &w, &h);

	int dX = w - startWindowWidth;
	int dY = h - startWindowHeight;
	if (dX || dY) {
		SDL_Rect usableBounds;
		SDL_GetDisplayUsableBounds(0, &usableBounds);

		int wTop, wLeft, wBottom, wRight;
		SDL_GetWindowBordersSize(mainWindow, &wTop, &wLeft, &wBottom, &wRight);

		usableBounds.w -= wLeft + wRight;
		usableBounds.h -= wTop + wBottom;


		if (abs(dX) >= abs(dY)) {
			h = w * 9 / 16;
		}else {
			w = h * 16 / 9;
		}

		if (w < 640 || h < 360) {
			w = 640;
			h = 360;
		}
		if (w > usableBounds.w) {
			w = usableBounds.w;
			h = w * 9 / 16;
		}
		if (h > usableBounds.h) {
			h = usableBounds.h;
			w = h * 16 / 9;
		}

		SDL_SetWindowSize(GV::mainWindow, w, h);

		GV::width = w;
		GV::height = h;
		Config::set("window_width", w);
		Config::set("window_height", h);
	}

	startWindowWidth = 0;
	startWindowHeight = 0;
}



bool init() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		Utils::outMsg("SDL_Init", SDL_GetError());
		return true;
	}

	if (TTF_Init()) {
		Utils::outMsg("TTF_Init", TTF_GetError());
		return true;
	}

	if (SDL_GetDesktopDisplayMode(0, &displayMode)) {
		Utils::outMsg("SDL_GetDesktopDisplayMode", SDL_GetError());
		return true;
	}

	Utils::init();
	Music::init();
	SyntaxChecker::init();
	Mouse::init();

	Config::init();

	int x = Config::get("window_x").toInt();
	int y = Config::get("window_y").toInt();
	GV::width = Utils::inBounds(Config::get("window_width").toInt(), 640, 2400);
	GV::height = Utils::inBounds(Config::get("window_height").toInt(), 360, 1350);

	size_t fps = Config::get("max_fps").toInt();
	Game::setMaxFps(fps);

	String windowTitle = Config::get("window_title");

	int flags = SDL_WINDOW_RESIZABLE;
	mainWindow = GV::mainWindow = SDL_CreateWindow(windowTitle.c_str(), x, y, GV::width, GV::height, flags);
	if (!mainWindow) {
		Utils::outMsg("SDL_CreateWindow", SDL_GetError());
		return true;
	}
	changeWindowSize();

	SDL_Surface *icon = IMG_Load((Utils::ROOT + "images/misc/icon16.png").c_str());
	SDL_SetWindowIcon(mainWindow, icon);
	SDL_FreeSurface(icon);

	mainRenderer = GV::mainRenderer = SDL_CreateRenderer(mainWindow, -1, 0);
	if (!mainRenderer) {
		Utils::outMsg("SDL_CreateRenderer", SDL_GetError());
		return true;
	}

	return false;
}

void render() {
	GV::renderGuard.lock();

	SDL_SetRenderDrawColor(mainRenderer, 0, 0, 0, 0);
	SDL_RenderClear(mainRenderer);
	Group *screens = GV::screens;
	if (screens) {
		screens->draw();
	}
	SDL_RenderPresent(mainRenderer);

	GV::renderGuard.unlock();
}

void loop() {
	bool mouseOutPrevDown = false;

	while (!GV::exit) {
		GV::updateGuard.lock();

		int startTime = Utils::getTimer();

		bool resizeWithoutMouseDown = false;
		bool mouseWasDown = false;
		bool mouseWasUp = false;
		bool mouseOutDown = SDL_GetGlobalMouseState(0, 0);

		Mouse::update();
		BtnRect::checkMouseCursor();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				GV::exit = true;
				return;
			}else

			if (event.window.event == SDL_WINDOWEVENT_MOVED) {
				int x, y;
				SDL_GetWindowPosition(GV::mainWindow, &x, &y);
				if (x || y) {//Если оба равны 0, то скорее всего это глюк, игнорируем его
					int captionHeight;
					SDL_GetWindowBordersSize(mainWindow, &captionHeight, nullptr, nullptr, nullptr);
					y -= captionHeight;
					if (y < 1) {
						y = 1;
					}
					
					Config::set("window_x", x);
					Config::set("window_y", y);
				}
			}else

			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				if (!startWindowWidth && !startWindowHeight) {
					resizeWithoutMouseDown = true;
				}
			}else

			if (event.type == SDL_MOUSEBUTTONDOWN) {
				mouseWasDown = true;
				BtnRect::checkMouseClick();
			}else
			if (event.type == SDL_MOUSEBUTTONUP) {
				mouseWasUp = true;
			}else

			if (event.type == SDL_KEYDOWN) {
				if (!event.key.repeat) {
					Game::updateKeyboard();

					SDL_Scancode key = event.key.keysym.scancode;

					if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_SPACE) {
						if (BtnRect::checkMouseClick()) {
							ScreenKey::setToNotReact(key);
						}else {
							ScreenKey::setFirstDownState(key);
						}
					}else {
						ScreenKey::setFirstDownState(key);
					}
				}
			}else

			if (event.type == SDL_KEYUP) {
				SDL_Scancode key = event.key.keysym.scancode;

				Game::updateKeyboard();
				ScreenKey::setUpState(key);
			}
		}

		if (resizeWithoutMouseDown || !(mouseWasDown || mouseWasUp)) {
			if (resizeWithoutMouseDown ||
				(mouseOutPrevDown && !mouseOutDown && startWindowWidth && startWindowHeight)
			) {
				changeWindowSize();
			}
			if (mouseOutDown && !mouseOutPrevDown) {
				SDL_GetWindowSize(GV::mainWindow, &startWindowWidth, &startWindowHeight);
			}
		}
		mouseOutPrevDown = mouseOutDown;

		GUI::update();
		render();
		Config::save();

		PyUtils::exec("globals().has_key('persistent_save') and persistent_save()");

		int spent = Utils::getTimer() - startTime;
		int timeToSleep = Game::getFrameTime() - spent;
//		std::cout << spent << ' ' << timeToSleep << '\n';

		if (false) {
			auto screenObjects = ScreenChild::getScreenObjects();

			std::cout << "FPS: " << Game::getFps() << '\n';

			size_t count = std::count_if(screenObjects.begin(), screenObjects.end(), [](ScreenChild* scr) { return scr->enable; });
			std::cout << count << '/' << screenObjects.size() << '\n';

			std::map<String, int> m;
			for (ScreenChild *i : screenObjects) {
				auto c = i->getType();
				m[c] = m[c] + 1;
			}
			for (auto p : m) {
				std::cout << p.first << ": " << p.second << '\n';
			}
			std::cout << '\n';
		}

		GV::updateGuard.unlock();
		Utils::sleep(timeToSleep);
	}
}

void destroy() {
	GV::exitGuard.lock();

	SDL_CloseAudio();
	SDL_Quit();
}

int main() {
	setlocale(LC_ALL, "en_EN.utf8");

	if (init()) {
		return 0;
	}
	Game::startMod("main_menu");

	loop();
	destroy();

	std::cout << "\nOk!\n";
	return 0;
}
