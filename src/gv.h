#ifndef GV_H
#define GV_H

#include <mutex>
#include <thread>

#include <SDL2/SDL.h>


class Group;
class Node;


class GV {
public:
	static int width;
	static int height;
	static bool fullscreen;

	static int numUpdate;
	static bool minimized;
	static bool inGame;
	static bool exit;

	static bool beforeFirstFrame;
	static bool firstFrame;

	static std::thread::id messageThreadId;

	static SDL_DisplayMode displayMode;
	static SDL_Window *mainWindow;

	static Uint8 keyBoardState[SDL_NUM_SCANCODES];

	static Group *screens;

	static std::mutex updateMutex;
	static double prevFrameStartTime;
	static double frameStartTime;
	static double gameTime;

	static std::mutex mutexForSmartPtr;

	static Node *mainExecNode;
};

#endif // GV_H
