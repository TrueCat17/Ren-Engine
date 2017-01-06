#include "screen_key.h"

#include "gv.h"

#include "parser/node.h"
#include "utils/utils.h"


std::vector<ScreenKey*> ScreenKey::screenKeys;

ScreenKey::ScreenKey(Node *node): ScreenChild(node, nullptr) {
	screenKeys.push_back(this);
	keyStr = node->getFirstParam();
}
ScreenKey::~ScreenKey() {
	for (size_t i = 0; i < screenKeys.size(); ++i) {
		if (screenKeys[i] == this) {
			screenKeys.erase(screenKeys.begin() + i);
			break;
		}
	}
}

void ScreenKey::setToNotReact(SDL_Scancode key) {
	for (ScreenKey *sk : screenKeys) {
		SDL_Scancode skKey = sk->getKey();
		if (skKey == key) {
			sk->toNotReact = true;
		}
	}
}
void ScreenKey::setFirstDownState(SDL_Scancode key) {
	for (ScreenKey *sk : screenKeys) {
		SDL_Scancode skKey = sk->getKey();
		if (skKey == key) {
			sk->lastDown = 0;
			sk->inFirstDown = true;
			sk->wasFirstDelay = false;
		}
	}
}
void ScreenKey::setUpState(SDL_Scancode key) {
	for (ScreenKey *sk : screenKeys) {
		SDL_Scancode skKey = sk->getKey();
		if (skKey == key) {
			sk->lastDown = 0;
			sk->prevIsDown = false;
			if (sk->toNotReact) {
				sk->inFirstDown = false;
			}
			sk->toNotReact = false;
		}
	}
}


void ScreenKey::updateProps() {
	if (!isModal() || toNotReact) return;

	SDL_Scancode key = getKey();

	if (key == SDL_SCANCODE_UNKNOWN) {
		String keyName = Utils::execPython(keyStr, true);
		Utils::outMsg("SDL_GetScancodeFromName", "KeyName <" + keyName + ">\n" + SDL_GetError());
	}else

	if ((GV::keyBoardState && GV::keyBoardState[key]) || inFirstDown) {
		int dTime = Utils::getTimer() - lastDown;
		int delay = !wasFirstDelay ? firstKeyDelay : keyDelay;

		if (dTime >= delay) {
			if (!inFirstDown) {
				wasFirstDelay = true;
			}
			inFirstDown = false;
			lastDown = Utils::getTimer();

			const String action = node->getPropCode("action");
			if (action) {
				Utils::execPython("exec_funcs(" + action + ")");
			}
		}
		prevIsDown = true;
	}

	else {
		prevIsDown = false;
		inFirstDown = false;
		lastDown = 0;
	}
}
void ScreenKey::updateSize() {
	return;
}
void ScreenKey::updatePos() {
	return;
}

SDL_Scancode ScreenKey::getKey() const {
	String keyName = Utils::execPython(keyStr, true);
	if (keyName.startsWith("K_", false)) {
		keyName = keyName.substr(2);
	}

	SDL_Scancode res = SDL_GetScancodeFromName(keyName.c_str());
	return res;
}
