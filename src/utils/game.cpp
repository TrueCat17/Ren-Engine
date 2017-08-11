#include "game.h"

#include <thread>
#include <fstream>
#include <boost/filesystem.hpp>


#include "config.h"
#include "gv.h"

#include "gui/gui.h"
#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "media/image.h"
#include "media/music.h"

#include "parser/parser.h"

#include "utils/utils.h"


size_t Game::maxFps = 60;

size_t Game::fps = 60;
size_t Game::frameTime = 16;

bool Game::modeStarting = false;


void Game::startMod(const std::string &dir) {
	std::thread([=] { Game::_startMod(dir); }).detach();

	int toSleep = frameTime * 2;
	Utils::sleep(toSleep, false);
}
void Game::load(const std::string &table, int num) {
	static const String saves = Utils::ROOT + "saves/";
	const String tablePath = saves + table + '/';
	const String fullPath = saves + table + '/' + num + '/';

	auto outError = [=](const String &dir) {
		Utils::outMsg(String() + "Ошибка при попытке открыть сохранение <" + table + "_" + num + ">\n"
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


}
void Game::save() {
	const String table = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_table", true);
	const String num   = PyUtils::exec("CPP_EMBED: game.cpp", __LINE__, "save_num",   true);

	static const String saves = Utils::ROOT + "saves/";
	const String tablePath = saves + table + '/';
	const String fullPath = tablePath + num + '/';


	{//update & render
		GUI::update();

		SDL_SetRenderDrawColor(GV::mainRenderer, 0, 0, 0, 255);
		SDL_RenderClear(GV::mainRenderer);

		if (GV::screens) {
			GV::screens->draw();
		}
		SDL_RenderPresent(GV::mainRenderer);
	}


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
		infoFile << GV::mainExecNode->name << '\n';

		size_t lastIndex = GV::screens->children.size() - 1;
		for (size_t i = 0; i <= lastIndex; ++i) {
			Screen* s = dynamic_cast<Screen*>(GV::screens->children[i]);
			infoFile << s->name << (i == lastIndex ? '\n' : ' ');
		}

		infoFile << GV::numFor << '\n' <<
					GV::numScreenFor << '\n';
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
				SDL_Rect to = {0, 0, 320, 240};

				SDL_Surface *save = SDL_CreateRGBSurface(screenshot->flags, to.w, to.h, 32,
														 screenshot->format->Rmask, screenshot->format->Gmask,screenshot->format->Bmask, screenshot->format->Amask);
				SDL_BlitScaled(screenshot, &from, save, &to);

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

#include <iostream>
void Game::_startMod(const std::string &dir) {
	{
		modeStarting = true;
		GV::inGame = false;

		std::lock_guard<std::mutex> g(GV::updateGuard);

		int toSleep = frameTime * 2;
		Utils::sleep(toSleep, false);

		Music::clear();

		Node::destroyAll();

		Screen::clear();
		DisplayObject::destroyAll();
		GV::screens = new Group();

		Style::destroyAll();

		delete GV::pyUtils;
		GV::pyUtils = new PyUtils();
	}

	Parser p(Utils::ROOT + "mods/" + dir);
	GV::mainExecNode = p.parse();

	GV::inGame = true;
	modeStarting = false;

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
	SDL_Surface *surface = Image::getImage(image);
	if (surface) {
		return surface->w;
	}
	Utils::outMsg("Game::getTextureWidth", "surface == nullptr");
	return -1;
}
int Game::getTextureHeight(const std::string &image) {
	SDL_Surface *surface = Image::getImage(image);
	if (surface) {
		return surface->h;
	}
	Utils::outMsg("Game::getTextureHeight", "surface == nullptr");
	return -1;
}

Uint32 Game::getPixel(const std::string &image, int x, int y) {
	const SDL_Surface *surface = Image::getImage(image);
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
