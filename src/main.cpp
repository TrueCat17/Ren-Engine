#include <iostream>
#include <fstream>
#include <thread>

#include <SDL3/SDL.h>

#include <SDL3/SDL_image.h>
#include <SDL3/SDL_ttf.h>

#include "gv.h"
#include "config.h"
#include "logger.h"
#include "renderer.h"

#include "gui/gui.h"
#include "gui/screen/key.h"

#include "media/audio_manager.h"
#include "media/image_manipulator.h"
#include "media/py_utils.h"
#include "media/py_utils/py_code_disk_cache.h"
#include "media/scenario.h"

#include "parser/mods.h"
#include "parser/syntax_checker.h"

#include "utils/btn_rect.h"
#include "utils/file_system.h"
#include "utils/game.h"
#include "utils/image_caches.h"
#include "utils/math.h"
#include "utils/mouse.h"
#include "utils/scope_exit.h"
#include "utils/stage.h"
#include "utils/string.h"
#include "utils/utils.h"


//no declaration in cygwin
extern "C" {
int setenv(const char *name, const char *value, int replace);
}

static std::string rootDirectory;
static std::string setDir(std::string newRoot) {
	if (newRoot.empty()) {
		const char *charsPathUTF8 = SDL_GetBasePath();
		if (charsPathUTF8) {
			newRoot = charsPathUTF8;
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
			parts.erase(parts.begin() + long(i));
			--i;
		}else

		if (part == ".." && i != 0) {
			parts.erase(parts.begin() + long(i) - 1, parts.begin() + long(i) + 1);
			i -= 2;
		}
	}

	rootDirectory = newRoot = String::join(parts, "/");

	for (int i = 0; i < 2; ++i) {
		if (!parts.empty()) {
			parts.pop_back();
		}
	}
	
#ifdef __CYGWIN__
	//C:/path/to/ -> /cygdrive/c/path/to/
	if (!parts.empty() && String::endsWith(parts[0], ":")) {
		parts[0].pop_back();
		parts[0] = "/cygdrive/" + parts[0];
	}
#endif
	
	std::string pyLibs = String::join(parts, "/") + "/Ren-Engine/py_libs/";

	setenv("PYTHONPATH", pyLibs.c_str(), 1);
	setenv("PYTHONHOME", pyLibs.c_str(), 1);

	return FileSystem::setCurrentPath(newRoot);
}

static bool init() {
	GV::messageThreadId = std::this_thread::get_id();

	Math::init();
	Logger::init();
	Config::init();
	Mods::init();

	Utils::setThreadName(Config::get("window_title"));

	if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
		Utils::outMsg("SDL_Init", SDL_GetError());
		return false;
	}

	if (!TTF_Init()) {
		Utils::outMsg("TTF_Init", SDL_GetError());
		return false;
	}


	Mouse::init();


	const int fps = String::toInt(Config::get("max_fps"));
	Game::setMaxFps(fps);


	SDL_DisplayMode displayMode = Stage::getDisplayMode();
	Stage::width  = Math::inBounds(String::toInt(Config::get("window_width")),  Stage::MIN_WIDTH,  displayMode.w);
	Stage::height = Math::inBounds(String::toInt(Config::get("window_height")), Stage::MIN_HEIGHT, displayMode.h);

	std::string windowTitle = Config::get("window_title");
	Stage::window = SDL_CreateWindow(windowTitle.c_str(), Stage::width, Stage::height,
	                                 SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	if (!Stage::window) {
		Utils::outMsg("SDL_CreateWindow", SDL_GetError());
		return false;
	}

	std::string xStr = Config::get("window_x");
	std::string yStr = Config::get("window_y");
	int x, y;
	if (xStr == "None") {
		x = (displayMode.w - Stage::width) / 2;
	}else {
		x = String::toInt(xStr);
	}
	if (yStr == "None") {
		y = (displayMode.h - Stage::height) / 2;
	}else {
		y = String::toInt(yStr);
	}
	SDL_SetWindowPosition(Stage::window, x, y);

	//after set pos
	Stage::fullscreen = Config::get("window_fullscreen") == "True";
	if (Stage::fullscreen) {
		SDL_SetWindowFullscreen(Stage::window, true);
		SDL_SyncWindow(Stage::window);
	}

	Stage::needResize = true;
	Stage::changeWindowSize();

	std::string iconPath = Config::get("window_icon");
	if (!iconPath.empty() && iconPath != "None") {
		SurfacePtr icon = ImageCaches::getSurface(iconPath);
		if (icon) {
			SDL_SetWindowIcon(Stage::window, icon.get());
		}
	}


	if (Renderer::init()) {
		return false;
	}
	ImageManipulator::init();
	AudioManager::init();
	SyntaxChecker::init();
	PyUtils::init();

	return true;
}


