#include <iostream>
#include <thread>
#include <filesystem>
#include <float.h>

#include <SDL2/SDL.h>
#undef main //for cancel declare spec. start-func on Windows

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "gv.h"
#include "config.h"
#include "logger.h"
#include "renderer.h"

#include "gui/gui.h"
#include "gui/screen/key.h"

#include "media/image_manipulator.h"
#include "media/music.h"
#include "media/py_utils.h"

#include "parser/syntax_checker.h"

#include "utils/btn_rect.h"
#include "utils/game.h"
#include "utils/image_caches.h"
#include "utils/math.h"
#include "utils/mouse.h"
#include "utils/string.h"
#include "utils/utils.h"

static std::string versionStr;
std::string getVersion() {
	if (versionStr.empty()) {
		std::string major = "0";
		std::string minor = "9";
		std::string micro = "0";

		std::string date = __DATE__;

		std::string months = "JanFebMarAprMayJunJulAugSepOctNovDec";
		size_t monthInt = months.find(date.substr(0, 3)) / 3 + 1;
		std::string month = std::string() + char((monthInt / 10) + '0') + char((monthInt % 10) + '0');

		std::string day = (date[4] == ' ') ? ('0' + date.substr(5, 1)) : date.substr(4, 2);
		std::string year = date.substr(7, 4);

		versionStr = major + "." + minor + "." + micro + "-" + year + "." + month + "." + day;
	}
	return versionStr;
}

//before windowSize-changes
static int startWindowWidth = 0;
static int startWindowHeight = 0;

static void changeWindowSize(bool maximized) {
	int startW, startH;
	SDL_GetWindowSize(GV::mainWindow, &startW, &startH);

	int dX = startW - startWindowWidth;
	int dY = startH - startWindowHeight;
	if (dX || dY) {
		int w = startW;
		int h = startH;

		double k = String::toDouble(Config::get("window_w_div_h"));
		if (k < 0.1) {
			k = 1.777;
			Utils::outMsg("changeWindowSize",
						  "Invalid <window_w_div_h> in <../resources/params.conf>:\n"
						  "<" + Config::get("window_w_div_h") + ">");
		}

		if (maximized) {//can only scale down
			if (double(w) / h > k) {
				w = int(h * k);
			}else {
				h = int(w / k);
			}
		}else {
			SDL_Rect usableBounds;

			if (GV::fullscreen) {
				SDL_DisplayMode displayMode;
				SDL_GetWindowDisplayMode(GV::mainWindow, &displayMode);
				usableBounds = {0, 0, displayMode.w, displayMode.h};
			}else {
				int index = SDL_GetWindowDisplayIndex(GV::mainWindow);
				if (index == -1) {
					Utils::outMsg("changeWindowSize, SDL_GetWindowDisplayIndex", SDL_GetError());
					index = 0;
				}
				SDL_GetDisplayUsableBounds(index, &usableBounds);

				int wTop, wLeft, wBottom, wRight;
				SDL_GetWindowBordersSize(GV::mainWindow, &wTop, &wLeft, &wBottom, &wRight);

				usableBounds.w -= wLeft + wRight;
				usableBounds.h -= wTop + wBottom;
			}

			if (abs(dX) >= abs(dY)) {
				h = int(w / k);
			}else {
				w = int(h * k);
			}

			const int MIN_W = 640;
			if (w < MIN_W || h < MIN_W / k) {
				w = MIN_W;
				h = int(MIN_W / k);
			}
			if (w > usableBounds.w) {
				w = usableBounds.w;
				h = int(w / k);
			}
			if (h > usableBounds.h) {
				h = usableBounds.h;
				w = int(h * k);
			}
		}
		if (w == startW && w == GV::width && h == startH && h == GV::height) return;


		int x = 0;
		int y = 0;
		if (maximized || GV::fullscreen) {
			x = (startW - w) / 2;
			y = (startH - h) / 2;
		}

		std::lock_guard g1(Renderer::toRenderMutex);
		std::lock_guard g2(Renderer::renderMutex);

		if (GV::screens) {
			GV::screens->setX(x);
			GV::screens->setY(y);
			GV::screens->setWidth(w);
			GV::screens->setHeight(h);
			GV::screens->updateGlobal();
		}

		GV::width = w;
		GV::height = h;
		if (!GV::fullscreen) {
			Config::set("window_width", std::to_string(w));
			Config::set("window_height", std::to_string(h));
		}
		SDL_SetWindowSize(GV::mainWindow, w + x, h + y);

		Renderer::needToRedraw = true;
	}

	startWindowWidth = 0;
	startWindowHeight = 0;
}

