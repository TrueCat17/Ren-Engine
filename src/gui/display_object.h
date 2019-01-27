#ifndef DISPLAY_OBJECT_H
#define DISPLAY_OBJECT_H

#include <vector>

#include "utils/image_typedefs.h"

class Group;

class DisplayObject {
protected:
	int globalX = 0;
	int globalY = 0;
	int globalRotate = 0;

	double globalAlpha = 1;

	SDL_Rect rect;

public:
	static std::vector<DisplayObject*> objects;
	static void disableAll();
	static void destroyAll();

	static void pushToRender(const SurfacePtr &surface, float angle, Uint8 alpha,
							 const SDL_Rect *srcRect, const SDL_Rect *dstRect, const SDL_Point *center);

	SDL_Rect crop;
	double alpha = 1;

	bool enable = true;

	int rotate = 0;

	//center for rotation
	int xAnchor = 0;
	int yAnchor = 0;

	SurfacePtr surface = nullptr;

	Group *parent = nullptr;


	DisplayObject();
	virtual ~DisplayObject();

	void removeFromParent();

	const SDL_Rect& getDrawRect() const { return rect; }
	const SDL_Rect& getCropRect() const { return crop; }

	int getGlobalX() const { return globalX; }
	int getGlobalY() const { return globalY; }
	int getGlobalRotate() const { return globalRotate; }
	double getGlobalAlpha() const { return globalAlpha; }
	virtual void updateGlobal();

	int getX() const { return rect.x; }
	int getY() const { return rect.y; }
	int getWidth() const { return rect.w; }
	int getHeight() const { return rect.h; }
	double getAlpha() const { return alpha; }

	void setX(int value) { rect.x = value; }
	void setY(int value) { rect.y = value; }
	void setWidth(int value) { rect.w = value; }
	void setHeight(int value) { rect.h = value; }
	void setAlpha(double value) { alpha = value; }

	void setPos(int x, int y);
	void setSize(int w, int h);

	virtual bool checkAlpha(int x, int y) const;
	virtual void draw() const;
};

#endif // DISPLAY_OBJECT_H
