#ifndef MOUSE_H
#define MOUSE_H

class Mouse {
public:
	static bool out;

	static void init();
	static void update();

	static void setUsualMode();
	static void setButtonMode();

	static int getX();
	static int getY();

	static int getLocalX();
	static int getLocalY();
	static void setLocal(int x, int y);

	static bool getMouseDown();
	static void setMouseDown(bool value);

	static bool haveActionInLastFrame();
	static void setLastAction();
	static void checkCursorVisible();

	static bool getCanHide();
	static void setCanHide(bool value);
};

#endif // MOUSE_H
