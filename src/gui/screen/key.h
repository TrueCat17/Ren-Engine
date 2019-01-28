#ifndef KEY_H
#define KEY_H

#include "child.h"

#include "utils/string.h"


class Key: public Child {
private:
	static bool notReactOnSpace;
	static bool notReactOnEnter;
	static std::vector<Key*> screenKeys;

	int lastDown = 0;
	int lastUpdate = -1;
	bool prevIsDown = false;

	bool inFirstDown = false;
	bool wasFirstDelay = false;

	String prevKeyName;
	SDL_Scancode key;

public:
	double keyDelay = 0.010;
	double firstKeyDelay = 0.333;

	static void setToNotReact(const SDL_Scancode key);
	static void setFirstDownState(const SDL_Scancode key);
	static void setUpState(const SDL_Scancode key);

	Key(Node *node, Screen *screen);
	~Key();

	virtual void updateRect(bool needUpdatePos = true);
	virtual void checkEvents();
};

#endif // KEY_H
