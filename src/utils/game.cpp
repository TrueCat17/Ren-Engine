#include "game.h"


#include "gv.h"

#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "parser/music_channel.h"
#include "parser/parser.h"
#include "parser/py_guard.h"

#include "utils/utils.h"


size_t Game::fps = 1;
size_t Game::frameTime = 1000;

bool Game::modeStarting = false;


void Game::startMod(const std::string &dir) {
	std::thread([=] { Game::_startMod(dir); }).detach();

	int toSleep = frameTime * 2;
	Utils::sleep(toSleep, false);
}

void Game::_startMod(const std::string &dir) {
	GV::updateGuard.lock();
	modeStarting = true;

	GV::inGame = false;
	int toSleep = frameTime * 2;
	Utils::sleep(toSleep, false);

	delete GV::pyGuard;
	GV::pyGuard = new PyGuard();

	Node::destroyAll();
	Node::jumped = false;

	Screen::clear();
	DisplayObject::destroyAll();
	GV::screens = new Group();

	Utils::destroyAllTextures();
	Utils::destroyAllSurfaces();

	MusicChannel::clear();
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


int Game::getStageWidth() {
	return GV::width;
}
int Game::getStageHeight() {
	return GV::height;
}


void Game::updateKeyboard() {
	GV::keyBoardState = SDL_GetKeyboardState(nullptr);
	if (!GV::keyBoardState) {
		Utils::outMsg("SDL_GetKeyboardState", SDL_GetError());
	}
}


size_t Game::getFrameTime() {
	return frameTime;
}

size_t Game::getFps() {
	return fps;
}
void Game::setFps(size_t fps) {
	fps = Utils::inBounds(fps, 1, 60);

	Game::fps = fps;
	frameTime = 1000 / fps;
}
