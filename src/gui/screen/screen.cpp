#include "screen.h"

#include "screen_window.h"

#include "gv.h"
#include "utils/utils.h"

std::vector<Node*> Screen::declared;
std::vector<Screen*> Screen::created;

Node* Screen::getDeclared(const String& name) {
	for (Node *node : declared) {
		if (node->name == name) {
			return node;
		}
	}
	return nullptr;
}
Screen* Screen::getCreated(const String& name) {
	for (size_t i = 0; i < created.size(); ++i) {
		Screen *scr = created[i];
		if (scr->name == name) {
			return scr;
		}
	}
	return nullptr;
}

Screen* Screen::show(const String& name, bool depended) {
	Screen *scr = getCreated(name);
	if (scr) {
		scr->addShowedCount(depended);
		scr->parent->addChild(scr);//Наверх в списке потомков своего родителя
		return scr;
	}

	Node *node = getDeclared(name);
	if (!node) {
		Utils::outMsg("Screen::show", "Скрин с именем <" + name + "> не существует");
		return nullptr;
	}

	scr = new Screen(node, depended);
	scr->name = name;
	GV::screens->addChild(scr);

	return scr;
}
void Screen::hide(const String& name, bool depended) {
	if (!getDeclared(name)) {
		Utils::outMsg("Screen::hide", "Скрин с именем <" + name + "> не существует");
		return;
	}

	for (size_t i = 0; i < created.size(); ++i) {
		Screen *scr = created[i];
		if (scr->name == name) {
			if (!scr->subShowedCount(depended)) {
				static size_t depth = 0;
				++depth;

				bool inGame = GV::inGame;
				if (inGame && depth == 1) GV::updateGuard.lock(), GV::renderGuard.lock();
				delete scr;
				if (inGame && depth == 1) GV::updateGuard.unlock(), GV::renderGuard.unlock();

				--depth;
			}
			return;
		}
	}

	Utils::outMsg("Screen::hide", "Скрин с именем <" + name + "> не отображается, поэтому его нельзя скрыть");
}

void Screen::declare(Node *node) {
	declared.push_back(node);
}
void Screen::clear() {
	declared.clear();
}


void Screen::addShowedCount(bool depended) {
	if (depended) {
		countAsUsed += 1;
	}else {
		countAsMain += 1;
	}
}
bool Screen::subShowedCount(bool depended) {
	if (depended) {
		countAsUsed -= 1;
	}else {
		if (countAsMain) {
			countAsMain -= 1;
		}else {
			String extra = countAsUsed == 1 ? "другом скрине" : "других скринах (в " + String(int(countAsUsed)) + ")";
			Utils::outMsg("Screen::subShowedCount(depended == false)",
						  "Скрин с именем <" + name + "> нельзя скрыть командой hide,\n"
						  "потому что он отображается командой use в " + extra);
		}
	}
	return countAsMain || countAsUsed;
}

Screen::Screen(Node *node, bool depended): ScreenContainer(node, nullptr) {
	created.push_back(this);
	addShowedCount(depended);
}
Screen::~Screen() {
	for (size_t i = 0; i < created.size(); ++i) {
		Screen *scr = created[i];
		if (scr->name == name) {
			created.erase(created.begin() + i);
			break;
		}
	}
}
