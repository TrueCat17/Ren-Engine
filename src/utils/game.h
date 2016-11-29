#ifndef GAME_H
#define GAME_H

#include <string>

class Game {
private:
	static size_t fps;
	static size_t frameTime;

	static void _startMod(const std::string &dir);

public:
	static bool modeStarting;

	static void startMod(const std::string &dir);
	static void exitFromGame();

	static int getStageWidth();
	static int getStageHeight();

	static void updateKeyboard();

	static size_t getFrameTime();
	static size_t getFps();
	static void setFps(size_t fps);
};

#endif // GAME_H
