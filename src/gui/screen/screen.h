#ifndef SCREEN_H
#define SCREEN_H

#include <vector>

#include "gui/screen/screen_container.h"

#include "utils/string.h"

class Screen: public ScreenContainer {
private:
	static std::vector<Node*> declared;
	static Node* getDeclared(const String &name);

	static std::vector<Screen*> created;
	static bool _hasModal;

	static std::vector<std::pair<String, String>> toShowList;
	static std::vector<std::pair<String, String>> toHideList;
	static void show(const String &name, const String &dependOn);
	static void hide(const String &name, const String &dependOn);

	size_t countAsMain = 0;
	size_t countAsUsed = 0;
	std::vector<Screen*> usedScreens;

	void addShowedCount(const String &dependOn);
	bool subShowedCount(const String &dependOn);
	void updateModalProp();

	bool _isModal = false;

	Screen(Node *node, const String &dependOn);
	virtual ~Screen();

public:
	static void declare(Node *node);
	static Screen* getCreated(const String &name);
	static void clear();

	static void updateLists();
	static void updateModality();
	static bool hasModal() { return _hasModal; }

	static void addToShowSimply(const std::string &name);
	static void addToHideSimply(const std::string &name);
	static void addToShow(const String &name, const String &dependsOn);
	static void addToHide(const String &name, const String &dependsOn);

	String name;
	bool screenIsModal() const { return _isModal; }
};

#endif // SCREEN_H
