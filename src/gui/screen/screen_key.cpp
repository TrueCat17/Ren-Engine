#include "screen_key.h"

#include "utils/utils.h"

ScreenKey::ScreenKey(Node *node): ScreenChild(node, nullptr) {
	keyStr = node->getFirstParam();
}

void ScreenKey::updateProps() {
	String keyName = Utils::execPython(keyStr, true);

	SDL_Scancode key = SDL_GetScancodeFromName(keyName.c_str());

	prevIsDown = false;
	if (key == SDL_SCANCODE_UNKNOWN) {
		Utils::outMsg("SDL_GetScancodeFromName", "KeyName <" + keyName + ">\n" + SDL_GetError());
	}else

	if (GV::keyBoardState && GV::keyBoardState[key]) {
		int dTime = Utils::getTimer() - lastDown;
		int delay = prevIsDown ? keyDelay : firstKeyDelay;

		if (dTime >= delay) {
			lastDown = Utils::getTimer();

			const String action = node->getPropCode("action");
			if (action) {
				Utils::execPython("exec_funcs(" + action + ")");
			}
		}
		prevIsDown = true;
	}
}
void ScreenKey::update() {
	return;
}
