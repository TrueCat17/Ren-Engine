#ifndef SCREENNULL_H
#define SCREENNULL_H

#include "screen_container.h"

class ScreenNull: public ScreenContainer {
public:
	ScreenNull(Node* node, Screen *screen);
};

#endif // SCREENNULL_H
