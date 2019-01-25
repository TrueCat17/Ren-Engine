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
