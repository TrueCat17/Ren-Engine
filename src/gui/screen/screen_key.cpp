#include "screen_key.h"

#include "gv.h"
#include "media/py_utils.h"
#include "parser/node.h"
#include "utils/utils.h"

bool ScreenKey::notReactOnSpace = false;
bool ScreenKey::notReactOnEnter = false;
std::vector<ScreenKey*> ScreenKey::screenKeys;

ScreenKey::ScreenKey(Node *node, Screen *screen):
	ScreenChild(node, nullptr, screen),
	keyExpr(node->getFirstParam())
{
	screenKeys.push_back(this);

	needUpdateFields = false;
	clearProps();
	setProp(ScreenProp::KEY, NodeProp::initPyExpr(keyExpr, node->getNumLine()));
	setProp(ScreenProp::FIRST_DELAY, node->getPropCode("first_delay"));
	setProp(ScreenProp::DELAY, node->getPropCode("delay"));

	preparationToUpdateCalcProps();

	calculateProps();
}
ScreenKey::~ScreenKey() {
	const bool isModal = this->isModal();
	for (size_t i = 0; i < screenKeys.size(); ++i) {
		ScreenKey *sk = screenKeys[i];

		if (screenKeys[i] == this) {
			screenKeys.erase(screenKeys.begin() + i);
			--i;
		}else
		if (isModal && prevIsDown && sk->key == key && !sk->isModal()) {
			sk->lastDown = Utils::getTimer();
			sk->inFirstDown = true;
			sk->wasFirstDelay = false;
		}
	}
}

void ScreenKey::setToNotReact(const SDL_Scancode key) {
	if (key == SDL_SCANCODE_SPACE) {
		notReactOnSpace = true;
	}else
	if (key == SDL_SCANCODE_RETURN) {
		notReactOnEnter = true;
	}
}
void ScreenKey::setFirstDownState(const SDL_Scancode key) {
	for (ScreenKey *sk : screenKeys) {
		if (sk->key == key && sk->isModal()) {
			sk->lastDown = 0;
			sk->inFirstDown = true;
			sk->wasFirstDelay = false;
		}
	}
}
void ScreenKey::setUpState(const SDL_Scancode key) {
	if (key == SDL_SCANCODE_SPACE) {
		notReactOnSpace = false;
	}else
	if (key == SDL_SCANCODE_RETURN) {
		notReactOnEnter = false;
	}

	for (ScreenKey *sk : screenKeys) {
		if (sk->key == key) {
			sk->lastDown = 0;
			sk->prevIsDown = false;
		}
	}
}

void ScreenKey::calculateProps() {
	if (!isModal()) return;

	if (lastUpdate != GV::numUpdate - 1) {
		prevIsDown = false;
		inFirstDown = false;
	}
	lastUpdate = GV::numUpdate;
	ScreenChild::calculateProps();

	if (propWasChanged[ScreenProp::KEY]) {
		propWasChanged[ScreenProp::KEY] = false;

		const String keyName = PyUtils::getStr(propValues[ScreenProp::KEY]);
		const int start = keyName.startsWith("K_", false) ? 2 : 0;
		key = SDL_GetScancodeFromName(keyName.c_str() + start);

		lastDown = 0;
		prevIsDown = false;
		inFirstDown = false;
		wasFirstDelay = false;
	}
	if (propWasChanged[ScreenProp::FIRST_DELAY]) {
		propWasChanged[ScreenProp::FIRST_DELAY] = false;

		const py::object &firstKeyDelayObj = propValues[ScreenProp::FIRST_DELAY];
		const bool isInt = PyUtils::isInt(firstKeyDelayObj);
		const bool isFloat = !isInt && PyUtils::isFloat(firstKeyDelayObj);

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

		const py::object &keyDelayObj = propValues[ScreenProp::DELAY];
		const bool isInt = PyUtils::isInt(keyDelayObj);
		const bool isFloat = !isInt && PyUtils::isFloat(keyDelayObj);

		if (isInt || isFloat) {
			keyDelay = PyUtils::getDouble(keyDelayObj, isFloat) * 1000;
		}else {
			keyDelay = 10;
			Utils::outMsg("ScreenKey::calculateProps",
						  "delay is not a number (" + PyUtils::getStr(keyDelayObj) + ")");
		}
	}


	if (key == SDL_SCANCODE_UNKNOWN) {
		const String keyName = PyUtils::getStr(propValues[ScreenProp::KEY]);
		Utils::outMsg("SDL_GetScancodeFromName",
					  "KeyName <" + keyName + ">\n" +
					  SDL_GetError() + '\n' +
					  node->getPlace());
	}else {
		if ((key == SDL_SCANCODE_SPACE && notReactOnSpace) || (key == SDL_SCANCODE_RETURN && notReactOnEnter)) return;

		if ((GV::keyBoardState && GV::keyBoardState[key]) || inFirstDown) {
			const int dTime = GV::frameStartTime - lastDown;
			const int delay = !wasFirstDelay ? firstKeyDelay : keyDelay;

			if (dTime >= delay) {
				if (!inFirstDown) {
					wasFirstDelay = true;
				}
				inFirstDown = false;
				lastDown = GV::frameStartTime;

				const String action = node->getPropCode("action").pyExpr;
				if (action) {
					PyUtils::exec(getFileName(), getNumLine(), "exec_funcs(" + action + ")");
				}
			}
			prevIsDown = true;
		}

		else {
			lastDown = 0;
			prevIsDown = false;
			inFirstDown = false;
		}
	}
}
