#ifndef GV_H
#define GV_H

#include <mutex>
#include <thread>

#include <SDL2/SDL.h>


class Group;
class Node;


class GV {
public:
	static SDL_DisplayMode displayMode;

	static int width;
	static int height;
	static bool fullscreen;

	static int numUpdate;
	static bool minimized;
	static bool inGame;
	static bool exit;

	static std::thread::id messageThreadId;

	static SDL_Window *mainWindow;

	static Uint8 keyBoardState[SDL_NUM_SCANCODES];

	static Group *screens;

	static long frameStartTime;
	static std::mutex updateMutex;

	static std::mutex mutexForSmartPtr;

	static Node *mainExecNode;
};

#endif // GV_H
