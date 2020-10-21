#ifndef SCREEN_H
#define SCREEN_H

#include "gui/screen/container.h"

class Screen: public Container {
private:
	void *co;

	std::string name;
	std::string screenCode;

public:
	static bool destroyedScreenIsModal;


	static void declare(Node *node);
	static Node* getDeclared(const std::string &name);
	static void clear();

	static Screen* getMain(const std::string &name);

	static void updateLists();
	static void updateScreens();
	static bool hasModal();

	static void checkScreenEvents();

	static void addToShow(const std::string &name);
	static void addToHide(const std::string &name);
	static bool hasScreen(const std::string &name);

	static void logScreenCode(const std::string &name);


	double zorder = 0;
	bool modal = false;

	Screen(Node *node, Screen *screen);
	virtual ~Screen() { destroyedScreenIsModal = hasModal() && modal; }

	void calcProps();

	const std::string& getName() const { return name; }
};

#endif // SCREEN_H
