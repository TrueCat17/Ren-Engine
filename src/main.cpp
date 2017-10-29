#include <iostream>
#include <thread>

#include <SDL2/SDL.h>
#undef main //on Windows: int main(int argc, char **argv), but need: int main()

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "gv.h"
#include "config.h"
#include "logger.h"
#include "renderer.h"

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



//before windowSize-changes
int startWindowWidth = 0;
int startWindowHeight = 0;

void changeWindowSize(bool maximized) {
	int startW, startH;
	SDL_GetWindowSize(GV::mainWindow, &startW, &startH);

	int dX = startW - startWindowWidth;
	int dY = startH - startWindowHeight;
	if (dX || dY) {
		int w = startW;
		int h = startH;


		if (maximized) {//Можно только уменьшать, увеличивать нельзя
			if (double(w) / h > 16.0 / 9) {
				w = h * 16 / 9;
			}else {
				h = w * 9 / 16;
			}
		}else {
			SDL_Rect usableBounds;
			SDL_GetDisplayUsableBounds(0, &usableBounds);

			int wTop, wLeft, wBottom, wRight;
			SDL_GetWindowBordersSize(GV::mainWindow, &wTop, &wLeft, &wBottom, &wRight);

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
		}

		if (GV::screens) {
			int x, y;
			if (maximized) {
				x = (startW - w) / 2;
				y = (startH - h) / 2;
			}else {
				x = y = 0;
			}

			if (x + w != startW || y + h != startH) {
				std::lock_guard<std::mutex> g1(Renderer::toRenderMutex);
				std::lock_guard<std::mutex> g2(Renderer::renderMutex);
				SDL_SetWindowSize(GV::mainWindow, x + w, y + h);
			}

			GV::screens->setPos(x, y);
			GV::screens->updateGlobalPos();
		}

		GV::width = w;
		GV::height = h;
		Config::set("window_width", w);
		Config::set("window_height", h);
	}

	startWindowWidth = 0;
	startWindowHeight = 0;
}



bool init() {
	Utils::init();
	Logger::init();
	Config::init();

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		Utils::outMsg("SDL_Init", SDL_GetError());
		return true;
	}

	String scaleQuality = Config::get("scale_quality");
	if (scaleQuality != "0" && scaleQuality != "1" && scaleQuality != "2") {
		scaleQuality = "0";
		Utils::outMsg("Config::get",
					  "Значением параметра scale_quality ожидалось 0, 1 или 2, получено: <" + scaleQuality + ">");
	}
	if (scaleQuality != "0") {
		if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scaleQuality.c_str()) == SDL_FALSE) {
			Utils::outMsg("SDL_SetHint", "Не удалось настроить сглаживание");
		}
	}

	if (TTF_Init()) {
		Utils::outMsg("TTF_Init", TTF_GetError());
		return true;
	}

	if (SDL_GetDesktopDisplayMode(0, &GV::displayMode)) {
		Utils::outMsg("SDL_GetDesktopDisplayMode", SDL_GetError());
		return true;
	}

	Mouse::init();
	Music::init();
	SyntaxChecker::init();


	size_t fps = Config::get("max_fps").toInt();
	Game::setMaxFps(fps);


	GV::fullscreen = Config::get("window_fullscreen") == "True";

	int x = Config::get("window_x").toInt();
	int y = Config::get("window_y").toInt();

	int flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
	if (GV::fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		GV::width = GV::displayMode.w;
		GV::height = GV::displayMode.h;
	}else {
		GV::width = Utils::inBounds(Config::get("window_width").toInt(), 640, 2400);
		GV::height = Utils::inBounds(Config::get("window_height").toInt(), 360, 1350);
	}

	String windowTitle = Config::get("window_title");
	GV::mainWindow = SDL_CreateWindow(windowTitle.c_str(), x, y, GV::width, GV::height, flags);
	if (!GV::mainWindow) {
		Utils::outMsg("SDL_CreateWindow", SDL_GetError());
		return true;
	}
	if (!GV::fullscreen) {
		changeWindowSize(false);
	}

	String iconPath = Config::get("window_icon");
	if (iconPath && iconPath != "None") {
		SurfacePtr icon = Utils::getSurface(iconPath);
		if (icon) {
			SDL_SetWindowIcon(GV::mainWindow, icon.get());
		}
	}


	int renderDriver = -1;
	if (Config::get("software_renderer") == "True") {
		flags = SDL_RENDERER_SOFTWARE;
	}else {
		flags = SDL_RENDERER_ACCELERATED;

		int countRenderDrivers = SDL_GetNumRenderDrivers();
		for (int i = 0; i < countRenderDrivers; ++i) {
			SDL_RendererInfo info;
			SDL_GetRenderDriverInfo(i, &info);
			if (String(info.name) == "opengl") {
				renderDriver = i;
				GV::isOpenGL = true;
				break;
			}
		}
		if (renderDriver == -1) {
			Utils::outMsg("init", "OpenGL driver not found");
		}
	}
	GV::mainRenderer = SDL_CreateRenderer(GV::mainWindow, renderDriver, flags);
	if (!GV::mainRenderer) {
		Utils::outMsg("SDL_CreateRenderer", SDL_GetError());
		return true;
	}

	return false;
}


