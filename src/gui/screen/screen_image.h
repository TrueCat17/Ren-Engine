#ifndef SCREENIMAGE_H
#define SCREENIMAGE_H

#include "screen_container.h"

class ScreenImage: public ScreenContainer {
private:
	String imageCode;
	String imagePath;

protected:
	virtual double getDefaultWidth() const { return -1; }
	virtual double getDefaultHeight() const { return -1; }

public:
	ScreenImage(Node *node);

	virtual void updateProps();
	virtual void update();
	virtual void draw() const;
};

#endif // SCREENIMAGE_H
