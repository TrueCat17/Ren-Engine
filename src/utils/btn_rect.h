#ifndef BTNRECT_H
#define BTNRECT_H

#include <vector>
#include <functional>

#include "gui/display_object.h"

class BtnRect {
private:
	static std::vector<BtnRect*> btnRects;

	bool buttonMode = true;

	DisplayObject *owner = nullptr;
	std::function<void(DisplayObject*)> onClick = nullptr;

public:
	static void checkMouseCursor();
	static void checkMouseClick();

	bool mouseOvered = false;

	BtnRect();
	void init(DisplayObject *owner, std::function<void(DisplayObject*)> onClick = nullptr, bool buttonMode = true);
	void click() const;
	DisplayObject* getOwner() const { return owner; }
	~BtnRect();
};

#endif // BTNRECT_H
