#ifndef MOUSE_H
#define MOUSE_H

#include <SDL2/SDL_mouse.h>

class Mouse {
private:
	static SDL_Cursor *usualModeCursor;
	static SDL_Cursor *btnModeCursor;

public:
	static void init();
	static void quit();

	static void setUsualMode();
	static void setButtonMode();

	static void getPos(int &x, int &y);
};

#endif // MOUSE_H
