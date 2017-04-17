#include "game.h"

#include "config.h"
#include "gv.h"

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

void Game::_startMod(const std::string &dir) {
	GV::updateGuard.lock();
	modeStarting = true;

	Music::clear();

	GV::inGame = false;
	int toSleep = frameTime * 2;
	Utils::sleep(toSleep, false);

	delete GV::pyUtils;
	GV::pyUtils = new PyUtils();

	Node::destroyAll();

	Screen::clear();
	DisplayObject::destroyAll();
	GV::screens = new Group();

	Style::destroyAll();

	GV::updateGuard.unlock();

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
