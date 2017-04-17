#ifndef GAME_H
#define GAME_H

#include <thread>
#include <string>

#include <SDL2/SDL_stdinc.h> //Uint32

#include "media/py_utils.h"


class Game {
private:
	static size_t maxFps;

	static size_t fps;
	static size_t frameTime;

	static void _startMod(const std::string &dir);

public:
	static bool modeStarting;

	static void startMod(const std::string &dir);
	static void exitFromGame();

	static bool hasLabel(const std::string &label);

	static int getStageWidth();
	static int getStageHeight();

	static int getTextureWidth(const std::string &image);
	static int getTextureHeight(const std::string &image);

	static Uint32 getPixel(const std::string &image, int x, int y);

	static std::string getFromConfig(const std::string &param);

	static py::object getArgs(const std::string &str);

	static void updateKeyboard();

	static void setMaxFps(size_t fps);

	static size_t getFrameTime();
	static size_t getFps();
	static void setFps(size_t fps);
};

#endif // GAME_H
