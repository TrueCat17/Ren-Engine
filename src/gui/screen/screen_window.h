#ifndef SCREEN_WINDOW_H
#define SCREEN_WINDOW_H

#include "gui/screen/screen_container.h"
#include "parser/node.h"

class ScreenWindow: public ScreenContainer {
private:
	static std::vector<ScreenWindow*> created;
	static bool _hasModal;

	bool _isModal = false;

public:
	static void updateModality();
	static bool hasModal() { return _hasModal; }

	ScreenWindow(Node *node);
	virtual ~ScreenWindow();

	int getMinX() const;
	int getMinY() const;
	int getMaxX() const;
	int getMaxY() const;

	void updateModalProp();
	bool isModal() const { return _isModal; }
};

#endif // SCREEN_WINDOW_H
