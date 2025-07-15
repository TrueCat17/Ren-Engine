#include "mouse.h"

#include <SDL2/SDL_mouse.h>

#include "config.h"
#include "utils/image_caches.h"
#include "utils/image_typedefs.h"
#include "utils/math.h"
#include "utils/string.h"


bool Mouse::out = false;


static SDL_Cursor *usualModeCursor = nullptr;
static SDL_Cursor *btnModeCursor = nullptr;

void Mouse::init() {
	SurfacePtr usual, btn;

	const std::string usualPath = Config::get("mouse_usual");
	if (!usualPath.empty() && usualPath != "None") {
		usual = ImageCaches::getSurface(usualPath);
	}
	if (usual) {
		usualModeCursor = SDL_CreateColorCursor(usual.get(), 0, 0);
	}else {
		usualModeCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	}

	const std::string btnPath = Config::get("mouse_btn");
	if (!btnPath.empty() && btnPath != "None") {
		btn = ImageCaches::getSurface(btnPath);
	}
	if (btn) {
		btnModeCursor = SDL_CreateColorCursor(btn.get(), 0, 0);
	}else {
		btnModeCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	}

	setUsualMode();
}


static SDL_Cursor* currentCursor = nullptr;
void Mouse::setUsualMode() {
	if (currentCursor != usualModeCursor) {
		currentCursor = usualModeCursor;
		SDL_SetCursor(usualModeCursor);
	}
}
void Mouse::setButtonMode() {
	if (currentCursor != btnModeCursor) {
		currentCursor = btnModeCursor;
		SDL_SetCursor(btnModeCursor);
	}
}


static int x = 0;
static int y = 0;

void Mouse::update() {
	if (out) {
		x = y = -1000000;
	}else {
		SDL_GetMouseState(&x, &y);
	}
}

int Mouse::getX() {
	return x;
}
int Mouse::getY() {
	return y;
}


static int localX = -1;
static int localY = -1;

int Mouse::getLocalX() {
	return localX;
}
int Mouse::getLocalY() {
	return localY;
}
void Mouse::setLocal(int x, int y) {
	localX = x;
	localY = y;
}

static bool mouseDown = false;
bool Mouse::getMouseDown() {
	return mouseDown;
}
void Mouse::setMouseDown(bool value) {
	mouseDown = value;
}

static bool canHided = true;
bool Mouse::getCanHide() {
	return canHided;
}
void Mouse::setCanHide(bool value) {
	canHided = value;
}


static double lastAction = 0;
bool Mouse::haveActionInLastFrame() {
	return Math::doublesAreEq(lastAction, GV::frameStartTime);
}
void Mouse::setLastAction() {
	lastAction = GV::frameStartTime;
}
void Mouse::checkCursorVisible() {
	bool show = true;
	if (canHided) {
		const double timeToHide = String::toDouble(Config::get("mouse_hide_time"));
		if (timeToHide > 0) {
			show = GV::frameStartTime - lastAction < timeToHide;
		}
	}
	SDL_ShowCursor(show);
}
