#ifndef BTNRECT_H
#define BTNRECT_H

#include "gui/screen/child.h"

class BtnRect {
private:
	Child *owner;

public:
	static void checkMouseCursor();
	static bool checkMouseClick(bool left, bool withKeyboard = false);

	bool buttonMode = true;

	bool prevMouseOvered = false;
	bool mouseOvered = false;

	bool mouseLeftDown = false;
	bool mouseRightDown = false;

	BtnRect(Child *owner);
	~BtnRect();

	void onHovered() const;
	void onUnhovered() const;

	void onLeftClick() const;
	void onRightClick() const;

	bool needIgnore() const;
	void checkEvents();
};

#endif // BTNRECT_H
