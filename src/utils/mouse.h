#ifndef MOUSE_H
#define MOUSE_H

#include <SDL2/SDL_mouse.h>

class Mouse {
private:
	static SDL_Cursor *usualModeCursor;
	static SDL_Cursor *btnModeCursor;

	static int x;
	static int y;

	static int localX;
	static int localY;

	static bool mouseDown;

public:
	static void init();
	static void update();

	static void setUsualMode();
	static void setButtonMode();

	static int getX() { return x; }
	static int getY() { return y; }

	static int getLocalX() { return localX; }
	static int getLocalY() { return localY; }
	static void setLocal(int x, int y) { localX = x; localY = y; }

	static bool getMouseDown() { return mouseDown; }
	static void setMouseDown(bool v) { mouseDown = v; }
};

#endif // MOUSE_H