static SDL_Keycode numpadKeys[] = {SDLK_END,  SDLK_DOWN,  SDLK_PAGEDOWN, // 1, 2, 3
                                   SDLK_LEFT, SDLK_SPACE, SDLK_RIGHT,    // 4, 5, 6
                                   SDLK_HOME, SDLK_UP,    SDLK_PAGEUP};  // 7, 8, 9
static SDL_Keycode getKeyCode(const SDL_KeyboardEvent &event) {
	SDL_Keycode key = event.key;
	if (key == SDLK_MENU) {     //Because SDL converts name "Menu" to keycode SDLK_APPLICATION,
		key = SDLK_APPLICATION; //but events contains keycode SDLK_MENU
	}else
	if (key == SDLK_KP_ENTER) {
		key = SDLK_RETURN;
	}else
	if (key == SDLK_KP_PERIOD) {
		key = SDLK_DELETE;
	}else
	if (key >= SDLK_KP_1 && key <= SDLK_KP_0) {
		if (key == SDLK_KP_0) {
			key = SDLK_0;
		}else {
			if (event.mod & SDL_KMOD_NUM) {
				key -= SDLK_KP_1 - SDLK_1;
			}else {
				key = numpadKeys[key - SDLK_KP_1];
			}
		}
	}
	return key;
}

static std::vector<SDL_Event> events;
static std::mutex eventMutex;

