#include "game.h"

#include <thread>
#include <fstream>
#include <time.h>

#include <SDL2/SDL_image.h>


#include "config.h"
#include "gv.h"
#include "logger.h"
#include "renderer.h"

#include "gui/gui.h"
#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "media/audio_manager.h"
#include "media/image_manipulator.h"
#include "media/py_utils.h"
#include "media/py_utils/convert_to_py.h"
#include "media/scenario.h"
#include "media/sprite.h"
#include "media/translation.h"

#include "parser/parser.h"
#include "parser/screen_code_generator.h"
#include "parser/screen_update_funcs.h"

#include "utils/algo.h"
#include "utils/btn_rect.h"
#include "utils/file_system.h"
#include "utils/math.h"
#include "utils/mouse.h"
#include "utils/string.h"
#include "utils/stage.h"
#include "utils/utils.h"


static long maxFps = 60;

static long fps = 60;
static double frameTime = 1.0 / 60;

static int modStartTime = 0;
static int modIndex = -1;

static const std::string savesPath = "../var/saves";


static void _startMod(const std::string &dir, const std::string &loadPath = "");
static void makeScreenshotHelper(const std::string &screenshotPath);

void Game::startMod(const std::string &dir) {
	std::thread(_startMod, dir, "").detach();
}
int Game::getModStartTime() {
	return modStartTime;
}
int Game::getCurrentModIndex() {
	return modIndex;
}


void Game::load(const std::string &table, const std::string &name) {
	const std::string tablePath = savesPath + '/' + table;
	const std::string fullPath = tablePath + '/' + name;

	for (const std::string &path : {savesPath, tablePath, fullPath}) {
		if (!FileSystem::exists(path)) {
			Utils::outError("Game::load",
			                "Failed to open save <%_%>\n"
			                "Directory <%> not found",
			                table, name, path);
			return;
		}
	}

	auto fileExists = [](const std::string &path) -> bool {
		return FileSystem::exists(path) && !FileSystem::isDirectory(path) && FileSystem::getFileSize(path);
	};
	for (const char *fileName : {"/stack", "/info", "/py_globals"}) {
		if (!fileExists(fullPath + fileName)) {
			Utils::outError("Game::load", "File <%%> not exists or empty", fullPath, fileName);
			return;
		}
	}

	std::string modName;
	{//load info
		std::ifstream is(fullPath + "/info");

		std::getline(is, modName);
	}

	std::thread(_startMod, modName, fullPath).detach();
}
const std::vector<std::string> Game::loadInfo(const std::string &loadPath) {
	std::vector<std::string> startScreensVec;

	const char *loadFunc = "CPP_Embed: Game::loadInfo";
	std::string tmp;


	std::ifstream is(loadPath + "/info");
	std::string modName;
	std::getline(is, modName);

	std::getline(is, tmp);
	GV::gameTime = String::toDouble(tmp);

	{
		long fps;
		bool hideMouse;

		std::getline(is, tmp);
		const std::vector<std::string> tmpVec = String::split(tmp, " ");
		if (tmpVec.size() != 2) {
			Utils::outError(loadFunc, "In string <%> expected 2 args", tmp);
			fps = 60;
			hideMouse = true;
		}else {
			fps = String::toInt(tmpVec[0]);
			if (fps <= 0 || fps > maxFps) {
				fps = maxFps;
			}

			hideMouse = tmpVec[1] == "True";
		}

		Game::setFps(fps);
		Mouse::setCanHide(hideMouse);
	}
	std::getline(is, tmp);

	std::getline(is, tmp);
	size_t countScreens = size_t(String::toInt(tmp));
	for (size_t i = 0; i < countScreens; ++i) {
		std::string name;
		std::getline(is, name);
		startScreensVec.push_back(name);
	}
	std::getline(is, tmp);

	AudioManager::load(is);

	return startScreensVec;
}

static void saveInfo(const std::string &path) {
	std::ofstream infoFile(path, std::ios::binary);

	//mod-name
	//game-time
	//fps hideMouse
	infoFile << GV::mainExecNode->params << '\n'
	         << GV::gameTime << '\n'
	         << Game::getFps() << ' '
	         << (Mouse::getCanHide() ? "True" : "False")  << "\n\n";

	//screens
	std::vector<const Screen*> mainScreens;
	for (const DisplayObject *d : Stage::screens->children) {
		const Screen* s = static_cast<const Screen*>(d);
		if (s->save) {
			mainScreens.push_back(s);
		}
	}
	infoFile << mainScreens.size() << '\n';
	for (const Screen *s : mainScreens) {
		infoFile << s->getName() << '\n';
	}
	infoFile << '\n';

	AudioManager::save(infoFile);
}

void Game::save() {
	const std::string page = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_page", true);
	const std::string slot = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_slot", true);

	const std::string tablePath = savesPath + '/' + page;
	const std::string fullPath = tablePath + '/' + slot;

	//check directory to save
	for (const std::string &path : {savesPath, tablePath, fullPath}) {
		if (!FileSystem::exists(path)) {
			FileSystem::createDirectory(path);
		}
	}


	std::string varsAreSaved = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__,
	    "pickling.save_global_vars('" + fullPath + "/py_globals')", true);
	if (varsAreSaved != "True") return;

	Scenario::saveStack(fullPath + "/stack");

	saveInfo(fullPath + "/info");

	GUI::update(true);
	makeScreenshotHelper(fullPath + "/screenshot.png");
}