void loop() {
	std::thread(Renderer::renderThreadFunc).detach();

	bool maximazed = false;
	bool mouseOut = false;
	bool mouseOutPrevDown = false;

	while (!GV::exit) {
		while (!GV::inGame) {
			Utils::sleep(10, false);
		}

		GV::updateMutex.lock();

		int startTime = Utils::getTimer();

		bool resizeWithoutMouseDown = false;
		bool mouseWasDown = false;
		bool mouseWasUp = false;

		bool mouseOutDown = false;


		Mouse::update();
		BtnRect::checkMouseCursor();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				GV::exit = true;
				return;
			}

			if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
				Renderer::needToRedraw = true;
			}else
			if (event.window.event == SDL_WINDOWEVENT_ENTER) {
				mouseOut = false;
			}else
			if (event.window.event == SDL_WINDOWEVENT_LEAVE) {
				mouseOut = true;
			}else

			if (event.window.event == SDL_WINDOWEVENT_MOVED) {
				int x, y;
				SDL_GetWindowPosition(GV::mainWindow, &x, &y);
				if (x || y) {//Если оба равны 0, то скорее всего это глюк, игнорируем его
					int leftBorderSize;
					int captionHeight;
					SDL_GetWindowBordersSize(GV::mainWindow, &captionHeight, &leftBorderSize, nullptr, nullptr);

					x = std::max(x - leftBorderSize, 1);
					y = std::max(y - captionHeight, 1);

					Config::set("window_x", x);
					Config::set("window_y", y);
				}
			}else

			if (event.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
				maximazed = true;
			}else
			if (event.window.event == SDL_WINDOWEVENT_RESTORED) {
				maximazed = false;
			}else

			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				if (!startWindowWidth && !startWindowHeight) {
					resizeWithoutMouseDown = true;
				}
			}else

			if (event.type == SDL_MOUSEBUTTONDOWN) {
				bool left = event.button.button == SDL_BUTTON_LEFT;
				bool right = event.button.button == SDL_BUTTON_RIGHT;
				if (left || right) {
					if (left) {
						mouseWasDown = true;
					}
					BtnRect::checkMouseClick(left);
				}
			}else
			if (event.type == SDL_MOUSEBUTTONUP) {
				mouseWasUp = true;
				Mouse::setMouseDown(false);
			}else

			if (event.type == SDL_KEYDOWN) {
				if (!event.key.repeat) {
					Game::updateKeyboard();

					SDL_Scancode key = event.key.keysym.scancode;

					if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_SPACE) {
						if (BtnRect::checkMouseClick(true, true)) {
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

		if (mouseOut) {
			mouseOutDown = SDL_GetGlobalMouseState(nullptr, nullptr);
		}

		if (resizeWithoutMouseDown || !(mouseWasDown || mouseWasUp)) {
			if (resizeWithoutMouseDown ||
				(mouseOutPrevDown && !mouseOutDown && startWindowWidth && startWindowHeight)
			) {
				changeWindowSize(maximazed);
			}
			if (mouseOutDown && !mouseOutPrevDown) {
				SDL_GetWindowSize(GV::mainWindow, &startWindowWidth, &startWindowHeight);
			}
		}
		mouseOutPrevDown = mouseOutDown;

		GUI::update();

		{
			while (Renderer::needToRender) {
				Utils::sleep(1);
			}
			std::lock_guard<std::mutex> g(Renderer::toRenderMutex);
			Renderer::toRender.clear();
			if (GV::screens) {
				GV::screens->draw();
			}
			Renderer::needToRender = true;
		}

		Config::save();

		PyUtils::exec("CPP_EMBED: main.cpp", __LINE__, "globals().has_key('persistent_save') and persistent_save()");

#if 0
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
#endif

		if (PyUtils::exec("CPP_EMBED: main.cpp", __LINE__, "need_save", true) == "True") {
			PyUtils::exec("CPP_EMBED: main.cpp", __LINE__, "need_save = False");
			Game::save();
		}

		GV::updateMutex.unlock();

		int spent = Utils::getTimer() - startTime;
		int timeToSleep = Game::getFrameTime() - spent;
//		std::cout << spent << ' ' << timeToSleep << '\n';
		Utils::sleep(timeToSleep);
	}
}

void destroy() {
	GV::exit = true;
	GV::inGame = false;

	int toSleep = Game::getFrameTime() * 2;
	Utils::sleep(toSleep, false);

	SDL_CloseAudio();
	SDL_Quit();
}


int main() {
	if (!setlocale(LC_ALL, "C.UTF-8")) {
		Utils::outMsg("main", "Fail on set locale <C.UTF-8>");
	}

	int initStartTime = Utils::getTimer();
	if (init()) {
		return 0;
	}
	Logger::logEvent("Ren-Engine Initing", Utils::getTimer() - initStartTime);

	SDL_RendererInfo info;
	SDL_GetRendererInfo(GV::mainRenderer, &info);
	String driverInfo = String("Driver: ") + info.name + ", "
						"maxTextureWidth = " + info.max_texture_width + ", "
						"maxTextureHeight = " + info.max_texture_height + "\n\n";
	Logger::log(driverInfo);

	Game::startMod("main_menu");
//	Game::startMod("snow");

	loop();
	destroy();

	std::cout << "\nOk!\n";
	return 0;
}
