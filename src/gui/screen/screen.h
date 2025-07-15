#ifndef SCREEN_H
#define SCREEN_H

#include <float.h>

#include "container_box.h"

class Screen: public ContainerBox {
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

	static void addToShow(std::string name,
	                      const std::string &fileName, uint32_t numLine,
	                      PyObject *args = nullptr, PyObject *kwargs = nullptr);
	static void addToHide(std::string name, const std::string &fileName, uint32_t numLine);
	static bool hasScreen(std::string name);

	static void logScreenCode(std::string name);

	static void allowArrowsForCalcedScreen(const std::string &fileName, uint32_t numLine);


	float zorder = FLT_MAX;//set new (not inited) screen on top with sorting by zorder
	bool modal = false;
	bool allowArrows = false;
	bool ignore_modal = false;
	bool save = true;

	Screen(Node *node, Screen *screen);
	virtual ~Screen() { destroyedScreenIsModal = hasModal() && modal; }

	void calcProps();

	const std::string& getName() const { return name; }
};

#endif // SCREEN_H
