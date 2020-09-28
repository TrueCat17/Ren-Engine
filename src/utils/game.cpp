#include "game.h"

#include <set>
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

#include "media/image_manipulator.h"
#include "media/music.h"
#include "media/py_utils.h"
#include "media/scenario.h"

#include "parser/parser.h"
#include "parser/screen_node_utils.h"

#include "utils/algo.h"
#include "utils/file_system.h"
#include "utils/math.h"
#include "utils/mouse.h"
#include "utils/path_finder.h"
#include "utils/string.h"
#include "utils/utils.h"


static long maxFps = 60;

static long fps = 60;
static long frameTime = 16;

static int modStartTime = 0;
static bool canAutoSave = true;

static const std::string savesPath = "../var/saves";


static void _startMod(const std::string &dir, const std::string &loadPath = "");

void Game::startMod(const std::string &dir) {
	std::thread(_startMod, dir, "").detach();
}
int Game::getModStartTime() {
	return modStartTime;
}

bool Game::getCanAutoSave() {
	return canAutoSave;
}
void Game::setCanAutoSave(bool v) {
	canAutoSave = v;
}


void Game::load(const std::string &table, const std::string &name) {
	const std::string tablePath = savesPath + '/' + table;
	const std::string fullPath = tablePath + '/' + name;

	for (const std::string &path : {savesPath, tablePath, fullPath}) {
		if (!FileSystem::exists(path)) {
			Utils::outMsg("Could not to open save <" + table + "_" + name + ">\n"
			              "Directory <" + path + "> not found");
			return;
		}
	}

	auto fileExists = [](const std::string &path) -> bool {
		return FileSystem::exists(path) && !FileSystem::isDirectory(path) && FileSystem::getFileSize(path);
	};
	for (const char *fileName : {"/stack", "/info", "/py_globals"}) {
		if (!fileExists(fullPath + fileName)) {
			Utils::outMsg("File <" + fullPath + fileName + "> not exists or empty");
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

	const char *loadFile = "CPP_Embed: Game::loadInfo";
	std::string tmp;


	std::ifstream is(loadPath + "/info");
	std::string modName;
	std::getline(is, modName);

	{
		long fps;
		bool hideMouse, autosave;

		std::getline(is, tmp);
		const std::vector<std::string> tmpVec = String::split(tmp, " ");
		if (tmpVec.size() != 3) {
			Utils::outMsg(loadFile, "In string <" + tmp + "> expected 3 args");
			fps = 60;
			hideMouse = autosave = true;
		}else {
			fps = String::toInt(tmpVec[0]);
			if (fps == 0) {
				fps = 60;
			}else
			if (fps > maxFps) {
				fps = maxFps;
			}

			hideMouse = tmpVec[1] == "True";
			autosave = tmpVec[2] == "True";
		}

		Game::setFps(fps);
		Mouse::setCanHide(hideMouse);
		Game::setCanAutoSave(autosave);
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


	std::getline(is, tmp);
	size_t countMusicChannels = size_t(String::toInt(tmp));
	for (size_t i = 0; i < countMusicChannels; ++i) {
		std::getline(is, tmp);
		const std::vector<std::string> tmpVec = String::split(tmp, " ");
		if (tmpVec.size() != 4) {
			Utils::outMsg(loadFile, "In string <" + tmp + "> expected 4 args");
			continue;
		}

		const std::string &name = tmpVec[0];
		const std::string &mixer = tmpVec[1];
		const std::string &loop = tmpVec[2];
		double volume = String::toDouble(tmpVec[3]);

		if (!Music::hasChannel(name)) {
			Music::registerChannel(name, mixer, loop == "True", loadFile, 0);
		}
		Music::setVolume(volume, name, loadFile, 0);
	}
	std::getline(is, tmp);

	std::getline(is, tmp);
	size_t countMusics = size_t(String::toInt(tmp));
	for (size_t i = 0; i < countMusics; ++i) {
		std::string url, fileName;

		std::getline(is, url);
		std::getline(is, fileName);

		std::getline(is, tmp);
		const std::vector<std::string> tmpVec = String::split(tmp, " ");
		if (tmpVec.size() != 5) {
			Utils::outMsg(loadFile, "In string <" + tmp + "> expected 5 args");
			continue;
		}

		uint32_t numLine = uint32_t(String::toInt(tmpVec[0]));
		const std::string &channel = tmpVec[1];
		int fadeIn = String::toInt(tmpVec[2]);
		int fadeOut = String::toInt(tmpVec[3]);
		int64_t pos = int64_t(String::toDouble(tmpVec[4]));

		Music::play(channel + " '" + url + "'", fileName, numLine);
		Music *music = nullptr;
		for (Music *i : Music::getMusics()) {
			const Channel *c = i->getChannel();
			if (c && c->name == channel) {
				music = i;
				break;
			}
		}

		if (music) {
			music->setFadeIn(fadeIn);
			if (fadeOut) {
				music->setFadeOut(fadeOut);
			}
			music->setPos(pos);
		}else {
			Utils::outMsg(loadFile,
			              "Sound-file <" + url + "> not restored\n"
			              "Play from:\n"
			              "  File <" + fileName + ">\n"
			              "  Line " + std::to_string(numLine));
		}
	}
	std::getline(is, tmp);


	std::getline(is, tmp);
	size_t countMixers = size_t(String::toInt(tmp));
	for (size_t i = 0; i < countMixers; ++i) {
		std::getline(is, tmp);

		const std::vector<std::string> tmpVec = String::split(tmp, " ");
		if (tmpVec.size() != 2) {
			Utils::outMsg(loadFile, "In string <" + tmp + "> expected 2 args");
			continue;
		}

		const std::string &name = tmpVec[0];
		double volume = String::toDouble(tmpVec[1]);

		Music::setMixerVolume(volume, name, loadFile, 0);
	}
	std::getline(is, tmp);

	return startScreensVec;
}

void Game::save() {
	const std::string table = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_table", true);
	const std::string name  = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_name",  true);

	const std::string tablePath = savesPath + '/' + table;
	const std::string fullPath = tablePath + '/' + name;


	{//check directory to save
		for (const std::string &path : {savesPath, tablePath, fullPath}) {
			if (!FileSystem::exists(path)) {
				FileSystem::createDirectory(path);
			}
		}
	}



	{//save stack
		std::ofstream stackFile(fullPath + "/stack", std::ios::binary);

		std::vector<std::pair<std::string, std::string>> stackToSave = Scenario::getStackToSave();
		if (stackToSave.empty()) {
			stackFile << '\n';
		}else {
			for (auto p : stackToSave) {
				stackFile << p.first << ' ' << p.second << '\n';
			}
		}
	}


	{//save info
		std::ofstream infoFile(fullPath + "/info", std::ios::binary);

		//mod-name
		//fps hideMouse autosave
		infoFile << GV::mainExecNode->params << '\n'
		         << Game::getFps() << ' '
		         << (Mouse::getCanHide() ? "True" : "False") << ' '
		         << (Game::getCanAutoSave() ? "True" : "False") << "\n\n";

		//screens
		std::vector<const Screen*> mainScreens;
		for (const DisplayObject *d : GV::screens->children) {
			const Screen* s = static_cast<const Screen*>(d);

			static const std::set<std::string> noSaveScreens = {"pause", "load", "save"};
			if (!noSaveScreens.count(s->getName())) {
				mainScreens.push_back(s);
			}
		}
		infoFile << mainScreens.size() << '\n';
		for (const Screen *s : mainScreens) {
			infoFile << s->getName() << '\n';
		}
		infoFile << '\n';

		//music-channels
		const std::vector<Channel*> &channels = Music::getChannels();
		infoFile << channels.size() << '\n';
		for (size_t i = 0; i < channels.size(); ++i) {
			const Channel *channel = channels[i];
			infoFile << channel->name << ' '
					 << channel->mixer << ' '
					 << (channel->loop ? "True" : "False") << ' '
					 << int(channel->volume * 1000) / 1000.0 << '\n';
		}
		infoFile << '\n';

		//music-files
		const std::vector<Music*> &musics = Music::getMusics();
		infoFile << musics.size() << '\n';
		for (size_t i = 0; i < musics.size(); ++i) {
			const Music *music = musics[i];

			std::string musicUrl = music->getUrl();
			std::string musicFileName = music->getFileName();

			infoFile << musicUrl << '\n'
					 << musicFileName << '\n'
					 << music->getNumLine() << ' '
					 << music->getChannel()->name << ' '
					 << music->getFadeIn() << ' '
					 << music->getFadeOut() << ' '
					 << music->getPos() << '\n';
		}
		infoFile << '\n';

		const std::map<std::string, double> mixerVolumes = Music::getMixerVolumes();
		infoFile << mixerVolumes.size() << '\n';
		for (auto &p : mixerVolumes) {
			infoFile << p.first << ' ' << p.second << '\n';
		}
		infoFile << '\n';
	}


	{//save screenshot
		PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_screenshotting = True");
		GUI::update();
		PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_screenshotting = False");

		{
			while (Renderer::needToRender) {
				Utils::sleep(1);
			}

			std::lock_guard g(Renderer::toRenderMutex);

			Renderer::toRender.clear();
			if (GV::screens) {
				GV::screens->draw();
			}

			Renderer::needMakeScreenshot();
		}

		const SurfacePtr screenshot = Renderer::getScreenshot();
		if (screenshot) {
			std::string screenshotPath = fullPath + "/screenshot.png";
			std::string width = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshot_width", true);
			std::string height = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshot_height", true);

			ImageManipulator::saveSurface(screenshot, screenshotPath, width, height);
		}
	}


	{//save python global-vars
		PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_global_vars('" + fullPath + "/py_globals')");
	}
}

static std::mutex modMutex;
static void _startMod(const std::string &dir, const std::string &loadPath) {
	Utils::setThreadName("scenario");

	long waitingStartTime = Utils::getTimer();

	GV::inGame = false;

	std::lock_guard g(modMutex);

	{
		std::lock_guard g2(GV::updateMutex);

		modStartTime = int(std::time(nullptr));
		canAutoSave = true;

		Logger::log("Start mod <" + dir + ">");
		Logger::logEvent("Waiting while stoped executed mod", Utils::getTimer() - waitingStartTime);

		long clearStartTime = Utils::getTimer();
		Music::clear();

		Utils::clearImages();
		Node::destroyAll();

		int x, y;
		if (GV::screens) {
			x = GV::screens->getX();
			y = GV::screens->getY();
		}else {
			x = y = 0;
		}

		Screen::clear();
		DisplayObject::destroyAll();

		GV::screens = new Group();
		GV::screens->setX(x);
		GV::screens->setY(y);
		GV::screens->setWidth(GV::width);
		GV::screens->setHeight(GV::height);
		GV::screens->updateGlobal();

		Style::destroyAll();

		PyUtils::init();

		ScreenNodeUtils::clear();

		Logger::logEvent("Clearing", Utils::getTimer() - clearStartTime);
	}

	Parser p("mods/" + dir);
	GV::mainExecNode = p.parse();

	GV::inGame = true;

	Node::loadPath = loadPath;
	Scenario::execute();
}


void Game::makeScreenshot() {
	static const std::string path = "../var/screenshots";

	if (!FileSystem::exists(path)) {
		FileSystem::createDirectory(path);
	}

	{
		while (Renderer::needToRender) {
			Utils::sleep(1);
		}

		std::lock_guard g(Renderer::toRenderMutex);

		Renderer::toRender.clear();
		if (GV::screens) {
			GV::screens->draw();
		}

		Renderer::needMakeScreenshot();
	}

	const SurfacePtr screenshot = Renderer::getScreenshot();
	if (!screenshot) return;

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

	std::string width = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshot_width", true);
	std::string height = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshot_height", true);
	ImageManipulator::saveSurface(screenshot, screenshotPath, width, height);
}

void Game::exitFromGame() {
	GV::inGame = false;
	GV::exit = true;
}

bool Game::hasLabel(const std::string &label) {
	for (Node *node : GV::mainExecNode->children) {
		if (node->command == "label" && node->params == label) {
			return true;
		}
	}
	return false;
}
PyObject* Game::getAllLabels() {
	PyObject *res = PyList_New(0);
	for (Node *node : GV::mainExecNode->children) {
		if (node->command == "label") {
			PyObject *name = PyString_FromString(node->params.c_str());
			PyList_Append(res, name);
			Py_DECREF(name);
		}
	}
	return res;
}


int Game::getStageWidth() {
	return GV::width;
}
int Game::getStageHeight() {
	return GV::height;
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
	if (surface) {
		SDL_Rect draw = {x, y, surface->w, surface->h};
		SDL_Rect crop = {0, 0, surface->w, surface->h};
		Uint32 pixel = Utils::getPixel(surface, draw, crop);
		return pixel;
	}
	Utils::outMsg("Game::getImagePixel", "surface == nullptr");
	return 0;
}


std::string Game::getFromConfig(const std::string &param) {
	return Config::get(param);
}
PyObject* Game::getArgs(const std::string &str) {
	std::vector<std::string> vec = Algo::getArgs(str);

	PyObject *res = PyList_New(long(vec.size()));
	for (size_t i = 0; i < vec.size(); ++i) {
		const std::string &str = vec[i];
		PyObject *pyStr = PyString_FromString(str.c_str());
		PyList_SET_ITEM(res, i, pyStr);
	}
	return res;
}



void Game::setMaxFps(long fps) {
	maxFps = Math::inBounds(fps, 1, 60);
}

long Game::getFrameTime() {
	return frameTime;
}
long Game::getFps() {
	return fps;
}
void Game::setFps(long newFps) {
	newFps = Math::inBounds(newFps, 1, maxFps);

	fps = newFps;
	frameTime = 1000 / newFps;
}

void Game::setStageSize(int width, int height) {
	if (GV::minimized) return;

	std::lock_guard g1(Renderer::toRenderMutex);
	std::lock_guard g2(Renderer::renderMutex);

	bool fullscreen = GV::fullscreen;
	if (fullscreen) {
		GV::fullscreen = false;
		Config::set("window_fullscreen", "False");

		SDL_SetWindowFullscreen(GV::mainWindow, 0);
	}

	SDL_RestoreWindow(GV::mainWindow);
	SDL_Event eventRestored;
	eventRestored.type = SDL_WINDOWEVENT;
	eventRestored.window.event = SDL_WINDOWEVENT_RESTORED;
	if (SDL_PushEvent(&eventRestored) < 0) {
		Utils::outMsg("setStageSize, SDL_PushEvent", SDL_GetError());
	}

	SDL_SetWindowSize(GV::mainWindow, width, height);
	SDL_Event eventResized;
	eventResized.type = SDL_WINDOWEVENT;
	eventResized.window.event = SDL_WINDOWEVENT_RESIZED;
	if (SDL_PushEvent(&eventResized) < 0) {
		Utils::outMsg("setStageSize, SDL_PushEvent", SDL_GetError());
	}
}
void Game::setFullscreen(bool value) {
	if (GV::minimized) return;

	GV::fullscreen = value;
	Config::set("window_fullscreen", value ? "True" : "False");

	std::lock_guard g1(Renderer::toRenderMutex);
	std::lock_guard g2(Renderer::renderMutex);

	Renderer::needToUpdateViewPort = true;
	SDL_SetWindowFullscreen(GV::mainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP * value);
}
