#include "key.h"

#include <SDL2/SDL_keyboard.h>

#include "gv.h"

#include "screen.h"
#include "style.h"

#include "media/py_utils.h"
#include "utils/math.h"
#include "utils/string.h"
#include "utils/utils.h"


bool Key::notReactOnSpace = false;
bool Key::notReactOnEnter = false;
std::vector<Key*> Key::screenKeys;

Key::Key(Node *node, Screen *screen):
	Child(node, nullptr, screen)
{
	screenKeys.push_back(this);
}
Key::~Key() {
	const bool isModal = Screen::destroyedScreenIsModal;
	for (size_t i = 0; i < screenKeys.size(); ++i) {
		Key *sk = screenKeys[i];

		if (screenKeys[i] == this) {
			screenKeys.erase(screenKeys.begin() + long(i));
			--i;
		}else
		if (isModal && prevIsDown && screen != sk->screen &&
		    sk->key == key && !sk->isModal())
		{
			sk->lastDown = Utils::getTimer();
			sk->inFirstDown = true;
			sk->wasFirstDelay = false;
		}
	}
}

void Key::setToNotReact(const SDL_Keycode key) {
	if (key == SDLK_SPACE) {
		notReactOnSpace = true;
	}else if (key == SDLK_RETURN) {
		notReactOnEnter = true;
	}
}
void Key::setFirstDownState(const SDL_Keycode key) {
	GV::keyBoardState[key & ~SDLK_SCANCODE_MASK] = true;

	for (Key *sk : screenKeys) {
		if (sk->key == key && sk->isModal()) {
			sk->lastDown = 0;
			sk->inFirstDown = true;
			sk->wasFirstDelay = false;
		}
	}
}
void Key::setUpState(const SDL_Keycode key) {
	GV::keyBoardState[key & ~SDLK_SCANCODE_MASK] = false;

	if (key == SDLK_SPACE) {
		notReactOnSpace = false;
	}else
	if (key == SDLK_RETURN) {
		notReactOnEnter = false;
	}

	for (Key *sk : screenKeys) {
		if (sk->key == key) {
			sk->lastDown = 0;
			sk->prevIsDown = false;
		}
	}
}

void Key::updateRect(bool) {}

void Key::checkEvents() {
	if (!isModal()) return;

	if (lastUpdate != GV::numUpdate - 1) {
		prevIsDown = false;
		inFirstDown = false;
	}
	lastUpdate = GV::numUpdate;

	if (first_param != prevKeyName) {
		prevKeyName = first_param;

		const int start = String::startsWith(first_param, "K_", true) ? 2 : 0;
		key = SDL_GetKeyFromName(first_param.c_str() + start);

		lastDown = 0;
		prevIsDown = false;
		inFirstDown = false;
		wasFirstDelay = false;
	}

	if (key == SDLK_UNKNOWN) {
		Utils::outMsg("SDL_GetScancodeFromName",
					  "KeyName <" + first_param + ">\n" +
					  SDL_GetError() + '\n' +
					  node->getPlace());
	}else {
		if ((key == SDLK_SPACE && notReactOnSpace) || (key == SDLK_RETURN && notReactOnEnter)) return;

		if (inFirstDown || GV::keyBoardState[key & ~SDLK_SCANCODE_MASK]) {
			const double dTime = GV::frameStartTime - lastDown;
			const double delay = !wasFirstDelay ? firstKeyDelay : keyDelay;

			if (dTime >= delay || Math::doublesAreEq(lastDown, 0.0)) {
				if (!inFirstDown) {
					wasFirstDelay = true;
				}
				inFirstDown = false;
				lastDown = GV::frameStartTime;

				const Node *action = node->getProp("action");
				if (action) {
					PyUtils::exec(action->getFileName(), action->getNumLine(), "exec_funcs(" + action->params + ")");
				}else {
					Style::execAction(node->getFileName(), node->getNumLine(), style, "action");
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
