#ifndef SCREENKEY_H
#define SCREENKEY_H

#include "screen_child.h"

#include "utils/string.h"

class ScreenKey: public ScreenChild {
private:
	static bool notReactOnSpace;
	static bool notReactOnEnter;
	static std::vector<ScreenKey*> screenKeys;

	int lastDown = 0;
	int lastUpdate = -1;
	bool prevIsDown = false;

	bool inFirstDown = false;
	bool wasFirstDelay = false;

	int keyDelay = 10;
	int firstKeyDelay = 500;

	String keyExpr;
	SDL_Scancode key;

public:
	static void setToNotReact(const SDL_Scancode key);
	static void setFirstDownState(const SDL_Scancode key);
	static void setUpState(const SDL_Scancode key);

	ScreenKey(Node *node, Screen *screen);
	~ScreenKey();

	virtual void calculateProps();
	virtual void updateSize() {}
	virtual void updatePos() {}
};

#endif // SCREENKEY_H
