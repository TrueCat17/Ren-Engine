#ifndef GV_H
#define GV_H

#include <memory>
#include <mutex>
#include <vector>

#include <SDL2/SDL.h>


class Group;
class ScreenChild;

class Node;
class PyUtils;


typedef std::shared_ptr<SDL_Texture> TexturePtr;
struct RenderStruct {
	TexturePtr texture;
	Uint8 alpha;

	bool srcRectIsNull;
	bool dstRectIsNull;
	bool centerIsNull;

	SDL_Rect srcRect;
	SDL_Rect dstRect;

	double angle;
	SDL_Point center;

	bool operator==(const RenderStruct &o) const {
		return  texture == o.texture &&
				alpha == o.alpha &&
				srcRectIsNull == o.srcRectIsNull &&
				dstRectIsNull == o.dstRectIsNull &&
				centerIsNull == o.centerIsNull &&
				srcRect.x == o.srcRect.x && srcRect.y == o.srcRect.y && srcRect.w == o.srcRect.w && srcRect.h == o.srcRect.h &&
				dstRect.x == o.dstRect.x && dstRect.y == o.dstRect.y && dstRect.w == o.dstRect.w && dstRect.h == o.dstRect.h &&
				angle == o.angle &&
				center.x == o.center.x && center.y == o.center.y;
	}
	bool operator!=(const RenderStruct &o) const {
		return !(*this == o);
	}
};


class GV {
public:
	static int width;
	static int height;

	static size_t numFor;
	static size_t numScreenFor;

	static bool inGame;
	static bool exit;

	static PyUtils *pyUtils;

	static SDL_Window *mainWindow;
	static SDL_Renderer *mainRenderer;

	static const Uint8 *keyBoardState;

	static Group *screens;

	static std::mutex updateMutex;
	static std::mutex renderMutex;

	static std::mutex toRenderMutex;
	static std::vector<RenderStruct> toRender;

	static Node *mainExecNode;
};

#endif // GV_H
