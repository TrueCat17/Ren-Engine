#ifndef BTNRECT_H
#define BTNRECT_H

#include <array>

#include "SDL2/SDL_keycode.h"

#include "gui/screen/child.h"


class BtnRect {
private:
	static void processTabKey(bool shift);

	Child *owner;

public:
	static void checkMouseCursor();
	static bool checkMouseClick(bool left, bool withKeyboard = false);

	static bool getSelectMode();
	static void disableSelectMode(bool clearLastBtn = false);
	static std::array<SDL_Point, 4> getPointsOfSelectedRect();
	static void processKey(SDL_Keycode key, bool shift);
	static void checkSelectedBtn();


	Uint32 id;

	bool buttonMode:1;
	bool selectedByKeyboard:1;

	bool prevHovered:1;
	bool mouseOvered:1;

	bool mouseLeftDown:1;
	bool mouseRightDown:1;

	BtnRect(Child *owner);
	~BtnRect();

	bool isHovered() const {
		return mouseOvered || selectedByKeyboard;
	}

	void onHovered() const;
	void onUnhovered() const;

	void onLeftClick() const;
	void onRightClick() const;

	bool needIgnore() const;
	void checkEvents();
};

#endif // BTNRECT_H
