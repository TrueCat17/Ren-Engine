#ifndef SCREEN_WINDOW_H
#define SCREEN_WINDOW_H

#include "gui/screen/screen_container.h"
#include "parser/node.h"

class ScreenWindow: public ScreenContainer {
public:
	ScreenWindow(Node *node);

	int getMinX() const;
	int getMinY() const;
	int getMaxX() const;
	int getMaxY() const;
};

#endif // SCREEN_WINDOW_H
