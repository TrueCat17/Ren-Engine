#include "game.h"

#include <thread>


#include "gv.h"

#include "parser/parser.h"
#include "parser/py_guard.h"

#include "utils/utils.h"


bool Game::modeStarting = false;

size_t Game::fps = 1;
size_t Game::frameTime = 1000;

void Game::startMod(const std::string &dir) {
	std::thread([=] { Game::_startMod(dir); }).detach();
}

void Game::_startMod(const std::string &dir) {
	Game::modeStarting = true;
	GV::inGame = false;//указать потоку предыдущего мода на то, что пора заканчивать
	const int toSleep = Game::getFrameTime() * 2;
	Utils::sleep(toSleep, false);//Подождать, пока он не завершиться (он должен отслеживать состояние GV::inGame)

	delete GV::pyGuard;
	GV::pyGuard = new PyGuard();

	Node::destroyAll();
	Node::jumped = false;
	Game::modeStarting = false;

	try {
		Parser p(Utils::ROOT + "mods/" + dir);
		GV::mainExecNode = p.parse();

		GV::inGame = true;//Указываем создаваемому потоку, что можно работать и не надо будет завершаться при исполнении первой же команды

		GV::mainExecNode->execute();
	}catch (String e) {
		Utils::outMsg(e);
	}

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