static void loop() {
	Utils::setThreadName("loop");

	bool leftShift = false;
	bool rightShift = false;

	bool &mouseOut = Mouse::out;
	bool mouseOutPrevDown = false;
	int startWindowWidth = 0;
	int startWindowHeight = 0;

	//unmaximized = restored + resized
	//but this events can be not in one frame (or even in nears)
	double resizedTime = -1;
	double restoredTime = -1;
	double maxTimeForUnmaximized = 0.1;

	std::vector<SDL_Event> tmpEvents;

	while (true) {
		while ((!GV::inGame || Scenario::initing) && !GV::exit) {
			Utils::sleep(0.010, false);
		}
		if (GV::exit) return;


		GV::updateMutex.lock();

		PyUtils::callInPythonThread([&]() { //for instant (for python) changes of 3 vars
			GV::prevFrameStartTime = GV::frameStartTime;
			GV::frameStartTime = Utils::getTimer();
			GV::gameTime += Game::getLastTick();

			pyExecFromCpp("signals.send('enter_frame')");
		});

		bool resizeWithoutMouseDown = false;
		bool mouseWasDown = false;
		bool mouseWasUp = false;

		tmpEvents.clear();
		{
			auto g = std::lock_guard(eventMutex);
			events.swap(tmpEvents);
		}

		for (const SDL_Event &event : tmpEvents) {
			Uint32 type = event.type;
			if (
			    type == SDL_EVENT_MOUSE_MOTION      ||
			    type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
			    type == SDL_EVENT_MOUSE_BUTTON_UP   ||
			    type == SDL_EVENT_MOUSE_WHEEL
			) {
				Mouse::setLastAction();
				break;
			}
		}

		Mouse::update();
		BtnRect::checkMouseCursor();

		for (const SDL_Event &event : tmpEvents) {
			if (GV::exit) return;

			Uint32 type = event.type;


			//window events

			if (type == SDL_EVENT_WINDOW_EXPOSED || type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
				Renderer::needToRedraw = true;
				Stage::minimized = false;
			}else
			if (type == SDL_EVENT_WINDOW_MOUSE_ENTER) {
				mouseOut = false;
			}else
			if (type == SDL_EVENT_WINDOW_MOUSE_LEAVE) {
				mouseOut = true;
			}else

			if (type == SDL_EVENT_WINDOW_MOVED) {
				int x, y;
				SDL_GetWindowPosition(Stage::window, &x, &y);
				int leftBorderSize = 0;
				int captionHeight = 0;
#ifdef __LINUX__
				//fix for SDL_GetWindowPosition on linux (x11? wm?)
				SDL_GetWindowBordersSize(Stage::window, &captionHeight, &leftBorderSize, nullptr, nullptr);
#endif
				x = std::max(x - leftBorderSize, 1);
				y = std::max(y - captionHeight, 1);

				Config::set("window_x", std::to_string(x));
				Config::set("window_y", std::to_string(y));
			}else

			if (type == SDL_EVENT_WINDOW_MINIMIZED) {
				Stage::minimized = true;
			}else
			if (type == SDL_EVENT_WINDOW_MAXIMIZED || type == SDL_EVENT_WINDOW_RESTORED) {
				Renderer::needToRedraw = true;
				Stage::minimized = false;
				if (type == SDL_EVENT_WINDOW_MAXIMIZED) {
					Stage::needResize = true;
					Stage::maximized = true;
					resizeWithoutMouseDown = true;
				}else {
					restoredTime = Utils::getTimer();
				}
			}else

			if (type == SDL_EVENT_WINDOW_RESIZED) {
				resizedTime = Utils::getTimer();
				if (!Stage::fullscreen && !startWindowWidth && !startWindowHeight) {
					resizeWithoutMouseDown = true;
				}
			}else


			//mouse events

			if (type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				BtnRect::disableSelectMode();

				bool left  = event.button.button == SDL_BUTTON_LEFT;
				bool right = event.button.button == SDL_BUTTON_RIGHT;
				if (left || right) {
					if (left) {
						mouseWasDown = true;
					}
					BtnRect::checkMouseClick(left, false);
				}
			}else
			if (type == SDL_EVENT_MOUSE_BUTTON_UP) {
				mouseWasUp = true;
				Mouse::setMouseDown(false);
			}else


			// keyboard events

			if (type == SDL_EVENT_KEY_DOWN) {
				SDL_Keycode key = getKeyCode(event.key);
				if (Key::getPressed(key)) continue;

				if (key == SDLK_RETURN || key == SDLK_SPACE) {
					if (BtnRect::checkMouseClick(true, true)) {
						Key::setToNotReact(key);
					}else {
						Key::setFirstDownState(key);
					}
				}else {
					bool isModKey = false;
					if (key == SDLK_LCTRL || key == SDLK_RCTRL || key == SDLK_LALT || key == SDLK_RALT) {
						isModKey = true;
					}else
					if (key == SDLK_LSHIFT) {
						isModKey = true;
						leftShift = true;
					}else
					if (key == SDLK_RSHIFT) {
						isModKey = true;
						rightShift = true;
					}

					bool isFKey = key >= SDLK_F1 && key <= SDLK_F12;

					bool selectMode;
					if (isModKey || isFKey) {
						selectMode = false;
					}else {
						selectMode = BtnRect::getSelectMode();
						BtnRect::processKey(key, leftShift || rightShift);
						if (BtnRect::getSelectMode()) {
							selectMode = true;
						}
					}

					if (!selectMode) {
						Key::setFirstDownState(key);
					}
				}
			}else

			if (type == SDL_EVENT_KEY_UP) {
				SDL_Keycode key = getKeyCode(event.key);
				Key::setUpState(key);

				if (key == SDLK_LSHIFT) {
					leftShift = false;
				}else
				if (key == SDLK_RSHIFT) {
					rightShift = false;
				}
			}
		}

		if (!Stage::fullscreen && Utils::getTimer() - std::min(restoredTime, resizedTime) < maxTimeForUnmaximized ) {
			restoredTime = resizedTime = -1;
			Stage::needResize = true;
			Stage::maximized = false;
		}

		Mouse::checkCursorVisible();

		bool mouseOutDown = false;
		if (mouseOut) {
			mouseOutDown = SDL_GetGlobalMouseState(nullptr, nullptr);
		}

		if (resizeWithoutMouseDown || !(mouseWasDown || mouseWasUp)) {
			if (resizeWithoutMouseDown ||
			    (mouseOutPrevDown && !mouseOutDown && startWindowWidth && startWindowHeight)
			) {
				Stage::changeWindowSize(0, 0, startWindowWidth, startWindowHeight);
				startWindowWidth = startWindowHeight = 0;
			}
			if (mouseOutDown && !mouseOutPrevDown) {
				SDL_GetWindowSize(Stage::window, &startWindowWidth, &startWindowHeight);
			}
		}
		mouseOutPrevDown = mouseOutDown;

		GUI::update();

		BtnRect::checkSelectedBtn();

		if (!Stage::minimized) {
			while (Renderer::needToRender) {
				Utils::sleep(0.001, false);
			}

			std::lock_guard g(Renderer::renderDataMutex);
			Renderer::updateSelectedRect();
			Renderer::renderData.clear();
			if (Stage::screens) {
				Stage::screens->draw();
			}
			Renderer::needToRender = true;
		}

		Config::save();

		if (pyExecFromCppWithRes("need_save") == "True") {
			pyExecFromCpp("need_save = False");
			Game::save();
		}
		if (pyExecFromCppWithRes("need_screenshot") == "True") {
			pyExecFromCpp("need_screenshot = False");
			Game::makeScreenshot();
		}

		pyExecFromCpp("signals.send('exit_frame')");

		GV::updateMutex.unlock();

		PyUtils::callInPythonThread(PyCodeDiskCache::checkSaving);

		const double spent = Utils::getTimer() - GV::frameStartTime;
		const double timeToSleep = Game::getFrameTime() - spent;
//		std::cout << spent << ' ' << timeToSleep << '\n';
		Utils::sleep(timeToSleep);

		if (!GV::beforeFirstFrame) {
			GV::firstFrame = false;
		}
	}
}

