#ifndef SCREEN_H
#define SCREEN_H

#include <vector>

#include "gui/screen/screen_container.h"

#include "utils/string.h"

class Screen: public ScreenContainer {
private:
	static std::vector<Node*> declared;

	static bool _hasModal;

	static std::vector<String> toShowList;
	static std::vector<String> toHideList;
	static void show(const String &name);
	static void hide(const String &name);


	double _zOrder = 0;
	bool _isModal = false;

	String name;

	void updateScreenProps();

public:
	static void declare(Node *node) { declared.push_back(node); }
	static Node* getDeclared(const String &name);
	static void clear() { declared.clear(); }

	static Screen* getMain(const String &name);

	static void updateLists();
	static void updateScreens();
	static bool hasModal() { return _hasModal; }

	static void addToShow(const std::string &name) { toShowList.push_back(name); }
	static void addToHide(const std::string &name) { toHideList.push_back(name); }
	static bool hasScreen(const std::string &name) { return getMain(name); }


	Screen(Node *node);

	const String& getName() const { return name; }
	double zOrder() const { return _zOrder; }
	bool screenIsModal() const { return _isModal; }
};

#endif // SCREEN_H