static std::string rootDirectory;
std::string setDir(std::string newRoot) {

#ifdef __WIN32__
	Py_SetPythonHome(const_cast<char*>("./py_libs/"));
#else
	if (!setlocale(LC_ALL, "C.UTF-8")) {
		Utils::outMsg("main", "Fail on set locale <C.UTF-8>");
	}
	setenv("PYTHONPATH", "./py_libs/Lib/", 1);
	setenv("PYTHONHOME", "./py_libs/Lib/", 1);
#endif

	if (newRoot.empty()) {
		char *charsPathUTF8 = SDL_GetBasePath();
		if (charsPathUTF8) {
			newRoot = charsPathUTF8;
			SDL_free(charsPathUTF8);
		}else {
			Utils::outMsg("setDir, SDL_GetBasePath", SDL_GetError());
		}

		String::replaceAll(newRoot, "\\", "/");
		newRoot += "../resources/";
	}

	std::vector<std::string> parts = String::split(newRoot, "/");
	for (size_t i = 0; i < parts.size(); ++i) {
		std::string &part = parts[i];
		if (part == ".") {
			parts.erase(parts.begin() + int(i));
			--i;
		}else

		if (part == ".." && i != 0) {
			parts.erase(parts.begin() + int(i) - 1, parts.begin() + int(i) + 1);
			i -= 2;
		}
	}
	if (parts.size() && parts[parts.size() - 1] != "") {
		parts.push_back("");
	}

	rootDirectory = newRoot = String::join(parts, "/");

	std::filesystem::path path(newRoot.c_str());
	std::error_code ec;
	std::filesystem::current_path(path, ec);

	if (!ec.value()) return "";
	return "Set current_path to <" + newRoot + "> failed";
}

bool init() {
	Math::init();
	Logger::init();
	Config::init();

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		Utils::outMsg("SDL_Init", SDL_GetError());
		return true;
	}

	std::string scaleQuality = Config::get("scale_quality");
	if (scaleQuality != "0" && scaleQuality != "1" && scaleQuality != "2") {
		Utils::outMsg("Config::get",
		              "Value of param scale_quality expected 0, 1 or 2, got <" + scaleQuality + ">");
		scaleQuality = "0";
	}
	if (scaleQuality != "0") {
		if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scaleQuality.c_str()) == SDL_FALSE) {
			Utils::outMsg("SDL_SetHint", "Could not to set scale quality");
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


	const int fps = String::toInt(Config::get("max_fps"));
	Game::setMaxFps(fps);


	GV::fullscreen = Config::get("window_fullscreen") == "True";

	int x = String::toInt(Config::get("window_x"));
	int y = String::toInt(Config::get("window_y"));
	int w, h;

	Uint32 flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;

	GV::width = w = Math::inBounds(String::toInt(Config::get("window_width")), 640, 2400);
	GV::height = h = Math::inBounds(String::toInt(Config::get("window_height")), 360, 1350);

	const std::string windowTitle = Config::get("window_title");
	GV::mainWindow = SDL_CreateWindow(windowTitle.c_str(), x, y, GV::width, GV::height, flags);
	if (!GV::mainWindow) {
		Utils::outMsg("SDL_CreateWindow", SDL_GetError());
		return true;
	}
	if (GV::fullscreen) {
		SDL_SetWindowFullscreen(GV::mainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}else{
		changeWindowSize(false);

		if (GV::width != w || GV::height != h) {
			SDL_SetWindowSize(GV::mainWindow, GV::width, GV::height);
		}
	}

	const std::string iconPath = Config::get("window_icon");
	if (!iconPath.empty() && iconPath != "None") {
		SurfacePtr icon = ImageCaches::getSurface(iconPath);
		if (icon) {
			SDL_SetWindowIcon(GV::mainWindow, icon.get());
		}
	}


	if (Renderer::init()) {
		return true;
	}
	ImageManipulator::init();
	Music::init();
	SyntaxChecker::init();

	return false;
}


void loop() {
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

		bool updateKeyboard = false;


		Mouse::update();
		BtnRect::checkMouseCursor();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.window.event == SDL_WINDOWEVENT_CLOSE || event.type == SDL_QUIT) {
				GV::exit = true;
				return;
			}
			if ((event.type & (SDL_MOUSEMOTION | SDL_MOUSEBUTTONDOWN | SDL_MOUSEBUTTONUP | SDL_MOUSEWHEEL)) == event.type) {
				Mouse::setLastAction();
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
				if (x || y) {//if x and y are 0 - then probably it error, ignore
					int leftBorderSize;
					int captionHeight;
					SDL_GetWindowBordersSize(GV::mainWindow, &captionHeight, &leftBorderSize, nullptr, nullptr);

					x = std::max(x - leftBorderSize, 1);
					y = std::max(y - captionHeight, 1);

					Config::set("window_x", std::to_string(x));
					Config::set("window_y", std::to_string(y));
				}
			}else

			if (event.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
				maximazed = true;
			}else
			if (event.window.event == SDL_WINDOWEVENT_RESTORED) {
				maximazed = false;
				GV::minimized = false;
			}else
			if (event.window.event == SDL_WINDOWEVENT_MINIMIZED) {
				GV::minimized = true;
			}else

			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				if (!GV::minimized && !startWindowWidth && !startWindowHeight) {
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
					updateKeyboard = true;

					SDL_Scancode key = event.key.keysym.scancode;

					if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_SPACE) {
						if (BtnRect::checkMouseClick(true, true)) {
							Key::setToNotReact(key);
						}else {
							Key::setFirstDownState(key);
						}
					}else {
						Key::setFirstDownState(key);
					}
				}
			}else

			if (event.type == SDL_KEYUP) {
				updateKeyboard = true;

				SDL_Scancode key = event.key.keysym.scancode;
				Key::setUpState(key);
			}
		}
		if (GV::minimized) {
			resizeWithoutMouseDown = false;
		}

		Mouse::checkCursorVisible();

		if (mouseOut) {
			mouseOutDown = SDL_GetGlobalMouseState(nullptr, nullptr);
		}
		if (updateKeyboard) {
			Game::updateKeyboard();
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

		GV::frameStartTime = Utils::getTimer();
		GUI::update();

		if (!GV::minimized) {
			while (Renderer::needToRender) {
				Utils::sleep(1);
			}
			std::lock_guard g(Renderer::toRenderMutex);
			Renderer::toRender.clear();
			if (GV::screens) {
				GV::screens->draw();
			}
			Renderer::needToRender = true;

			PyUtils::exec("CPP_EMBED: main.cpp", __LINE__, "globals().has_key('sl_check_autosave') and sl_check_autosave()");
		}
		PyUtils::exec("CPP_EMBED: main.cpp", __LINE__, "globals().has_key('persistent_save') and persistent_save()");

		Config::save();

		if (PyUtils::exec("CPP_EMBED: main.cpp", __LINE__, "need_save", true) == "True") {
			PyUtils::exec("CPP_EMBED: main.cpp", __LINE__, "need_save = False");
			Game::save();
		}
		if (PyUtils::exec("CPP_EMBED: main.cpp", __LINE__, "need_screenshot", true) == "True") {
			PyUtils::exec("CPP_EMBED: main.cpp", __LINE__, "need_screenshot = False");
			Game::makeScreenshot();
		}

		GV::updateMutex.unlock();

		const int spent = Utils::getTimer() - startTime;
		const int timeToSleep = Game::getFrameTime() - spent;
//		std::cout << spent << ' ' << timeToSleep << '\n';
		Utils::sleep(timeToSleep);
	}
}

