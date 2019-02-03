#include "gv.h"

SDL_DisplayMode GV::displayMode;

int GV::width;
int GV::height;
bool GV::fullscreen;

size_t GV::numFor = 0;
size_t GV::numScreenFor = 0;

int GV::numUpdate = 0;
bool GV::minimized = false;
bool GV::inGame = false;
bool GV::exit = false;

SDL_Window *GV::mainWindow;
SDL_Renderer *GV::mainRenderer;

const Uint8 *GV::keyBoardState = nullptr;

Group *GV::screens = nullptr;

int GV::frameStartTime = 0;
std::mutex GV::updateMutex;

Node *GV::mainExecNode = nullptr;
