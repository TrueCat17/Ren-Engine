#include "mouse.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>

#include "config.h"
#include "utils/image_caches.h"
#include "utils/image_typedefs.h"
#include "utils/utils.h"


static SDL_Cursor *usualModeCursor = nullptr;
static SDL_Cursor *btnModeCursor = nullptr;

void Mouse::init() {
	SurfacePtr usual, btn;

	const String usualPath = Config::get("mouse_usual");
	if (usualPath && usualPath != "None") {
		usual = ImageCaches::getSurface(usualPath);
	}
	if (usual) {
		usualModeCursor = SDL_CreateColorCursor(usual.get(), 0, 0);
	}else {
		usualModeCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	}

	const String btnPath = Config::get("mouse_btn");
	if (btnPath && btnPath != "None") {
		btn = ImageCaches::getSurface(btnPath);
	}
	if (btn) {
		btnModeCursor = SDL_CreateColorCursor(btn.get(), 0, 0);
	}else {
		btnModeCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	}

	setUsualMode();
}


void Mouse::setUsualMode() {
	SDL_SetCursor(usualModeCursor);
}
void Mouse::setButtonMode() {
	SDL_SetCursor(btnModeCursor);
}


static int x = 0;
static int y = 0;

void Mouse::update() {
	SDL_GetMouseState(&x, &y);
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


static int lastAction = Utils::getTimer();
void Mouse::setLastAction() {
	lastAction = Utils::getTimer();
}
void Mouse::checkCursorVisible() {
	bool show = true;
	if (canHided) {
		const double timeToHide = Config::get("mouse_hide_time").toDouble();

		if (timeToHide > 0) {
			const double time = (Utils::getTimer() - lastAction) / 1000.0;
			show = time < timeToHide;
		}
	}
	SDL_ShowCursor(show);
}
