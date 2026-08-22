#ifndef GV_H
#define GV_H

#include <mutex>

class Node;

class GV {
public:
	static int numUpdate;

	static std::mutex updateMutex;

	static bool inGame;
	static bool initing;
	static bool exit;

	static double prevFrameStartTime;
	static double frameStartTime;
	static double gameTime;

	static std::mutex mutexForSurfacePtr;

	static Node *mainExecNode;
};

#endif // GV_H
