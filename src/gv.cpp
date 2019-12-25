#include "gv.h"

SDL_DisplayMode GV::displayMode;

int GV::width;
int GV::height;
bool GV::fullscreen;

int GV::numUpdate = 0;
bool GV::minimized = false;
bool GV::inGame = false;
bool GV::exit = false;

SDL_Window *GV::mainWindow;

const Uint8 *GV::keyBoardState = nullptr;

Group *GV::screens = nullptr;

int GV::frameStartTime = 0;
std::mutex GV::updateMutex;

std::mutex GV::mutexForSmartPtr;

Node *GV::mainExecNode = nullptr;