static void eventLoop() {
	std::thread(loop).detach();

	while (!GV::exit) {
		while (Utils::realOutMsg()) {}
		Stage::applyChanges();

		Utils::sleep(0.010, false);

		//SDL can change window size inside SDL_PollEvent => need lock renderMutex
		//  This code displays messages while trying lock the mutex (for case when messages appearance in render)
		while (true) {
			bool success = Renderer::renderMutex.try_lock();
			if (success) break;

			while (Utils::realOutMsg()) {}
			Utils::sleep(0.001, false);
		}
		ScopeExit se([]() {
			Renderer::renderMutex.unlock();
		});

		//for no deadlock
		int i = 0;
		for (; i < 50; ++i) {
			bool success = eventMutex.try_lock();
			if (success) break;

			while (Utils::realOutMsg()) {}
			Utils::sleep(0.001, false);
		}
		if (i == 50) continue;

		ScopeExit se2([]() {
			eventMutex.unlock();
		});


		SDL_Event event;
		Uint32 prevType = 0;
		while (SDL_PollEvent(&event)) {
			Uint32 type = event.type;

			//<moving> after enter/leave fullscreen is not real moving, skip
			if (prevType == SDL_EVENT_WINDOW_ENTER_FULLSCREEN || prevType == SDL_EVENT_WINDOW_LEAVE_FULLSCREEN) {
				if (type == SDL_EVENT_WINDOW_MOVED) continue;
			}

			if (type == SDL_EVENT_QUIT || type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
				GV::exit = true;
				return;
			}

			events.push_back(event);
			prevType = type;
		}
	}
}

