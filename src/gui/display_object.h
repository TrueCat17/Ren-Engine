#ifndef DISPLAY_OBJECT_H
#define DISPLAY_OBJECT_H

#include <memory>
#include <vector>

#include <SDL2/SDL.h>

#include "utils/utils.h"

class Group;

class DisplayObject {
protected:
	int globalX = 0;
	int globalY = 0;
	int globalRotate = 0;

	double alpha = 1;
	double globalAlpha = 1;

	SDL_Rect rect;
	SDL_Rect crop;

public:
	static std::vector<DisplayObject*> objects;
	static void destroyAll();


	bool enable = true;

	TexturePtr texture = nullptr;

	Group *parent = nullptr;


	int rotate = 0;

	//center for rotation
	double xAnchor = 0;
	double yAnchor = 0;


	DisplayObject();
	virtual ~DisplayObject();

	void removeFromParent();

	const SDL_Rect& getDrawRect() const { return rect; }
	const SDL_Rect& getCropRect() const { return crop; }

	int getGlobalX() const { return globalX; }
	int getGlobalY() const { return globalY; }
	int getGlobalRotate() const { return globalRotate; }
	virtual void updateGlobalPos();

	double getGlobalAlpha() const { return globalAlpha; }
	virtual void updateGlobalAlpha();

	int getX() const { return rect.x; }
	int getY() const { return rect.y; }
	double getAlpha() const { return alpha; }

	void setX(int value) { rect.x = value; }
	void setY(int value) { rect.y = value; }
	void steAlpha(double value) { alpha = value; }

	int getWidth() const { return rect.w; }
	int getHeight() const { return rect.h; }

	void setPos(int x, int y);
	void setSize(int w, int h);

	virtual bool checkAlpha(int x, int y) const;
	virtual void draw() const;
};

#endif // DISPLAY_OBJECT_H
