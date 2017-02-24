#ifndef MOUSE_H
#define MOUSE_H

#include <SDL2/SDL_mouse.h>

class Mouse {
private:
	static SDL_Cursor *usualModeCursor;
	static SDL_Cursor *btnModeCursor;

	static int x;
	static int y;

public:
	static void init();
	static void update();

	static void setUsualMode();
	static void setButtonMode();

	static void getPos(int &x, int &y);
	static int getX() { return x; }
	static int getY() { return y; }
};

#endif // MOUSE_H