void destroy() {
	GV::exit = true;
	GV::inGame = false;

	int toSleep = Game::getFrameTime() + 3;

	SDL_Event event;
	while (toSleep) {
		while (SDL_PollEvent(&event)) {}

		Utils::sleep(1, false);
		--toSleep;
	}

	SDL_CloseAudio();
	SDL_Quit();
}

int main(int argc, char **argv) {
	int initStartTime = Utils::getTimer();

	std::string arg;
	if (argc == 2) {
		arg = argv[1];
	}else if (argc > 2) {
		Utils::outMsg("main", "Expected max 1 arg, get: " + std::to_string(argc - 1));
		return 0;
	}

	for (const char *i : {"--help", "-help", "-h", "h", "/?", "?"}) {
		if (arg == i) {
			std::cout << "Ren-Engine is fast analog of Ren'Py\n"
						 "Usage:\n"
						 "  ./Ren-Engine [resources_dir=../resources/]\n"
						 "  ./Ren-Engine --help    - show this help and exit\n"
						 "  ./Ren-Engine --version - show version and exit\n"
						 "See https://github.com/TrueCat17/Ren-Engine\n"
						 "and https://github.com/TrueCat17/Ren-Engine/tree/master/examples\n";
			return 0;
		}
	}
	for (const char *i : {"--version", "-version", "-ver", "-v", "v"}) {
		if (arg == i) {
			std::cout << "Ren-Engine " << getVersion() << '\n';
			return 0;
		}
	}

	const std::string errMsg = setDir(arg);
	if (!errMsg.empty()) {
		Utils::outMsg("setDir", errMsg);
		return 0;
	}

	if (init()) {
		return 0;
	}

	{
		Logger::logEvent("Ren-Engine (version " + getVersion() + ") Initing", Utils::getTimer() - initStartTime);

		const char *platform = SDL_GetPlatform();
		Logger::log(std::string("OS: ") + platform);

		SDL_RendererInfo info;
		SDL_GetRendererInfo(GV::mainRenderer, &info);
		std::string driverInfo =
		        std::string("Renderer: ") + info.name + ", "
		        "maxTextureWidth = " + std::to_string(info.max_texture_width) + ", "
		        "maxTextureHeight = " + std::to_string(info.max_texture_height) + "\n";
		Logger::log(driverInfo);

		Logger::log("Resource Directory: " + rootDirectory);
		Logger::log("Application Name:   " + Config::get("window_title") + "\n\n");
	}

	Game::startMod("main_menu");

	{//update mouse pos on start
		int mouseX, mouseY;
		SDL_GetGlobalMouseState(&mouseX, &mouseY);

		SDL_WarpMouseGlobal(mouseX ^ 1, mouseY);
		SDL_WarpMouseGlobal(mouseX, mouseY);
	}

	loop();
	destroy();

	std::cout << "\nOk!\n";
	return 0;
}
