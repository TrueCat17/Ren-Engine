#ifndef SCREENKEY_H
#define SCREENKEY_H

#include "screen_child.h"

#include "utils/string.h"

class ScreenKey: public ScreenChild {
private:
	static std::vector<ScreenKey*> screenKeys;

	int lastDown = 0;
	bool prevIsDown = false;
	bool toNotReact = false;

	bool inFirstDown = false;
	bool wasFirstDelay = false;

	int keyDelay = 10;
	int firstKeyDelay = 500;

	String keyStr;

	SDL_Scancode getKey() const;

public:
	static void setToNotReact(SDL_Scancode key);
	static void setFirstDownState(SDL_Scancode key);
	static void setUpState(SDL_Scancode key);

	ScreenKey(Node *node);
	~ScreenKey();

	virtual void calculateProps();
	virtual void updateSize() {}
	virtual void updatePos() {}
};

#endif // SCREENKEY_H