static std::mutex modMutex;
static void _startMod(const std::string &dir, const std::string &loadPath) {
	Utils::setThreadName("scenario");

	double waitingStartTime = Utils::getTimer();

	GV::inGame = false;
	BtnRect::disableSelectMode(true);

	std::lock_guard g(modMutex);

	{
		std::lock_guard g2(GV::updateMutex);

		modStartTime = int(std::time(nullptr));

		Logger::log("Start mod <" + dir + ">");
		Logger::logEvent("Waiting for the running mod to stop", Utils::getTimer() - waitingStartTime);

		double clearStartTime = Utils::getTimer();
		AudioManager::clear();

		Sprite::clearImages();
		Node::destroyAll();

		Screen::clear();
		DisplayObject::destroyAll();

		Stage::screens = new Group();
		Stage::screens->setX(float(Stage::x));
		Stage::screens->setY(float(Stage::y));
		Stage::screens->setWidth(float(Stage::width));
		Stage::screens->setHeight(float(Stage::height));
		Stage::screens->updateGlobal();

		StyleManager::destroyAll();

		PyUtils::initInterpreter();

		Translation::init();

		ScreenCodeGenerator::clear();
		ScreenUpdateFuncs::clear();

		Config::setDefaultScaleQuality();

		Logger::logEvent("Clearing", Utils::getTimer() - clearStartTime);
	}

	Parser p("mods/" + dir);
	GV::mainExecNode = p.parse();

	GV::gameTime = 0;
	GV::beforeFirstFrame = true;
	Scenario::initing = true;//before inGame = true
	GV::inGame = true;
	modIndex += 1;
	Scenario::execute(loadPath);
}


static void makeScreenshotHelper(const std::string &screenshotPath) {
	{
		while (Renderer::needToRender) {
			Utils::sleep(0.001);
		}

		std::lock_guard g(Renderer::renderDataMutex);

		Renderer::renderData.clear();
		if (Stage::screens) {
			Stage::screens->draw();
		}

		Renderer::needMakeScreenshot();
	}

	const SurfacePtr screenshot = Renderer::getScreenshot();
	if (!screenshot) return;

	std::string width = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshot_width", true);
	std::string height = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshot_height", true);
	ImageManipulator::saveSurface(screenshot, screenshotPath, width, height);
}
void Game::makeScreenshot() {
	static const std::string path = "../var/screenshots";

	if (!FileSystem::exists(path)) {
		FileSystem::createDirectory(path);
	}

	bool exists = true;
	int num = 1;
	std::string screenshotPath;
	while (exists) {
		std::string numStr = std::to_string(num);
		while (numStr.size() < 4) {
			numStr = '0' + numStr;
		}

		screenshotPath = path + "/screenshot_" + numStr + ".png";
		exists = FileSystem::exists(screenshotPath.c_str());

		++num;
	}

	makeScreenshotHelper(screenshotPath);
}

void Game::exitFromGame() {
	GV::inGame = false;
	GV::exit = true;
}

bool Game::hasLabel(const std::string &label) {
	for (const Node *node : GV::mainExecNode->children) {
		if (node->command == "label" && node->params == label) {
			return true;
		}
	}
	return false;
}
PyObject* Game::getAllLabels() {
	PyObject *res = PyList_New(0);
	for (const Node *node : GV::mainExecNode->children) {
		if (node->command == "label") {
			PyObject *name = convertToPy(node->params);
			PyList_Append(res, name);
			Py_DECREF(name);
		}
	}
	return res;
}


int Game::getImageWidth(const std::string &image) {
	SurfacePtr surface = ImageManipulator::getImage(image, false);
	if (surface) {
		return surface->w;
	}
	Utils::outMsg("Game::getImageWidth", "surface == nullptr");
	return 0;
}
int Game::getImageHeight(const std::string &image) {
	SurfacePtr surface = ImageManipulator::getImage(image, false);
	if (surface) {
		return surface->h;
	}
	Utils::outMsg("Game::getImageHeight", "surface == nullptr");
	return 0;
}

Uint32 Game::getImagePixel(const std::string &image, int x, int y) {
	SurfacePtr surface = ImageManipulator::getImage(image, false);
	if (!surface) {
		Utils::outMsg("Game::getImagePixel", "surface == nullptr");
		return 0;
	}
	SDL_Rect draw = { x, y, surface->w, surface->h };
	SDL_Rect crop = { 0, 0, surface->w, surface->h };
	Uint32 pixel = Utils::getPixel(surface, draw, crop);
	return pixel;
}


std::string Game::getFromConfig(const std::string &param) {
	return Config::get(param);
}
PyObject* Game::getArgs(const std::string &str) {
	std::vector<std::string> vec = Algo::getArgs(str);

	PyObject *res = PyList_New(Py_ssize_t(vec.size()));
	for (size_t i = 0; i < vec.size(); ++i) {
		PyList_SET_ITEM(res, Py_ssize_t(i), convertToPy(vec[i]));
	}
	return res;
}


void Game::setMaxFps(long fps) {
	maxFps = Math::inBounds(fps, 1, 60);
}

double Game::getFrameTime() {
	return frameTime;
}
long Game::getFps() {
	return fps;
}
void Game::setFps(long newFps) {
	newFps = Math::inBounds(newFps, 1, maxFps);

	fps = newFps;
	frameTime = 1 / double(newFps);
}


double Game::getLastTick() {
	if (!GV::beforeFirstFrame && !GV::firstFrame) {
		return GV::frameStartTime - GV::prevFrameStartTime;
	}
	return 0;
}

double Game::getGameTime() {
	return GV::gameTime;
}
