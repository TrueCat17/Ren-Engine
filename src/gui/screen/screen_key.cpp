#include "screen_key.h"

#include "gv.h"
#include "media/py_utils.h"
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

	firstKeyDelay = node->getProp("first_delay").toDouble() * 1000;
	keyDelay = node->getProp("delay").toDouble() * 1000;

	SDL_Scancode key = getKey();

	if (key == SDL_SCANCODE_UNKNOWN) {
		String keyName = PyUtils::exec(keyStr, true);
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

			String action = node->getPropCode("action");
			if (action) {
				PyUtils::exec("exec_funcs(" + action + ")");
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
	String keyName = PyUtils::exec(keyStr, true);
	if (keyName.startsWith("K_", false)) {
		keyName = keyName.substr(2);
	}

	SDL_Scancode res = SDL_GetScancodeFromName(keyName.c_str());
	return res;
}
