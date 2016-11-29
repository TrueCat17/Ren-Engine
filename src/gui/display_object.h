#ifndef DISPLAY_OBJECT_H
#define DISPLAY_OBJECT_H

#include <vector>

#include <SDL2/SDL.h>

#include "gv.h"
#include "utils/string.h"

class Group;

class DisplayObject {
protected:
	int globalX = 0;
	int globalY = 0;

	SDL_Rect rect;

public:
	static std::vector<DisplayObject*> objects;
	static bool useTexture(SDL_Texture *texture);
	static void destroyAll();

	bool enable = true;
	virtual bool enabled() const { return enable; }

	SDL_Texture* texture = nullptr;

	Group *parent = nullptr;

	DisplayObject();
	virtual ~DisplayObject();

	void removeFromParent();

	int getGlobalX() const { return globalX; }
	int getGlobalY() const { return globalY; }

	virtual void updateGlobalX();
	virtual void updateGlobalY();

	virtual int getMinX() const;
	virtual int getMinY() const;
	virtual int getMaxX() const;
	virtual int getMaxY() const;

	int getWidth() const { return enabled() ? getMaxX() - getMinX() : 0; }
	int getHeight() const { return enabled() ? getMaxY() - getMinY() : 0; }

	virtual int getX() const { return rect.x; }
	virtual int getY() const { return rect.y; }

	virtual void setX(int value) { rect.x = value; updateGlobalX(); }
	virtual void setY(int value) { rect.y = value; updateGlobalY(); }

	void setPos(int x, int y);
	void getSize(int &w, int &h) const;
	void setSize(int w, int h);

	virtual bool checkAlpha(int x, int y) const;
	virtual void draw() const;

	virtual SDL_Texture* getTextureIfOne(size_t &count) const;
};

#endif // DISPLAY_OBJECT_H
