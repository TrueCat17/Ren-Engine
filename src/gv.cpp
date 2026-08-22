#include "gv.h"

int GV::numUpdate = 0;

std::mutex GV::updateMutex;

bool GV::inGame = false;
bool GV::initing = false;
bool GV::exit = false;

double GV::prevFrameStartTime = 0;
double GV::frameStartTime = 0;
double GV::gameTime = 0;

std::mutex GV::mutexForSurfacePtr;

Node *GV::mainExecNode = nullptr;
