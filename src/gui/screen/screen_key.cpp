#include "screen_key.h"

#include "gv.h"
#include "media/py_utils.h"
#include "parser/node.h"


std::vector<ScreenKey*> ScreenKey::screenKeys;

ScreenKey::ScreenKey(Node *node, Screen *screen):
	ScreenChild(node, nullptr, screen),
	keyStr(node->getFirstParam())
{
	screenKeys.push_back(this);

	needUpdateFields = false;
	clearProps();
	setProp(ScreenProp::KEY, NodeProp::initPyExpr(keyStr, node->getNumLine()));
	setProp(ScreenProp::FIRST_DELAY, node->getPropCode("first_delay"));
	setProp(ScreenProp::DELAY, node->getPropCode("delay"));

	preparationToUpdateCalcProps();

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
		if (skKey == key && sk->isModal()) {
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
	if (toNotReact || !isModal()) return;

	ScreenChild::calculateProps();

	if (propWasChanged[ScreenProp::FIRST_DELAY]) {
		propWasChanged[ScreenProp::FIRST_DELAY] = false;

		py::object firstKeyDelayObj = propValues[ScreenProp::FIRST_DELAY];
		bool isInt = PyUtils::isInt(firstKeyDelayObj);
		bool isFloat = !isInt && PyUtils::isFloat(firstKeyDelayObj);

		if (isInt || isFloat) {
			firstKeyDelay = PyUtils::getDouble(firstKeyDelayObj, isFloat) * 1000;
		}else {
			firstKeyDelay = 500;
			Utils::outMsg("ScreenKey::calculateProps",
						  "first_delay is not a number (" + PyUtils::getStr(firstKeyDelayObj) + ")");
		}
	}
	if (propWasChanged[ScreenProp::DELAY]) {
		propWasChanged[ScreenProp::DELAY] = false;

		py::object keyDelayObj = propValues[ScreenProp::DELAY];
		bool isInt = PyUtils::isInt(keyDelayObj);
		bool isFloat = !isInt && PyUtils::isFloat(keyDelayObj);

		if (isInt || isFloat) {
			keyDelay = PyUtils::getDouble(keyDelayObj, isFloat) * 1000;
		}else {
			keyDelay = 10;
			Utils::outMsg("ScreenKey::calculateProps",
						  "delay is not a number (" + PyUtils::getStr(keyDelayObj) + ")");
		}
	}

	SDL_Scancode key = getKey();

	if (key == SDL_SCANCODE_UNKNOWN) {
		const String keyName = PyUtils::getStr(propValues[ScreenProp::KEY]);
		Utils::outMsg("SDL_GetScancodeFromName",
					  "KeyName <" + keyName + ">\n" +
					  SDL_GetError() + '\n' +
					  node->getPlace());
	}else

	if ((GV::keyBoardState && GV::keyBoardState[key]) || inFirstDown) {
		int dTime = GV::frameStartTime - lastDown;
		int delay = !wasFirstDelay ? firstKeyDelay : keyDelay;

		if (dTime >= delay) {
			if (!inFirstDown) {
				wasFirstDelay = true;
			}
			inFirstDown = false;
			lastDown = GV::frameStartTime;

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
	const String keyName = PyUtils::getStr(propValues[ScreenProp::KEY]);

	int start = 0;
	if (keyName.startsWith("K_", false)) {
		start = 2;
	}

	SDL_Scancode res = SDL_GetScancodeFromName(keyName.c_str() + start);
	return res;
}
