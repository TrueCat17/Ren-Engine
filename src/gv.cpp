#include "gv.h"

int GV::numUpdate = 0;
bool GV::inGame = false;
bool GV::exit = false;

bool GV::beforeFirstFrame = false;
bool GV::firstFrame = false;

std::thread::id GV::messageThreadId;

std::mutex GV::updateMutex;
double GV::prevFrameStartTime = 0;
double GV::frameStartTime = 0;
double GV::gameTime = 0;

std::mutex GV::mutexForSmartPtr;

Node *GV::mainExecNode = nullptr;
