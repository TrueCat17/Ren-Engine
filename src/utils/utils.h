#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

#include "utils/image_typedefs.h"


class Utils {
public:
	static std::string getVersion();

	static void setThreadName(std::string name);

	static std::string getClipboardText();
	static bool setClipboardText(const std::string &text);

	static std::vector<std::string> getFileNames(const std::string &path);

	static double getTimer();
	static void sleep(double sec, bool checkInGame = true);

	static void outMsg(std::string msg, const std::string &err = "");
	static bool realOutMsg();

	static Uint32 getPixel(const SurfacePtr &surface, const SDL_Rect &draw, const SDL_Rect &crop);

	static std::string md5(const std::string &str);
};

#endif // UTILS_H
