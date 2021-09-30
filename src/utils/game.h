#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>

#include <SDL2/SDL_stdinc.h> //Uint32

#include <Python.h>


class Game {
public:
	static void startMod(const std::string &dir);
	static int getModStartTime();

	static bool getCanAutoSave();
	static void setCanAutoSave(bool v);

	static void load(const std::string &table, const std::string &name);
	static const std::vector<std::string> loadInfo(const std::string &loadPath);

	static void save();
	static void exitFromGame();


	static void makeScreenshot();

	static bool hasLabel(const std::string &label);
	static PyObject* getAllLabels();

	static int getImageWidth(const std::string &image);
	static int getImageHeight(const std::string &image);

	static Uint32 getImagePixel(const std::string &image, int x, int y);

	static std::string getFromConfig(const std::string &param);

	static PyObject* getArgs(const std::string &str);

	static void setMaxFps(long fps);

	static double getFrameTime();
	static long getFps();
	static void setFps(long fps);

	static double getLastTick();
	static double getGameTime();
};

#endif // GAME_H
