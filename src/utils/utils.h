#ifndef UTILS_H
#define UTILS_H

#include <vector>

#include "utils/image_typedefs.h"


typedef struct _TTF_Font TTF_Font;
typedef struct _object PyObject;
class Node;


class Utils {
public:
	static std::vector<std::string> getFileNames(const std::string &path);

	static int getTimer();
	static void sleep(int ms, bool checkInGame = true);
	static void sleepMicroSeconds(int ms);

	static void outMsg(std::string msg, const std::string &err = "");

	static TTF_Font* getFont(const std::string &name, int size);

	static Uint32 getPixel(const SurfacePtr &surface, const SDL_Rect &draw, const SDL_Rect &crop);

	static void registerImage(Node *imageNode);
	static bool imageWasRegistered(const std::string &name);
	static void clearImages();

	static PyObject* getImageDeclAt(const std::string &name);
	static std::vector<std::string> getVectorImageDeclAt(const std::string &name);
};

#endif // UTILS_H
