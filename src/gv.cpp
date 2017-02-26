#include "gv.h"

#include "media/py_utils.h"
#include "parser/node.h"

int GV::width;
int GV::height;

bool GV::inGame = false;
bool GV::exit = false;

PyUtils *GV::pyUtils = nullptr;

SDL_Window *GV::mainWindow;
SDL_Renderer *GV::mainRenderer;

const Uint8 *GV::keyBoardState = nullptr;

Group *GV::screens = nullptr;

std::mutex GV::updateGuard;
std::mutex GV::renderGuard;

Node *GV::mainExecNode = nullptr;
