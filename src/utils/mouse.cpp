#include "mouse.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "config.h"
#include "utils/image_caches.h"
#include "utils/utils.h"

SurfacePtr Mouse::usual;
SurfacePtr Mouse::btn;

SDL_Cursor *Mouse::usualModeCursor = nullptr;
SDL_Cursor *Mouse::btnModeCursor = nullptr;

int Mouse::x = 0;
int Mouse::y = 0;

int Mouse::localX = -1;
int Mouse::localY = -1;

bool Mouse::canHided = true;
int Mouse::lastAction = Utils::getTimer();

bool Mouse::mouseDown = false;


void Mouse::init() {
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

void Mouse::update() {
	SDL_GetMouseState(&x, &y);
}

void Mouse::setUsualMode() {
	SDL_SetCursor(usualModeCursor);
}
void Mouse::setButtonMode() {
	SDL_SetCursor(btnModeCursor);
}


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
