#include "gv.h"


int GV::width;
int GV::height;

bool GV::inGame = false;
bool GV::exit = false;

PyGuard *GV::pyGuard = nullptr;

SDL_Window *GV::mainWindow;
SDL_Renderer *GV::mainRenderer;

const Uint8 *GV::keyBoardState = nullptr;

Group *GV::stage = nullptr;
Group *GV::screens = nullptr;

std::mutex GV::renderGuard;
std::mutex GV::updateGuard;

Node *GV::mainExecNode = nullptr;
