#ifndef UTILS_H
#define UTILS_H

#include <string>

#include "utils/image_typedefs.h"


class Utils {
public:
	static const std::string& getVersion();

	static void setThreadName(std::string name);

	static double getTimer();
	static void sleep(double sec, bool checkInGame = true);

	static Uint32 getPixel(const SurfacePtr &surface, const SDL_Rect &draw, const SDL_Rect &crop);

	static std::string md5(const std::string &str);
};

#endif // UTILS_H
