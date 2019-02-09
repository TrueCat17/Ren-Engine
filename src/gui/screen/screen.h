#ifndef SCREEN_H
#define SCREEN_H

#include <vector>
#include <mutex>

#include "gui/screen/container.h"

#include "utils/string.h"

class Screen: public Container {
private:
	PyCodeObject *co;

	String name;
	String screenCode;

public:
	static void declare(Node *node);
	static Node* getDeclared(const String &name);
	static void clear();

	static Screen* getMain(const String &name);

	static void updateLists();
	static void updateScreens();
	static bool hasModal();

	static void checkScreenEvents();
	static void errorProcessing();

	static void addToShow(const std::string &name);
	static void addToHide(const std::string &name);
	static bool hasScreen(const std::string &name);


	double zorder = 0;
	bool modal = false;

	Screen(Node *node, Screen *screen);

	void calcProps();

	const String& getName() const { return name; }
};

#endif // SCREEN_H
