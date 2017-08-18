#include "screen.h"

#include "gv.h"
#include "media/py_utils.h"

std::vector<Node*> Screen::declared;
std::vector<Screen*> Screen::created;

std::vector<std::pair<String, String>> Screen::toShowList;
std::vector<std::pair<String, String>> Screen::toHideList;

Node* Screen::getDeclared(const String &name) {
	for (Node *node : declared) {
		if (node->name == name) {
			return node;
		}
	}
	return nullptr;
}
Screen* Screen::getCreated(const String &name) {
	for (size_t i = 0; i < created.size(); ++i) {
		Screen *scr = created[i];
		if (scr->name == name) {
			return scr;
		}
	}
	return nullptr;
}


void Screen::updateLists() {
	for (size_t i = 0 ; i < toHideList.size(); ++i) {
		auto &p = toHideList[i];
		const String &name = p.first;
		const String &dependOn = p.second;
		hide(name, dependOn);
	}
	toHideList.clear();

	for (size_t i = 0 ; i < toShowList.size(); ++i) {
		auto &p = toShowList[i];
		const String &name = p.first;
		const String &dependOn = p.second;
		show(name, dependOn);
	}
	toShowList.clear();
}


void Screen::addToShowSimply(const std::string &name) {
	Screen::addToShow(name, "");
}
void Screen::addToShow(const String &name, const String &dependsOn) {
	toShowList.push_back(std::make_pair(name, dependsOn));
}

void Screen::show(const String &name, const String &dependOn) {
	Screen *scr = getCreated(name);
	if (scr) {
		scr->addShowedCount(dependOn);
		scr->parent->addChild(scr);//Наверх в списке потомков своего родителя
		return;
	}

	Node *node = getDeclared(name);
	if (!node) {
		Utils::outMsg("Screen::show", "Скрин с именем <" + name + "> не существует");
		return;
	}

	scr = new Screen(node, dependOn);
	scr->name = name;
	GV::screens->addChild(scr);

	scr->calculateProps();
}

void Screen::addToHideSimply(const std::string &name) {
	Screen::addToHide(name, "");
}
void Screen::addToHide(const String &name, const String &dependsOn) {
	toHideList.push_back(std::make_pair(name, dependsOn));
}

void Screen::hide(const String &name, const String &dependOn) {
	if (!getDeclared(name)) {
		Utils::outMsg("Screen::hide", "Скрин с именем <" + name + "> не существует");
		return;
	}

	for (size_t i = 0; i < created.size(); ++i) {
		Screen *scr = created[i];
		if (scr->name == name) {
			if (!scr->subShowedCount(dependOn)) {
				delete scr;
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


void Screen::addShowedCount(const String &dependOn) {
	if (dependOn) {
		auto i = std::find_if(created.begin(), created.end(),
								[&dependOn](Screen *scr) { return scr->name == dependOn; }
				 );
		if (i != created.end()) {
			Screen *scr = *i;
			scr->usedScreens.push_back(this);
		}

		countAsUsed += 1;
	}else {
		countAsMain += 1;
	}
}
bool Screen::subShowedCount(const String &dependOn) {
	if (dependOn) {
		countAsUsed -= 1;
	}else {
		if (countAsMain) {
			countAsMain -= 1;
		}else {
			String extra = countAsUsed == 1 ? "другом скрине" : ("других скринах (в " + String(int(countAsUsed)) + ")");
			Utils::outMsg("Screen::subShowedCount(!depended)",
						  "Скрин с именем <" + name + "> нельзя скрыть командой hide,\n"
						  "потому что он отображается командой use в " + extra);
		}
	}
	return countAsMain || countAsUsed;
}



Screen::Screen(Node *node, const String &dependOn):
	ScreenContainer(node, this)
{
	created.push_back(this);
	addShowedCount(dependOn);

	setProp("modal", node->getPropCode("modal"));
	setProp("zorder", node->getPropCode("zorder"));
}
Screen::~Screen() {
	for (size_t i = 0; i < created.size(); ++i) {
		Screen *scr = created[i];
		if (scr->name == name) {
			created.erase(created.begin() + i);
			break;
		}
	}

	for (Screen *scr : usedScreens) {
		if (!scr->subShowedCount(name)) {
			delete scr;
		}
	}
	usedScreens.clear();
}


bool Screen::_hasModal = false;
void Screen::updateScreens() {
	for (Screen *w : created) {
		w->updateScreenProps();
	}

	_hasModal = false;
	for (Screen *w : created) {
		if (w->screenIsModal()) {
			_hasModal = true;
			break;
		}
	}

	auto zOrderCmp = [](DisplayObject *a, DisplayObject *b) -> bool {
		Screen *sA = dynamic_cast<Screen*>(a);
		Screen *sB = dynamic_cast<Screen*>(b);
		double zA = sA ? sA->zOrder() : 0;
		double zB = sB ? sB->zOrder() : 0;
		return zA < zB;
	};
	std::sort(GV::screens->children.begin(), GV::screens->children.end(), zOrderCmp);
}
void Screen::updateScreenProps() {
	needUpdateChildren = false;
	xSize = GV::width;
	ySize = GV::height;
	calculateProps();
	needUpdateChildren = true;

	_isModal = propValues.at("modal") == "True";
	_zOrder = propValues.at("zorder").toDouble();
}
