#include "game.h"

#include <iostream>
#include <thread>
#include <fstream>
#include <boost/filesystem.hpp>


#include "config.h"
#include "gv.h"
#include "logger.h"
#include "renderer.h"

#include "gui/gui.h"
#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "media/image.h"
#include "media/music.h"

#include "parser/parser.h"


int Game::maxFps = 60;

int Game::fps = 60;
int Game::frameTime = 16;

bool Game::modeStarting = false;


void Game::startMod(const std::string &dir) {
	std::thread([=] { Game::_startMod(dir); }).detach();
}

void Game::load(const std::string &table, const std::string &name) {
	static const std::string saves = Utils::ROOT + "saves/";
	const std::string tablePath = saves + table + '/';
	const std::string fullPath = saves + table + '/' + name + '/';

	auto outError = [=](const std::string &dir) {
		Utils::outMsg(std::string() + "Ошибка при попытке открыть сохранение <" + table + "_" + name + ">\n"
									  "Отсутствует директория <" + dir + ">");
	};

	namespace fs = boost::filesystem;
	if (!fs::exists(saves)) {
		outError(saves);
		return;
	}
	if (!fs::exists(tablePath)) {
		outError(tablePath);
		return;
	}

	if (!fs::exists(fullPath)) {
		outError(fullPath);
		return;
	}


	auto fileExists = [](const std::string &path) -> bool {
		if (!boost::filesystem::exists(path)) return false;
		if (boost::filesystem::is_directory(path)) return false;
		if (!boost::filesystem::file_size(path)) return false;
		return true;
	};
	if (!fileExists(fullPath + "stack")) {
		Utils::outMsg("Файл <" + fullPath + "stack" + "> не существует или пуст");
		return;
	}
	if (!fileExists(fullPath + "info")) {
		Utils::outMsg("Файл <" + fullPath + "info" + "> не существует или пуст");
		return;
	}
	if (!fileExists(fullPath + "py_globals")) {
		Utils::outMsg("Файл <" + fullPath + "py_globals" + "> не существует или пуст");
		return;
	}

	String modName;
	{//load info
		std::ifstream is(fullPath + "info");

		std::getline(is, modName);
	}

	std::thread([=] { Game::_startMod(modName, fullPath); }).detach();
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
	size_t countScreens = tmp.toInt();
	for (size_t i = 0; i < countScreens; ++i) {
		String name;
		std::getline(is, name);
		startScreensVec.push_back(name);
	}
	std::getline(is, tmp);


	std::getline(is, tmp);
	size_t countMusicChannels = tmp.toInt();
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
	size_t countMusics = tmp.toInt();
	for (size_t i = 0; i < countMusics; ++i) {
		String url, fileName;

		std::getline(is, url);
		if (url.startsWith(Utils::ROOT)) {
			url.erase(0, Utils::ROOT.size());
		}


		std::getline(is, fileName);

		std::getline(is, tmp);
		const std::vector<String> tmpVec = tmp.split(' ');
		if (tmpVec.size() != 5) {
			Utils::outMsg(loadFile, "В строке <" + tmp + "> ожидалось 5 аргументов");
			continue;
		}

		int numLine = tmpVec[0].toInt();
		const String &channel = tmpVec[1];
		int fadeIn = tmpVec[2].toInt();
		int fadeOut = tmpVec[3].toInt();
		int64_t pos = tmpVec[4].toDouble();

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
	size_t countMixers = tmp.toInt();
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

	static const std::string saves = Utils::ROOT + "saves/";
	const std::string tablePath = saves + table + '/';
	const std::string fullPath = tablePath + name + '/';


	{//check directory to save
		namespace fs = boost::filesystem;
		if (!fs::exists(saves)) {
			fs::create_directory(saves);
		}
		if (!fs::exists(tablePath))  {
			fs::create_directory(tablePath);
		}
		if (!fs::exists(fullPath)) {
			fs::create_directory(fullPath);
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
		infoFile << GV::mainExecNode->name << "\n\n";

		//screens
		std::vector<const Screen*> mainScreens;
		for (const DisplayObject *d : GV::screens->children) {
			const Screen* s = dynamic_cast<const Screen*>(d);
			if (s && s->getName() != "pause" && s->getName() != "save") {
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
			infoFile << music->getUrl() << '\n'
					 << music->getFileName() << '\n'
					 << music->getNumLine() << ' '
					 << music->getChannel()->name << ' '
					 << music->getFadeIn() << ' '
					 << music->getFadeOut() << ' '
					 << music->getPos() << '\n';
		}
		infoFile << '\n';

		const std::map<std::string, double> mixerVolumes = Music::getMixerVolumes();
		infoFile << mixerVolumes.size() << '\n';
		for (const std::pair<std::string, double> &p : mixerVolumes) {
			infoFile << p.first << ' ' << p.second << '\n';
		}
		infoFile << '\n';
	}


	{//save screenshot
		PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshotting = True");
		GUI::update();
		PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshotting = False");

		Utils::sleep(20);
		while (Renderer::needToRender) {
			Utils::sleep(1);
		}
		{
			std::lock_guard<std::mutex> g(Renderer::renderMutex);
			std::lock_guard<std::mutex> g2(Renderer::toRenderMutex);

			Renderer::toRender.clear();
			if (GV::screens) {
				GV::screens->draw();
			}
		}

		const SurfacePtr screenshot = Renderer::getScreenshot();
		if (screenshot) {
			const String screenshotPath = fullPath + "screenshot.png";
			IMG_SavePNG(screenshot.get(), screenshotPath.c_str());
		}
	}


	{//save python global-vars
		PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_global_vars('" + fullPath + "py_globals')");
	}
}

void Game::_startMod(const String &dir, const String &loadPath) {
	static std::mutex modMutex;


	Logger::log("Start mod <" + dir + ">");

	int waitingStartTime = Utils::getTimer();

	modeStarting = true;
	GV::inGame = false;

	std::lock_guard<std::mutex> g(modMutex);

	{
		std::lock_guard<std::mutex> g2(GV::updateMutex);
		Logger::logEvent("Waiting while stoped executed mod", Utils::getTimer() - waitingStartTime);

		int clearStartTime = Utils::getTimer();
		Music::clear();

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
		GV::screens->updateGlobalPos();

		GV::numFor = GV::numScreenFor = 0;
		Node::stackDepth = 0;
		Node::stack.clear();

		Style::destroyAll();

		delete GV::pyUtils;
		GV::pyUtils = new PyUtils();

		Logger::logEvent("Clearing", Utils::getTimer() - clearStartTime);
	}

	Parser p(Utils::ROOT + "mods/" + dir);
	GV::mainExecNode = p.parse();

	GV::inGame = true;
	modeStarting = false;


	if (loadPath) {
		{//load stack
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


		GV::mainExecNode->loadPath = loadPath;
	}
	GV::mainExecNode->execute();
}

void Game::exitFromGame() {
	GV::inGame = false;
	GV::exit = true;
}

bool Game::hasLabel(const std::string &label) {
	for (Node *node : GV::mainExecNode->children) {
		if (node->command == "label" && node->name == label) {
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
	SurfacePtr surface = Image::getImage(image);
	if (surface) {
		return surface->w;
	}
	Utils::outMsg("Game::getTextureWidth", "surface == nullptr");
	return -1;
}
int Game::getTextureHeight(const std::string &image) {
	SurfacePtr surface = Image::getImage(image);
	if (surface) {
		return surface->h;
	}
	Utils::outMsg("Game::getTextureHeight", "surface == nullptr");
	return -1;
}

Uint32 Game::getPixel(const std::string &image, int x, int y) {
	SurfacePtr surface = Image::getImage(image);
	if (surface) {
		SDL_Rect draw = {x, y, surface->w, surface->h};
		SDL_Rect crop = {0, 0, surface->w, surface->h};
		return Utils::getPixel(surface, draw, crop);
	}
	Utils::outMsg("Game::getPixel", "surface == nullptr");
	return 0;
}


std::string Game::getFromConfig(const std::string &param) {
	return Config::get(param);
}
py::object Game::getArgs(const std::string &str) {
	std::vector<String> vec = Utils::getArgs(str);

	py::list res;
	for (const std::string& s : vec) {
		res.append(s);
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
	maxFps = Utils::inBounds(fps, 1, 60);
}

int Game::getFrameTime() {
	return frameTime;
}
int Game::getFps() {
	return fps;
}
void Game::setFps(int fps) {
	fps = Utils::inBounds(fps, 1, maxFps);

	Game::fps = fps;
	frameTime = 1000 / fps;
}

void Game::setStageSize(int width, int height) {
	bool fullscreen = GV::fullscreen;
	{
		std::lock_guard<std::mutex> g1(Renderer::toRenderMutex);
		std::lock_guard<std::mutex> g2(Renderer::renderMutex);

		if (fullscreen) {
			GV::fullscreen = false;
			Config::set("window_fullscreen", "False");

			SDL_SetWindowFullscreen(GV::mainWindow, 0);
		}

		SDL_RestoreWindow(GV::mainWindow);
		SDL_Event event;
		event.type = SDL_WINDOWEVENT;
		event.window.event = SDL_WINDOWEVENT_RESTORED;
		if (SDL_PushEvent(&event) < 0) {
			Utils::outMsg("setStageSize, SDL_PushEvent", SDL_GetError());
		}

		SDL_SetWindowSize(GV::mainWindow, width, height);
		event.window.event = SDL_WINDOWEVENT_RESIZED;
		if (SDL_PushEvent(&event) < 0) {
			Utils::outMsg("setStageSize, SDL_PushEvent", SDL_GetError());
		}
	}
}
void Game::setFullscreen(bool value) {
	if (value) {
		GV::fullscreen = true;
		Config::set("window_fullscreen", "True");

		{
			std::lock_guard<std::mutex> g1(Renderer::toRenderMutex);
			std::lock_guard<std::mutex> g2(Renderer::renderMutex);
			SDL_SetWindowFullscreen(GV::mainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
	}else {
		GV::fullscreen = false;
		Config::set("window_fullscreen", "False");

		{
			std::lock_guard<std::mutex> g1(Renderer::toRenderMutex);
			std::lock_guard<std::mutex> g2(Renderer::renderMutex);
			SDL_SetWindowFullscreen(GV::mainWindow, 0);
		}
	}
}
