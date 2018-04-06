#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL_stdinc.h> //Uint32

#include "utils/string.h"

namespace boost { namespace python {
namespace api
{
  class object;
}
using api::object;
}} // namespace boost::python


class Game {
private:
	static int maxFps;

	static int fps;
	static int frameTime;

	static void _startMod(const String &dir, const String &loadPath = "");

public:
	static bool modeStarting;


	static void startMod(const std::string &dir);

	static void load(const std::string &table, const std::string &name);
	static const std::vector<String> loadInfo(const String &loadPath);

	static void save();
	static void exitFromGame();


	static void makeScreenshot();

	static bool hasLabel(const std::string &label);

	static int getStageWidth();
	static int getStageHeight();

	static int getTextureWidth(const std::string &image);
	static int getTextureHeight(const std::string &image);

	static Uint32 getPixel(const std::string &image, int x, int y);

	static std::string getFromConfig(const std::string &param);

	static boost::python::object getArgs(const std::string &str);

	static void updateKeyboard();

	static void setMaxFps(int fps);

	static int getFrameTime();
	static int getFps();
	static void setFps(int fps);

	static void setStageSize(int width, int height);
	static void setFullscreen(bool value);
};

#endif // GAME_H