static void destroy() {
	GV::exit = true;
	GV::inGame = false;

	double toSleep = Game::getFrameTime() + 0.003;

	SDL_Event event;
	while (toSleep > 0) {
		while (SDL_PollEvent(&event)) {}

		Utils::sleep(0.001, false);
		toSleep -= 0.001;
	}

	AudioManager::clear();
	SDL_Quit();
}

static bool argsProcessing(int argc, char **argv, std::string &resources) {
	if (argc > 2) {
		Utils::outError("argsProcessing", "Expected max 1 arg, got: %", argc - 1);
		return true;
	}

	if (argc == 2) {
		resources = argv[1];
	}else { //for more comfort debug
		std::string path;
		const char *charsPathUTF8 = SDL_GetBasePath();
		if (charsPathUTF8) {
			path = charsPathUTF8;
		}else {
			std::cout << "argsProcessing, SDL_GetBasePath:\n" <<
			             SDL_GetError() << '\n';
			path = FileSystem::getCurrentPath();
		}

		String::replaceAll(path, "\\", "/");
		if (!String::endsWith(path, "/")) {
			path += "/";
		}
		path += "../path_to_resources.txt";

		if (FileSystem::exists(path)) {
			std::ifstream is(path);
			while (!is.eof()) {
				std::string s;
				std::getline(is, s);
				if (!s.empty()) {
					resources = s;
				}
			}
			if (!FileSystem::isDirectory(resources)) {
				std::cout << "Path of resources is not a directory or does not exist\n"
				             "Path: " << resources << '\n';
				return true;
			}
		}
	}


	for (const char *i : { "--help", "-help", "-h", "h", "/?", "?" }) {
		if (resources == i) {
			std::cout << "Ren-Engine is fast analog of Ren'Py\n"
			             "Usage:\n"
			             "  ./Ren-Engine [resources_dir=../resources/]\n"
			             "  ./Ren-Engine --help    - show this help and exit\n"
			             "  ./Ren-Engine --version - show version and exit\n"
			             "Github: https://github.com/TrueCat17/Ren-Engine\n"
			             "Wiki:   https://github.com/TrueCat17/Ren-Engine/wiki\n";
			return true;
		}
	}
	for (const char *i : { "--version", "-version", "-ver", "-v", "v" }) {
		if (resources == i) {
			std::cout << "Ren-Engine " << Utils::getVersion() << '\n';
			return true;
		}
	}

	return false;
}

int main(int argc, char **argv) {
	std::string resources;
	if (argsProcessing(argc, argv, resources)) {
		return 0;
	}

	const std::string errMsg = setDir(resources);
	if (!errMsg.empty()) {
		Utils::outMsg("setDir", errMsg);
		return 0;
	}

	{
		double initStartTime = Utils::getTimer();
		if (!init()) {
			return 0;
		}
		Logger::logEvent("Ren-Engine (version " + Utils::getVersion() + ") Initing", Utils::getTimer() - initStartTime);

		std::string platform = SDL_GetPlatform();
		Logger::log("OS: " + platform);

		std::string driverInfo =
		        "Renderer: " + Renderer::driver + ", "
		        "maxTextureSize = " + std::to_string(Renderer::maxSize) + "\n";
		Logger::log(driverInfo);

		Logger::log("Resource Directory: " + rootDirectory);
		Logger::log("Application Name:   " + Config::get("window_title") + "\n\n");
	}

	Game::startMod("main_menu");

	{//update mouse pos on start
		float mouseX, mouseY;
		SDL_GetGlobalMouseState(&mouseX, &mouseY);

		SDL_WarpMouseGlobal(mouseX + 1, mouseY);
		SDL_WarpMouseGlobal(mouseX, mouseY);
	}

	eventLoop();
	destroy();

	std::cout << "\nOk!\n";
	return 0;
}
