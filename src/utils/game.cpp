#include "game.h"

#include <set>
#include <thread>
#include <filesystem>
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

#include "parser/parser.h"
#include "parser/screen_node_utils.h"

#include "utils/algo.h"
#include "utils/math.h"
#include "utils/utils.h"


static int maxFps = 60;

static int fps = 60;
static int frameTime = 16;

static int modStartTime = 0;
static bool canAutoSave = true;

bool Game::modStarting = false;


static void _startMod(const String &dir, const String &loadPath = "");

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
	static const std::string saves = "saves/";
	const std::string tablePath = saves + table + '/';
	const std::string fullPath = saves + table + '/' + name + '/';

	namespace fs = std::filesystem;

	auto outError = [&](const std::string &dir) {
		Utils::outMsg(std::string() + "Ошибка при попытке открыть сохранение <" + table + "_" + name + ">\n"
									  "Отсутствует директория <" + dir + ">");
	};
	for (const std::string &path : {saves, tablePath, fullPath}) {
		if (!fs::exists(path)) {
			outError(path);
			return;
		}
	}

	auto fileExists = [](const std::string &path) -> bool {
		return fs::exists(path) && !fs::is_directory(path) && fs::file_size(path);
	};
	for (const char *fileName : {"stack", "info", "py_globals"}) {
		if (!fileExists(fullPath + fileName)) {
			Utils::outMsg("Файл <" + fullPath + fileName + "> не существует или пуст");
			return;
		}
	}

	String modName;
	{//load info
		std::ifstream is(fullPath + "info");

		std::getline(is, modName);
	}

	std::thread(_startMod, modName, fullPath).detach();
}
const std::vector<String> Game::loadInfo(const String &loadPath) {
	std::vector<String> startScreensVec;

	const char *loadFile = "CPP_Embed: Game::loadInfo";
	String tmp;


	std::ifstream is(loadPath + "info");
	String modName;
	std::getline(is, modName);
	std::getline(is, tmp);

	std::getline(is, tmp);
	size_t countScreens = size_t(tmp.toInt());
	for (size_t i = 0; i < countScreens; ++i) {
		String name;
		std::getline(is, name);
		startScreensVec.push_back(name);
	}
	std::getline(is, tmp);


	std::getline(is, tmp);
	size_t countMusicChannels = size_t(tmp.toInt());
	for (size_t i = 0; i < countMusicChannels; ++i) {
		std::getline(is, tmp);
		const std::vector<String> tmpVec = tmp.split(' ');
		if (tmpVec.size() != 4) {
			Utils::outMsg(loadFile, "В строке <" + tmp + "> ожидалось 4 аргумента");
			continue;
		}

		const String &name = tmpVec[0];
		const String &mixer = tmpVec[1];
		const String &loop = tmpVec[2];
		double volume = tmpVec[3].toDouble();

		if (!Music::hasChannel(name)) {
			Music::registerChannel(name, mixer, loop == "True", loadFile, 0);
		}
		Music::setVolume(volume, name, loadFile, 0);
	}
	std::getline(is, tmp);

	std::getline(is, tmp);
	size_t countMusics = size_t(tmp.toInt());
	for (size_t i = 0; i < countMusics; ++i) {
		String url, fileName;

		std::getline(is, url);
		std::getline(is, fileName);

		std::getline(is, tmp);
		const std::vector<String> tmpVec = tmp.split(' ');
		if (tmpVec.size() != 5) {
			Utils::outMsg(loadFile, "В строке <" + tmp + "> ожидалось 5 аргументов");
			continue;
		}

		size_t numLine = size_t(tmpVec[0].toInt());
		const String &channel = tmpVec[1];
		int fadeIn = tmpVec[2].toInt();
		int fadeOut = tmpVec[3].toInt();
		int64_t pos = int64_t(tmpVec[4].toDouble());

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
						  "Не удалось восстановить музыку из файла <" + url + ">\n"
						  "Место вызова:\n"
						  "  Файл <" + fileName + ">\n"
						  "  Строка " + String(numLine));
		}
	}
	std::getline(is, tmp);


	std::getline(is, tmp);
	size_t countMixers = size_t(tmp.toInt());
	for (size_t i = 0; i < countMixers; ++i) {
		std::getline(is, tmp);

		const std::vector<String> tmpVec = tmp.split(' ');
		if (tmpVec.size() != 2) {
			Utils::outMsg(loadFile, "В строке <" + tmp + "> ожидалось 2 аргумента");
			continue;
		}

		const String &name = tmpVec[0];
		double volume = tmpVec[1].toDouble();

		Music::setMixerVolume(volume, name, loadFile, 0);
	}
	std::getline(is, tmp);

	return startScreensVec;
}

