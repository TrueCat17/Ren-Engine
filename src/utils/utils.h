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


typedef std::shared_ptr<SDL_Surface> SurfacePtr;
typedef std::shared_ptr<SDL_Texture> TexturePtr;


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

	static std::vector<std::pair<String, TexturePtr>> textures;
	static std::map<TexturePtr, SurfacePtr> textureSurfaces;

	static std::mutex surfaceGuard;
	static std::vector<std::pair<String, SurfacePtr>> surfaces;

	static double* sins;
	static double* coss;

public:
	static String ROOT;
	static String FONTS;

	static void init();

	static double getSin(int angle) { return sins[((angle % 360) + 360) % 360]; }
	static double getCos(int angle) { return coss[((angle % 360) + 360) % 360]; }

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

	static size_t getTextureWidth(const TexturePtr &texture);
	static size_t getTextureHeight(const TexturePtr &texture);

	static void trimSurfacesCache(const SurfacePtr &last);
	static SurfacePtr getThereIsSurfaceOrNull(const String &path);
	static SurfacePtr getSurface(const String &path);
	static void setSurface(const String &path, const SurfacePtr &surface);

	static void trimTexturesCache(const SurfacePtr &last);
	static TexturePtr getTexture(const String &path);

	static Uint32 getPixel(const SurfacePtr &surface, const SDL_Rect &draw, const SDL_Rect &crop);
	static Uint32 getPixel(const TexturePtr &texture, const SDL_Rect &draw, const SDL_Rect &crop);

	static bool registerImage(const String &desc, Node *declAt);
	static bool imageWasRegistered(const std::string &name);

	static std::string getImageCode(const std::string &name);
	static py::list getImageDeclAt(const std::string &name);
	static std::vector<String> getVectorImageDeclAt(const std::string &name);

	static std::pair<String, size_t> getImagePlace(const std::string &name);
};

#endif // UTILS_H
