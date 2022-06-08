#ifndef BTNRECT_H
#define BTNRECT_H

#include <functional>

#include "gui/display_object.h"

class BtnRect {
private:
	DisplayObject *owner = nullptr;

	std::function<void (DisplayObject*)> _onLeftClick = nullptr;
	std::function<void (DisplayObject*)> _onRightClick = nullptr;

public:
	static void checkMouseCursor();
	static bool checkMouseClick(bool left, bool withKeyboard = false);


	bool buttonMode = true;

	bool mouseOvered = false;
	bool mouseLeftDown = false;
	bool mouseRightDown = false;

	BtnRect();
	~BtnRect();

	void init(DisplayObject *owner,
			  const std::function<void (DisplayObject*)> &onLeftClick = nullptr,
	          const std::function<void (DisplayObject*)> &onRightClick = nullptr);

	DisplayObject* getOwner() const { return owner; }

	void onLeftClick() const;
	void onRightClick() const;
};

#endif // BTNRECT_H
