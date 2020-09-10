#include "gv.h"

SDL_DisplayMode GV::displayMode;

int GV::width;
int GV::height;
bool GV::fullscreen;

int GV::numUpdate = 0;
bool GV::minimized = false;
bool GV::inGame = false;
bool GV::exit = false;

std::thread::id GV::messageThreadId;

SDL_Window *GV::mainWindow;

Uint8 GV::keyBoardState[SDL_NUM_SCANCODES];

Group *GV::screens = nullptr;

long GV::frameStartTime = 0;
std::mutex GV::updateMutex;

std::mutex GV::mutexForSmartPtr;

Node *GV::mainExecNode = nullptr;
