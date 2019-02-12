#ifndef RENDERER_H
#define RENDERER_H

#include <mutex>
#include <vector>

#include "utils/image_typedefs.h"

struct RenderStruct {
	SurfacePtr surface;
	int angle;
	Uint8 alpha;

	SDL_Rect srcRect;
	SDL_Rect dstRect;
	SDL_Point center;

	bool operator==(const RenderStruct &o) const {
		return  surface == o.surface &&
				angle == o.angle &&
				alpha == o.alpha &&
				srcRect.x == o.srcRect.x && srcRect.y == o.srcRect.y && srcRect.w == o.srcRect.w && srcRect.h == o.srcRect.h &&
				dstRect.x == o.dstRect.x && dstRect.y == o.dstRect.y && dstRect.w == o.dstRect.w && dstRect.h == o.dstRect.h &&
				center.x == o.center.x && center.y == o.center.y;
	}
	bool operator!=(const RenderStruct &o) const {
		return !(*this == o);
	}
};

class Renderer {
public:
	static int maxTextureWidth;
	static int maxTextureHeight;

	static bool needToRender;
	static bool needToRedraw;

	static std::mutex renderMutex;

	static std::mutex toRenderMutex;
	static std::vector<RenderStruct> toRender;

	static bool init();

	static void needMakeScreenshot();
	static SurfacePtr getScreenshot();

	static SurfacePtr getScaled(const SurfacePtr &src, int width, int height);
};

#endif // RENDERER_H
