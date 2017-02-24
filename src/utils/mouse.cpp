#include "mouse.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "utils/utils.h"


SDL_Cursor *Mouse::usualModeCursor = nullptr;
SDL_Cursor *Mouse::btnModeCursor = nullptr;

int Mouse::x = 0;
int Mouse::y = 0;


void Mouse::init() {
	SDL_Surface *usual = IMG_Load((Utils::ROOT + "images/misc/mouse/1.png").c_str());
	usualModeCursor = SDL_CreateColorCursor(usual, 0, 0);
	SDL_FreeSurface(usual);

	SDL_Surface *btn = IMG_Load((Utils::ROOT + "images/misc/mouse/2.png").c_str());
	btnModeCursor = SDL_CreateColorCursor(btn, 0, 0);
	SDL_FreeSurface(btn);

	SDL_SetCursor(usualModeCursor);
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

void Mouse::getPos(int &x, int &y) {
	x = Mouse::x;
	y = Mouse::y;
}