void Game::save() {
	const std::string table = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_table", true);
	const std::string name  = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_name",  true);

	static const std::string saves = "saves/";
	const std::string tablePath = saves + table + '/';
	const std::string fullPath = tablePath + name + '/';


	{//check directory to save
		namespace fs = std::filesystem;

		for (const std::string &path : {saves, tablePath, fullPath}) {
			if (!fs::exists(path)) {
				fs::create_directory(path);
			}
		}
	}



	{//save stack
		std::ofstream stackFile(fullPath + "stack");
		for (auto p : Node::stack) {
			stackFile << p.first << ' ' << p.second << '\n';
		}
	}


	{//save info
		std::ofstream infoFile(fullPath + "info");

		//mod-name
		infoFile << GV::mainExecNode->params << "\n\n";

		//screens
		std::vector<const Screen*> mainScreens;
		for (const DisplayObject *d : GV::screens->children) {
			const Screen* s = static_cast<const Screen*>(d);

			static const std::set<String> noSaveScreens = {"pause", "load", "save"};
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

			String musicUrl = music->getUrl();
			String musicFileName = music->getFileName();

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
			String screenshotPath = fullPath + "screenshot.png";
			String width = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshot_width", true);
			String height = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshot_height", true);

			ImageManipulator::saveSurface(screenshot, screenshotPath, width, height);
		}
	}


	{//save python global-vars
		PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_global_vars('" + fullPath + "py_globals')");
	}
}

static std::mutex modMutex;
static void _startMod(const String &dir, const String &loadPath) {
	int waitingStartTime = Utils::getTimer();

	Game::modStarting = true;
	GV::inGame = false;

	std::lock_guard g(modMutex);

	{
		std::lock_guard g2(GV::updateMutex);

		modStartTime = int(std::time(nullptr));
		canAutoSave = true;

		Logger::log("Start mod <" + dir + ">");
		Logger::logEvent("Waiting while stoped executed mod", Utils::getTimer() - waitingStartTime);

		int clearStartTime = Utils::getTimer();
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
		GV::screens->setPos(x, y);
		GV::screens->setSize(GV::width, GV::height);
		GV::screens->updateGlobal();

		GV::numFor = GV::numScreenFor = 0;
		Node::stackDepth = 0;
		Node::stack.clear();

		Style::destroyAll();

		delete PyUtils::obj;
		PyUtils::obj = new PyUtils();

		ScreenNodeUtils::clear();

		Logger::logEvent("Clearing", Utils::getTimer() - clearStartTime);
	}

	Parser p("mods/" + dir);
	GV::mainExecNode = p.parse();

	GV::inGame = true;
	Game::modStarting = false;


	if (loadPath) {
		//load stack
		Node::loading = true;

		std::ifstream is(loadPath + "stack");

		while (!is.eof()) {
			String first, second;
			is >> first >> second;
			if (!first || !second) break;

			std::pair<String, String> p(first, second);
			Node::stack.push_back(p);
		}
	}

	Node::loadPath = loadPath;
	GV::mainExecNode->execute();
}


void Game::makeScreenshot() {
	static const std::string path = "screenshots/";

	namespace fs = std::filesystem;
	if (!fs::exists(path)) {
		fs::create_directory(path);
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
	String screenshotPath;
	while (exists) {
		String numStr(num);
		while (numStr.size() < 4) {
			numStr = '0' + numStr;
		}

		screenshotPath = path + "screenshot_" + numStr + ".png";
		exists = fs::exists(screenshotPath.c_str());

		++num;
	}

	String width = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshot_width", true);
	String height = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshot_height", true);
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


int Game::getStageWidth() {
	return GV::width;
}
int Game::getStageHeight() {
	return GV::height;
}

int Game::getTextureWidth(const std::string &image) {
	SurfacePtr surface = ImageManipulator::getImage(image);
	if (surface) {
		return surface->w;
	}
	Utils::outMsg("Game::getTextureWidth", "surface == nullptr");
	return 0;
}
int Game::getTextureHeight(const std::string &image) {
	SurfacePtr surface = ImageManipulator::getImage(image);
	if (surface) {
		return surface->h;
	}
	Utils::outMsg("Game::getTextureHeight", "surface == nullptr");
	return 0;
}

Uint32 Game::getPixel(const std::string &image, int x, int y) {
	SurfacePtr surface = ImageManipulator::getImage(image);
	if (surface) {
		SDL_Rect draw = {x, y, surface->w, surface->h};
		SDL_Rect crop = {0, 0, surface->w, surface->h};
		Uint32 pixel = Utils::getPixel(surface, draw, crop);
		return pixel;
	}
	Utils::outMsg("Game::getPixel", "surface == nullptr");
	return 0;
}


std::string Game::getFromConfig(const std::string &param) {
	return Config::get(param);
}
PyObject* Game::getArgs(const std::string &str) {
	std::vector<String> vec = Algo::getArgs(str);

	PyObject *res = PyList_New(long(vec.size()));
	for (size_t i = 0; i < vec.size(); ++i) {
		const String &str = vec[i];
		PyObject *pyStr = PyString_FromString(str.c_str());
		PyList_SET_ITEM(res, i, pyStr);
	}
	return res;
}


void Game::updateKeyboard() {
	GV::keyBoardState = SDL_GetKeyboardState(nullptr);
	if (!GV::keyBoardState) {
		Utils::outMsg("SDL_GetKeyboardState", SDL_GetError());
	}
}


void Game::setMaxFps(int fps) {
	maxFps = Math::inBounds(fps, 1, 60);
}

int Game::getFrameTime() {
	return frameTime;
}
int Game::getFps() {
	return fps;
}
void Game::setFps(int newFps) {
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

	SDL_SetWindowFullscreen(GV::mainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP * value);
}
