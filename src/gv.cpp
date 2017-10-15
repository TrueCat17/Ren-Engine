#include "gv.h"


int GV::width;
int GV::height;

size_t GV::numFor = 0;
size_t GV::numScreenFor = 0;

bool GV::inGame = false;
bool GV::exit = false;

PyUtils *GV::pyUtils = nullptr;

SDL_Window *GV::mainWindow;
SDL_Renderer *GV::mainRenderer;
bool GV::isOpenGL = false;

const Uint8 *GV::keyBoardState = nullptr;

Group *GV::screens = nullptr;

std::mutex GV::updateMutex;

Node *GV::mainExecNode = nullptr;
