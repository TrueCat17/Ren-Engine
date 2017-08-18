#ifndef BTNRECT_H
#define BTNRECT_H

#include <vector>
#include <functional>

#include "gui/display_object.h"

class BtnRect {
private:
	static std::vector<BtnRect*> btnRects;


	DisplayObject *owner = nullptr;

	std::function<void (DisplayObject*)> _onLeftClick = nullptr;
	std::function<void (DisplayObject*)> _onRightClick = nullptr;

	bool buttonMode = true;

public:
	static void checkMouseCursor();
	static bool checkMouseClick(bool left);

	bool mouseOvered = false;
	bool mouseLeftDown = false;
	bool mouseRightDown = false;

	BtnRect();
	~BtnRect();

	void init(DisplayObject *owner,
			  const std::function<void (DisplayObject*)> &onLeftClick = nullptr,
			  const std::function<void (DisplayObject*)> &onRightClick = nullptr,
			  bool buttonMode = true);

	DisplayObject* getOwner() const { return owner; }

	void onLeftClick() const;
	void onRightClick() const;
};

#endif // BTNRECT_H
