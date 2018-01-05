#include "mouse.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "config.h"

#include "utils/utils.h"

SurfacePtr Mouse::usual;
SurfacePtr Mouse::btn;

SDL_Cursor *Mouse::usualModeCursor = nullptr;
SDL_Cursor *Mouse::btnModeCursor = nullptr;

int Mouse::x = 0;
int Mouse::y = 0;

int Mouse::localX = -1;
int Mouse::localY = -1;

bool Mouse::mouseDown = false;


void Mouse::init() {
	const String usualPath = Config::get("mouse_usual");
	if (usualPath && usualPath != "None") {
		usual = Utils::getSurface(usualPath);
	}
	if (usual) {
		usualModeCursor = SDL_CreateColorCursor(usual.get(), 0, 0);
	}else {
		usualModeCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	}

	const String btnPath = Config::get("mouse_btn");
	if (btnPath && btnPath != "None") {
		btn = Utils::getSurface(btnPath);
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
