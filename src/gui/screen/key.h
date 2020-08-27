#ifndef KEY_H
#define KEY_H

#include <SDL2/SDL_keycode.h>

#include "child.h"


class Key: public Child {
private:
	static bool notReactOnSpace;
	static bool notReactOnEnter;
	static std::vector<Key*> screenKeys;

	long lastDown = 0;
	int lastUpdate = -1;
	bool prevIsDown = false;

	bool inFirstDown = false;
	bool wasFirstDelay = false;

	std::string prevKeyName;
	SDL_Keycode key = SDLK_UNKNOWN;
	SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;

public:
	double keyDelay = 0.010;
	double firstKeyDelay = 0.333;

	static void setToNotReact(const SDL_Keycode key);
	static void setFirstDownState(const SDL_Keycode key);
	static void setUpState(const SDL_Keycode key);

	Key(Node *node, Screen *screen);
	~Key();

	virtual void updateRect(bool needUpdatePos = true);
	virtual void checkEvents();
};

#endif // KEY_H
