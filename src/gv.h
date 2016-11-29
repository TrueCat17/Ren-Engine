#ifndef GV_H
#define GV_H

#include <mutex>

#include <SDL2/SDL.h>

class Group;
class ScreenChild;

class Node;

class PyGuard;


class GV {
public:
	static const int STD_WIDTH = 1920;//Размеры картинок в оригинальной игре
	static const int STD_HEIGHT = 1080;

	static int width;
	static int height;

	static bool inGame;
	static bool exit;

	static PyGuard *pyGuard;

	static SDL_Window *mainWindow;
	static SDL_Renderer *mainRenderer;

	static const Uint8 *keyBoardState;

	static Group *stage;
	static Group *screens;

	static std::mutex renderGuard;
	static std::mutex updateGuard;

	static Node *mainExecNode;
};

#endif // GV_H
