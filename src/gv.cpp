#include "gv.h"

int GV::width;
int GV::height;
bool GV::fullscreen;

int GV::numUpdate = 0;
bool GV::minimized = false;
bool GV::inGame = false;
bool GV::exit = false;

bool GV::beforeFirstFrame = false;
bool GV::firstFrame = false;

std::thread::id GV::messageThreadId;

SDL_DisplayMode GV::displayMode;
SDL_Window *GV::mainWindow = nullptr;

Uint8 GV::keyBoardState[SDL_NUM_SCANCODES];

Group *GV::screens = nullptr;

std::mutex GV::updateMutex;
double GV::prevFrameStartTime = 0;
double GV::frameStartTime = 0;
double GV::gameTime = 0;

std::mutex GV::mutexForSmartPtr;

Node *GV::mainExecNode = nullptr;
