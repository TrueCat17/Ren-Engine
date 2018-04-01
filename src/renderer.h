#ifndef RENDERER_H
#define RENDERER_H

#include <mutex>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "gv.h"


typedef std::shared_ptr<SDL_Surface> SurfacePtr;
typedef std::shared_ptr<SDL_Texture> TexturePtr;

struct RenderStruct {
	SurfacePtr surface;
	float angle;
	Uint8 alpha;

	bool srcRectIsNull;
	bool dstRectIsNull;
	bool centerIsNull;

	SDL_Rect srcRect;
	SDL_Rect dstRect;
	SDL_Point center;

	bool operator==(const RenderStruct &o) const {
		return  surface == o.surface &&
				angle == o.angle &&
				alpha == o.alpha &&
				srcRectIsNull == o.srcRectIsNull &&
				dstRectIsNull == o.dstRectIsNull &&
				centerIsNull == o.centerIsNull &&
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
	static SurfacePtr getScreenshot(size_t width, size_t height);
	static SurfacePtr getScaled(const SurfacePtr &src, int width, int height);

private:
	static SDL_Texture *tmpTexture;

	static bool screenshotting;
	static size_t screenshotWidth;
	static size_t screenshotHeight;
	static SurfacePtr screenshot;
	static void readPixels();

	static bool scaled;
	static SurfacePtr toScaleSurface;
	static SurfacePtr scaledSurface;
	static int scaleWidth;
	static int scaleHeight;
	static void scale();

	static void loop();
	static GLuint queryTexture(SDL_Texture *texture, int &width, int &height);
	static void checkErrors(const char *from, const char *funcName);

	static void renderWithOpenGL(SDL_Texture *texture, Uint8 alpha,
								 int angle, const SDL_Point *center,
								 const SDL_Rect *src, const SDL_Rect *dst);
};

#endif // RENDERER_H
