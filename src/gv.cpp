#include "gv.h"

#include "parser/node.h"
#include "parser/py_guard.h"

int GV::width;
int GV::height;

bool GV::inGame = false;
bool GV::exit = false;

PyGuard *GV::pyGuard = nullptr;

SDL_Window *GV::mainWindow;
SDL_Renderer *GV::mainRenderer;

const Uint8 *GV::keyBoardState = nullptr;

Group *GV::screens = nullptr;

std::mutex GV::initGuard;
std::mutex GV::updateGuard;
std::mutex GV::renderGuard;

Node *GV::mainExecNode = nullptr;
