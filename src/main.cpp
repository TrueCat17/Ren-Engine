#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "gv.h"
#include "config.h"

#include "gui/gui.h"
#include "gui/display_object.h"
#include "gui/screen/screen_child.h"

#include "utils/btn_rect.h"
#include "utils/game.h"
#include "utils/mouse.h"
#include "utils/syntax_checker.h"
#include "utils/utils.h"

#include "parser/py_guard.h"

SDL_DisplayMode displayMode;
SDL_Window *mainWindow;
SDL_Renderer *mainRenderer;

bool init() {
	Utils::init();
	SyntaxChecker::init();
	Config::init();

	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		Utils::outMsg("SDL_Init", SDL_GetError());
		return true;
	}

	Mouse::init();

	if (TTF_Init()) {
		Utils::outMsg("TTF_Init", TTF_GetError());
		return true;
	}

	int mixInitFlags = MIX_INIT_OGG | MIX_INIT_MP3;
	if ((Mix_Init(mixInitFlags) & mixInitFlags) != mixInitFlags) {
		Utils::outMsg("Mix_Init", Mix_GetError());
		return true;
	}
	if (Mix_SetSoundFonts((Utils::ROOT + "sounds/TimGM6mb.sf2").c_str()) == -1) {
		Utils::outMsg("Mix_SetSoundFonts", Mix_GetError());
		return true;
	}
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
		Utils::outMsg("Mix_OpenAudio", Mix_GetError());
		return true;
	}

	if (SDL_GetDesktopDisplayMode(0, &displayMode)) {
		Utils::outMsg("SDL_GetDesktopDisplayMode", SDL_GetError());
		return true;
	}

	int x = Config::get("window_x").toInt();
	int y = Config::get("window_y").toInt();
	GV::width = Utils::inBounds(Config::get("window_width").toInt(), 320, 2000);
	GV::height = Utils::inBounds(Config::get("window_height").toInt(), 240, 1500);

	size_t fps = Config::get("fps").toInt();
	Game::setFps(fps);

	int flags = SDL_WINDOW_RESIZABLE;
	if (Config::get("window_fullscreen") == "True") {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	mainWindow = GV::mainWindow = SDL_CreateWindow("ES 2D", x, y, GV::width, GV::height, flags);
	if (!mainWindow) {
		Utils::outMsg("SDL_CreateWindow", SDL_GetError());
		return true;
	}

	SDL_Surface *icon = IMG_Load((Utils::ROOT + "images/misc/icon16.png").c_str());
	SDL_SetWindowIcon(mainWindow, icon);
	SDL_FreeSurface(icon);

	mainRenderer = GV::mainRenderer = SDL_CreateRenderer(mainWindow, -1, 0);
	if (!mainRenderer) {
		Utils::outMsg("SDL_CreateRenderer", SDL_GetError());
		return true;
	}

	GV::stage = new Group();
	GV::screens = new Group();
	GV::stage->addChild(GV::screens);

	return false;
}

void render() {
	GV::renderGuard.lock();

	SDL_SetRenderDrawColor(mainRenderer, 0, 0, 0, 0);
	SDL_RenderClear(mainRenderer);
	if (GV::stage) {
		GV::stage->draw();
	}
	SDL_RenderPresent(mainRenderer);

	GV::renderGuard.unlock();
}

void loop() {
	while (!GV::exit) {
		int startTime = Utils::getTimer();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				GV::exit = true;
				return;
			}else

			if (event.window.event == SDL_WINDOWEVENT_MOVED || event.window.event == SDL_WINDOWEVENT_RESIZED) {
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


				static int newW = GV::width;
				static int newH = GV::height;

				int oldW = newW;
				int oldH = newH;

				SDL_GetWindowSize(GV::mainWindow, &newW, &newH);

				int dW = newW - oldW;
				int dH = newH - oldH;

				if (dW * dW + dH * dH > 10) {
					newH = newW / double(GV::STD_WIDTH) * GV::STD_HEIGHT;
					GV::width = newW;
					GV::height = newH;
					SDL_SetWindowSize(mainWindow, GV::width, GV::height);

					Config::set("window_width", GV::width);
					Config::set("window_height", GV::height);
				}
			}else

			if (event.type == SDL_MOUSEBUTTONDOWN) {
				BtnRect::checkMouseClick();
			}else

			if (event.type == SDL_KEYDOWN) {
				SDL_Keycode key = event.key.keysym.sym;
				if (key == SDLK_RETURN || key == SDLK_SPACE) {
					BtnRect::checkMouseClick();
				}
			}
		}
		Game::updateKeyboard();
		BtnRect::checkMouseCursor();

		GUI::update();
		render();
		Config::save();

		size_t spent = Utils::getTimer() - startTime;
		size_t timeToSleep = Game::getFrameTime() - spent;
//		std::cout << spent << ' ' << timeToSleep << '\n';

		Utils::sleep(timeToSleep);
	}
}

void destroy() {
	GV::inGame = false;
	int toSleep = Game::getFrameTime() * 2;
	Utils::sleep(toSleep);

	DisplayObject::destroyAll();
	Utils::destroyAllTextures();
	Utils::destroyAllSurfaces();
	Utils::destroyAllFonts();

	Mouse::quit();
	TTF_Quit();
	Mix_CloseAudio();
	Mix_Quit();

	SDL_DestroyRenderer(mainRenderer);
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();

	delete GV::pyGuard;
}

int main(int argc, char **argv) {
	//Чтобы не было варнингов
	argc += 1;
	argv += 1;

	if (init()) {
		return 0;
	}

	Game::startMod("main_menu");

	loop();
	destroy();

	std::cout << "\nOk!\n";
	return 0;
}
