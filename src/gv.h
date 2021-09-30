#ifndef GV_H
#define GV_H

#include <mutex>
#include <thread>

#include <SDL2/SDL_scancode.h>


class Node;


class GV {
public:
	static int numUpdate;
	static bool inGame;
	static bool exit;

	static bool beforeFirstFrame;
	static bool firstFrame;

	static std::thread::id messageThreadId;

	static Uint8 keyBoardState[SDL_NUM_SCANCODES];

	static std::mutex updateMutex;
	static double prevFrameStartTime;
	static double frameStartTime;
	static double gameTime;

	static std::mutex mutexForSmartPtr;

	static Node *mainExecNode;
};

#endif // GV_H
