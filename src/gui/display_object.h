#ifndef DISPLAY_OBJECT_H
#define DISPLAY_OBJECT_H

#include <vector>

#include "utils/image_typedefs.h"

class Group;

class DisplayObject {
protected:
	//float for exact numbers, because zoom
	SDL_FRect rect = { 0, 0, -1, -1 };
	SDL_FRect clipRect = { 0, 0, 0, 0 };

	float globalX = 0;
	float globalY = 0;

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

	static SDL_Rect buildIntRect(float x, float y, float w, float h, bool exactSize);

	bool clipping = false;
	bool enable = true;

	SDL_Rect crop = { 0, 0, 0, 0 };//int, because zoom does not work with crop
	float alpha = 1;

	float rotate = 0;

	//center for rotation
	float xAnchor = 0;
	float yAnchor = 0;

	float xzoom = 1;
	float yzoom = 1;

	SurfacePtr surface = nullptr;

	Group *parent = nullptr;


	DisplayObject();
	virtual ~DisplayObject();

	void removeFromParent();

	float getGlobalX() const { return globalX; }
	float getGlobalY() const { return globalY; }
	float getGlobalZoomX() const { return globalZoomX; }
	float getGlobalZoomY() const { return globalZoomY; }
	float getGlobalRotate() const { return globalRotate; }
	float getGlobalAlpha() const { return globalAlpha; }
	virtual void updateGlobal();

	float getX() const { return rect.x; }
	float getY() const { return rect.y; }
	float getWidth() const { return rect.w; }
	float getHeight() const { return rect.h; }

	void setX(float value) { rect.x = value; }
	void setY(float value) { rect.y = value; }
	void setWidth(float value) { rect.w = value; }
	void setHeight(float value) { rect.h = value; }

	virtual bool checkAlpha(int x, int y) const;
	virtual void draw() const;
};

#endif // DISPLAY_OBJECT_H
