#include "key.h"

#include "gv.h"
#include "media/py_utils.h"
#include "parser/node.h"
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
	const bool isModal = this->isModal();
	for (size_t i = 0; i < screenKeys.size(); ++i) {
		Key *sk = screenKeys[i];

		if (screenKeys[i] == this) {
			screenKeys.erase(screenKeys.begin() + int(i));
			--i;
		}else
		if (isModal && prevIsDown && sk->key == key && !sk->isModal()) {
			sk->lastDown = Utils::getTimer();
			sk->inFirstDown = true;
			sk->wasFirstDelay = false;
		}
	}
}

void Key::setToNotReact(const SDL_Scancode key) {
	if (key == SDL_SCANCODE_SPACE) {
		notReactOnSpace = true;
	}else
	if (key == SDL_SCANCODE_RETURN) {
		notReactOnEnter = true;
	}
}
void Key::setFirstDownState(const SDL_Scancode key) {
	for (Key *sk : screenKeys) {
		if (sk->key == key && sk->isModal()) {
			sk->lastDown = 0;
			sk->inFirstDown = true;
			sk->wasFirstDelay = false;
		}
	}
}
void Key::setUpState(const SDL_Scancode key) {
	if (key == SDL_SCANCODE_SPACE) {
		notReactOnSpace = false;
	}else
	if (key == SDL_SCANCODE_RETURN) {
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
		const int start = first_param.startsWith("K_", false) ? 2 : 0;
		key = SDL_GetScancodeFromName(first_param.c_str() + start);

		lastDown = 0;
		prevIsDown = false;
		inFirstDown = false;
		wasFirstDelay = false;
	}

	if (key == SDL_SCANCODE_UNKNOWN) {
		Utils::outMsg("SDL_GetScancodeFromName",
					  "KeyName <" + first_param + ">\n" +
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

				const Node *action = node->getProp("action");
				if (action) {
					PyUtils::exec(action->getFileName(), action->getNumLine(), "exec_funcs(" + action->params + ")");
				}else {
					const Node *style = node->getProp("style");
					const String &styleName = style ? style->params : node->command;
					PyUtils::exec(getFileName(), getNumLine(),
								  "exec_funcs(style." + styleName + ".action)");
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
