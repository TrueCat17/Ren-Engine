#ifndef SCREEN_H
#define SCREEN_H

#include "gui/screen/container.h"

class Screen: public Container {
private:
	PyObject *co;

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

	static void replace(const std::string &fromName, const std::string &toName);

	static void addToShow(std::string name, PyObject *args = nullptr, PyObject *kwargs = nullptr);
	static void addToHide(std::string name);
	static bool hasScreen(std::string name);

	static void logScreenCode(std::string name);


	double zorder = 0;
	bool modal = false;
	bool ignore_modal = false;
	bool save = true;

	Screen(Node *node, Screen *screen);
	virtual ~Screen() { destroyedScreenIsModal = hasModal() && modal; }

	void calcProps();

	const std::string& getName() const { return name; }
};

#endif // SCREEN_H
