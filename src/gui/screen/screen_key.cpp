#include "screen_key.h"

#include "gv.h"
#include "media/py_utils.h"
#include "parser/node.h"


std::vector<ScreenKey*> ScreenKey::screenKeys;

ScreenKey::ScreenKey(Node *node):
	ScreenChild(node, nullptr),
	keyStr(node->getFirstParam())
{
	screenKeys.push_back(this);

	needUpdateFields = false;
	clearProps();
	setProp(ScreenProp::KEY, NodeProp::initPyExpr(keyStr, node->getNumLine()));
	setProp(ScreenProp::FIRST_DELAY, node->getPropCode("first_delay"));
	setProp(ScreenProp::DELAY, node->getPropCode("delay"));
	calculateProps();
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


void ScreenKey::calculateProps() {
	if (!isModal() || toNotReact) return;

	ScreenChild::calculateProps();
	firstKeyDelay = propValues.at(ScreenProp::FIRST_DELAY).toDouble() * 1000;
	keyDelay = propValues.at(ScreenProp::DELAY).toDouble() * 1000;

	SDL_Scancode key = getKey();

	if (key == SDL_SCANCODE_UNKNOWN) {
		const String &keyName = propValues.at(ScreenProp::KEY);
		Utils::outMsg("SDL_GetScancodeFromName",
					  "KeyName <" + keyName + ">\n" +
					  SDL_GetError() + '\n' +
					  node->getPlace());
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

			String action = node->getPropCode("action").pyExpr;
			if (action) {
				PyUtils::exec(getFileName(), getNumLine(), "exec_funcs(" + action + ")");
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

SDL_Scancode ScreenKey::getKey() const {
	const String &keyName = propValues.at(ScreenProp::KEY);
	if (!keyName) return SDL_SCANCODE_UNKNOWN;

	int start = 0;
	if (keyName.startsWith("K_", false)) {
		start = 2;
	}

	SDL_Scancode res = SDL_GetScancodeFromName(keyName.c_str() + start);
	return res;
}
