#include "game.h"

#include <thread>
#include <fstream>
#include <boost/filesystem.hpp>


#include "config.h"
#include "gv.h"
#include "logger.h"

#include "gui/gui.h"
#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "media/image.h"
#include "media/music.h"

#include "parser/parser.h"


size_t Game::maxFps = 60;

size_t Game::fps = 60;
size_t Game::frameTime = 16;

bool Game::modeStarting = false;


void Game::startMod(const std::string &dir) {
	std::thread([=] { Game::_startMod(dir); }).detach();
}

void Game::load(const std::string &table, const std::string &name) {
	static const String saves = Utils::ROOT + "saves/";
	const String tablePath = saves + table + '/';
	const String fullPath = saves + table + '/' + name + '/';

	auto outError = [=](const String &dir) {
		Utils::outMsg(String() + "Ошибка при попытке открыть сохранение <" + table + "_" + name + ">\n"
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


	auto fileExists = [](String path) -> bool {
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

		const String &name = tmpVec.at(0);
		const String &mixer = tmpVec.at(1);
		const String &loop = tmpVec.at(2);
		double volume = tmpVec.at(3).toDouble();

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

		int numLine = tmpVec.at(0).toInt();
		const String &channel = tmpVec.at(1);
		int fadeIn = tmpVec.at(2).toInt();
		int fadeOut = tmpVec.at(3).toInt();
		int64_t pos = tmpVec.at(4).toDouble();

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

		const String &name = tmpVec.at(0);
		double volume = tmpVec.at(1).toDouble();

		Music::setMixerVolume(volume, name, loadFile, 0);
	}
	std::getline(is, tmp);

	return startScreensVec;
}

void Game::save() {
	const String table = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_table", true);
	const String name  = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_name",  true);

	static const String saves = Utils::ROOT + "saves/";
	const String tablePath = saves + table + '/';
	const String fullPath = tablePath + name + '/';

	PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshotting = True");
	{//update & render
		GUI::update();

		SDL_SetRenderDrawColor(GV::mainRenderer, 0, 0, 0, 255);
		SDL_RenderClear(GV::mainRenderer);

		if (GV::screens) {
			GV::screens->draw();
		}
		//Важно! SDL_RenderPresent не нужен!
	}
	PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "screenshotting = False");


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
			const Channel *channel = channels.at(i);
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
			const Music *music = musics.at(i);
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
		SDL_Surface* info = SDL_GetWindowSurface(GV::mainWindow);
		if (info) {
			auto &format = info->format;
			Uint8 bpp = format->BytesPerPixel;

			Uint8 *pixels = new Uint8[info->w * info->h * bpp];

			if (SDL_RenderReadPixels(GV::mainRenderer, &info->clip_rect, format->format, pixels, info->w * bpp)) {
				Utils::outMsg("SDL_RenderReadPixels", SDL_GetError());
			}else {
				SDL_Surface *screenshot = SDL_CreateRGBSurfaceFrom(pixels, info->w, info->h,
																   bpp * 8, info->w * bpp,
																   format->Rmask, format->Gmask, format->Bmask, format->Amask);

				SDL_Rect from = info->clip_rect;
				SDL_Rect to = {0, 0, 640, 360};

				SDL_Surface *save = SDL_CreateRGBSurface(screenshot->flags, to.w, to.h, 32,
														 screenshot->format->Rmask, screenshot->format->Gmask,screenshot->format->Bmask, screenshot->format->Amask);
				{
					SDL_BlitScaled(screenshot, &from, save, &to);
				}

				if (save) {
					const String screenshotPath = fullPath + "screenshot.png";
					IMG_SavePNG(save, screenshotPath.c_str());
					SDL_FreeSurface(save);
				}else {
					Utils::outMsg("SDL_CreateRGBSurfaceFrom", SDL_GetError());
				}
			}
			delete[] pixels;

			SDL_FreeSurface(info);
		}else {
			Utils::outMsg("SDL_GetWindowSurface", SDL_GetError());
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


void Game::setMaxFps(size_t fps) {
	maxFps = Utils::inBounds(fps, 1, 60);
}

size_t Game::getFrameTime() {
	return frameTime;
}
size_t Game::getFps() {
	return fps;
}
void Game::setFps(size_t fps) {
	fps = Utils::inBounds(fps, 1, maxFps);

	Game::fps = fps;
	frameTime = 1000 / fps;
}
