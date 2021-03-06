#ifndef DISPLAY_OBJECT_H
#define DISPLAY_OBJECT_H

#include <vector>

#include "utils/image_typedefs.h"

class Group;

class DisplayObject {
protected:
	SDL_Rect rect = {0, 0, -1, -1};
	SDL_Rect clipRect = {0, 0, 0, 0};

	int globalX = 0;
	int globalY = 0;

	float globalRotate = 0;
	float globalAlpha = 1;

	float globalZoomX = 1;
	float globalZoomY = 1;

	bool globalClipping = false;

public:
	static std::vector<DisplayObject*> objects;
	static void disableAll();
	static void destroyAll();

	static void pushToRender(const SurfacePtr &surface, float angle, Uint8 alpha, bool clipping,
	                         const SDL_Rect &clipRect, const SDL_Rect &srcRect, const SDL_Rect &dstRect, const SDL_Point center);

	bool clipping = false;
	bool enable = true;

	SDL_Rect crop = {0, 0, 0, 0};
	float alpha = 1;

	float rotate = 0;

	//center for rotation
	int xAnchor = 0;
	int yAnchor = 0;

	float xzoom = 1;
	float yzoom = 1;

	SurfacePtr surface = nullptr;

	Group *parent = nullptr;


	DisplayObject();
	virtual ~DisplayObject();

	void removeFromParent();

	int getGlobalX() const { return globalX; }
	int getGlobalY() const { return globalY; }
	float getGlobalZoomX() const { return globalZoomX; }
	float getGlobalZoomY() const { return globalZoomY; }
	float getGlobalRotate() const { return globalRotate; }
	float getGlobalAlpha() const { return globalAlpha; }
	virtual void updateGlobal();

	int getX() const { return rect.x; }
	int getY() const { return rect.y; }
	int getWidth() const { return rect.w; }
	int getHeight() const { return rect.h; }

	void setX(int value) { rect.x = value; }
	void setY(int value) { rect.y = value; }
	void setWidth(int value) { rect.w = value; }
	void setHeight(int value) { rect.h = value; }

	virtual bool checkAlpha(int x, int y) const;
	virtual void draw() const;
};

#endif // DISPLAY_OBJECT_H
