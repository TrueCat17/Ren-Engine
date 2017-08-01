#ifndef UTILS_H
#define UTILS_H


#include <vector>
#include <map>
#include <chrono>
#include <mutex>

#include <boost/python.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "utils/string.h"


class StopException {};
class BreakException {};
class ContinueException {};
class ReturnException {};
class ExitException {};

class Node;

namespace py = boost::python;

class Utils {
private:
	static std::map<String, TTF_Font*> fonts;

	static std::map<String, String> images;
	static std::map<String, Node*> declAts;

	static std::vector<std::pair<String, SDL_Texture*>> textures;
	static std::map<const SDL_Texture*, SDL_Surface*> textureSurfaces;

	static std::mutex surfaceGuard;
	static std::vector<std::pair<String, SDL_Surface*>> surfaces;

public:
	static String ROOT;
	static String FONTS;

	static void init();

	static std::vector<String> getFileNames(const String &path);

	static std::chrono::system_clock::time_point startTime;
	static int getTimer();
	static void sleep(int ms, bool checkInGame = true);

	template<typename T, typename MIN, typename MAX>
	static T inBounds(T value, MIN min, MAX max) { return (value < T(min)) ? min : (value > T(max)) ? max : value; }

	template<typename T>
	static bool in(const T &value, const std::vector<T> &vec) { return std::find(vec.begin(), vec.end(), value) != vec.end(); }

	static bool isFirstByte(char c);
	static void outMsg(std::string msg, const std::string &err = "");

	static TTF_Font* getFont(const String &path, int size);
	static void destroyAllFonts();

	static size_t getStartArg(const String& args, size_t end);
	static size_t getEndArg(const String& args, size_t start);
	static String clear(String s);
	static std::vector<String> getArgs(String args);

	static size_t getTextureWidth(SDL_Texture *texture);
	static size_t getTextureHeight(SDL_Texture *texture);

	static void trimTexturesCache(const SDL_Surface *last);
	static SDL_Texture* getTexture(const String &path);

	static void trimSurfacesCache(const SDL_Surface* last);
	static SDL_Surface* getThereIsSurfaceOrNull(const String &path);
	static SDL_Surface* getSurface(const String &path);
	static void setSurface(const String &path, SDL_Surface *surface);

	static Uint32 getPixel(const SDL_Surface *surface, const SDL_Rect &draw, const SDL_Rect &crop);
	static Uint32 getPixel(SDL_Texture *texture, const SDL_Rect &draw, const SDL_Rect &crop);

	static bool registerImage(const String &desc, Node *declAt);
	static bool imageWasRegistered(const std::string &name);
	static std::string getImageCode(const std::string &name);
	static py::list getImageDeclAt(const std::string &name);
};

#endif // UTILS_H
