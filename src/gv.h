#ifndef GV_H
#define GV_H

#include <mutex>

#include <SDL2/SDL.h>


class Group;
class PyUtils;
class Node;


class GV {
public:
	static SDL_DisplayMode displayMode;

	static int width;
	static int height;
	static bool fullscreen;

	static size_t numFor;
	static size_t numScreenFor;

	static int numUpdate;
	static bool inGame;
	static bool exit;

	static PyUtils *pyUtils;

	static SDL_Window *mainWindow;
	static SDL_Renderer *mainRenderer;
	static bool isOpenGL;
	static bool checkOpenGlErrors;

	static const Uint8 *keyBoardState;

	static Group *screens;

	static int frameStartTime;
	static std::mutex updateMutex;

	static Node *mainExecNode;
};

#endif // GV_H
