#ifndef KEY_H
#define KEY_H

#include <SDL2/SDL_keycode.h>

#include "child.h"


class Key: public Child {
private:
	double lastDown = 0;
	int lastUpdate = -1;
	bool prevIsDown = false;

	bool inFirstDown = false;
	bool wasFirstDelay = false;

	std::string prevKeyName;
	SDL_Keycode key = SDLK_UNKNOWN;

public:
	static bool getPressed(const SDL_Keycode key);
	static void setPressed(const SDL_Keycode key, bool value);

	static void setToNotReact(const SDL_Keycode key);
	static void setFirstDownState(const SDL_Keycode key);
	static void setUpState(const SDL_Keycode key);

	double delay = 0.010;
	double first_delay = 0.333;

	Key(Node *node, Screen *screen);
	~Key();

	virtual void updateSize();
	virtual void checkEvents();
};

#endif // KEY_H
