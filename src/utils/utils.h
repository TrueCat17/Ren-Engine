#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "utils/image_typedefs.h"
#include "utils/string.h"


class Node;

typedef struct _object PyObject;


class Utils {
private:
	static std::map<String, String> images;
	static std::map<String, Node*> declAts;

public:
	static std::vector<String> getFileNames(const std::string &path);

	static int getTimer();
	static void sleep(int ms, bool checkInGame = true);
	static void sleepMicroSeconds(int ms);

	static void outMsg(std::string msg, const std::string &err = "");

	static TTF_Font* getFont(const String &name, int size);

	static Uint32 getPixel(const SurfacePtr &surface, const SDL_Rect &draw, const SDL_Rect &crop);

	static void registerImage(Node *imageNode);
	static bool imageWasRegistered(const std::string &name);
	static void clearImages() { images.clear(); declAts.clear(); }

	static std::string getImageCode(const std::string &name);
	static PyObject* getImageDeclAt(const std::string &name);
	static std::vector<String> getVectorImageDeclAt(const std::string &name);

	static std::pair<String, size_t> getImagePlace(const std::string &name);
};

#endif // UTILS_H
