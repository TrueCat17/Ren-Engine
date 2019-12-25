#include "renderer.h"


static
bool operator==(const SDL_Rect &a, const SDL_Rect &b) {
	return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}
static
bool operator==(const SDL_Point &a, const SDL_Point &b) {
	return a.x == b.x && a.y == b.y;
}

bool RenderStruct::operator==(const RenderStruct &o) const {
	auto a = std::tie(  surface,   angle,   alpha,   clip,   clipRect,   srcRect,   dstRect,   center);
	auto b = std::tie(o.surface, o.angle, o.alpha, o.clip, o.clipRect, o.srcRect, o.dstRect, o.center);
	return a == b;
}


#include <thread>
#include <map>
#include <cmath>

#ifdef __WIN32__
    #undef _DLL
    #undef GL_NO_STDCALL
#endif
#include <SDL2/SDL_opengl.h>

#include "gv.h"
#include "config.h"
#include "gui/group.h"
#include "media/image_manipulator.h"

#include "utils/math.h"
#include "utils/image_caches.h"
#include "utils/utils.h"



bool Renderer::needToRender = false;
bool Renderer::needToRedraw = false;

std::mutex Renderer::renderMutex;

std::mutex Renderer::toRenderMutex;
std::vector<RenderStruct> Renderer::toRender;

SDL_RendererInfo Renderer::info;


static bool fastOpenGL = false;
static bool checkOpenGlErrors = false;

static GLuint queryTexture(SDL_Texture *texture, int &width, int &height) {
	if (!texture) {
		width = height = 0;
		return 0;
	}

	//copy-paste from SDL sources

	struct SDL_SW_YUVTexture;

	struct GL_FBOList;

	enum SDL_ScaleMode {
		SDL_ScaleModeNearest,
		SDL_ScaleModeLinear,
		SDL_ScaleModeBest
	};

	struct Texture {
		const void *magic;
		Uint32 format;              /**< The pixel format of the texture */
		int access;                 /**< SDL_TextureAccess */
		int w;                      /**< The width of the texture */
		int h;                      /**< The height of the texture */
		int modMode;                /**< The texture modulation mode */
		SDL_BlendMode blendMode;    /**< The texture blend mode */
		SDL_ScaleMode scaleMode;    /**< The texture scale mode */
		Uint8 r, g, b, a;           /**< Texture modulation values */

		SDL_Renderer *renderer;

		/* Support for formats not supported directly by the renderer */
		SDL_Texture *native;
		SDL_SW_YUVTexture *yuv;
		void *pixels;
		int pitch;
		SDL_Rect locked_rect;

		Uint32 last_command_generation; /* last command queue generation this texture was in. */

		void *driverdata;           /**< Driver specific texture representation */

		SDL_Texture *prev;
		SDL_Texture *next;
	};

	struct TextureData {
		GLuint texture;
		GLfloat texw;
		GLfloat texh;
		GLenum format;
		GLenum formattype;
		void *pixels;
		int pitch;
		SDL_Rect locked_rect;

		/* YUV texture support */
		SDL_bool yuv;
		SDL_bool nv12;
		GLuint utexture;
		GLuint vtexture;

		GL_FBOList *fbo;
	};

	Texture *t = reinterpret_cast<Texture*>(texture);
	if (t->native) {
		t = reinterpret_cast<Texture*>(t->native);
	}
	TextureData *data = reinterpret_cast<TextureData*>(t->driverdata);

	width = t->w;
	height = t->h;

	return data->texture;
}



static void checkErrors(const char *from, const char *funcName) {
	size_t countErrors = 0;
	const size_t maxCountErrors = 10;

	GLuint error;
	while ((error = glGetError()) != GL_NO_ERROR) {
		if (++countErrors == maxCountErrors) {
			fastOpenGL = false;
			Utils::outMsg("Renderer::" + std::string(from) + ", " + funcName, "Using OpenGL failed");
			ImageCaches::clearTextures();
			break;
		}

		const char *str = "Unknown";
		     if (error == GL_INVALID_ENUM)      str = "GL_INVALID_ENUM";
		else if (error == GL_INVALID_VALUE)     str = "GL_INVALID_VALUE";
		else if (error == GL_INVALID_OPERATION) str = "GL_INVALID_OPERATION";
		else if (error == GL_STACK_OVERFLOW)    str = "GL_STACK_OVERFLOW";
		else if (error == GL_STACK_UNDERFLOW)   str = "GL_STACK_UNDERFLOW";
		else if (error == GL_OUT_OF_MEMORY)     str = "GL_OUT_OF_MEMORY";

		Utils::outMsg("Renderer::" + std::string(from) + ", " + funcName, str);
	}
}

static void renderWithOpenGL(SDL_Texture *texture, int angle, Uint8 alpha,
                                const SDL_Rect *src, const SDL_Rect *dst, const SDL_Point *center)
{
	if (!alpha || !dst->w || !dst->h) return;

	int textureWidth, textureHeight;
	GLuint textureId = queryTexture(texture, textureWidth, textureHeight);

	SDL_Rect realSrc;
	if (src) {
		if (!src->w || !src->h) return;
		realSrc = {src->x, src->y, src->w, src->h};
	}else {
		realSrc = {0, 0, textureWidth, textureHeight};
	}

	const bool check = checkOpenGlErrors;

	glBindTexture(GL_TEXTURE_2D, textureId);
	if (check) {
		checkErrors("renderWithOpenGL", "glBindTexture");
	}
	glColor4f(1, 1, 1, GLfloat(alpha) / 255);
	if (check) {
		checkErrors("renderWithOpenGL", "glColor4f");
	}

	if (angle) {
		int dX = dst->x + (center ? center->x : 0);
		int dY = dst->y + (center ? center->y : 0);

		glPushMatrix();
		if (check) {
			checkErrors("renderWithOpenGL", "glPushMatrix");
		}
		glTranslated(dX, dY, 0);
		if (check) {
			checkErrors("renderWithOpenGL", "glTranslated");
		}
		glRotated(angle, 0, 0, 1);
		if (check) {
			checkErrors("renderWithOpenGL", "glRotated");
		}
		glTranslated(-dX, -dY, 0);
		if (check) {
			checkErrors("renderWithOpenGL", "glTranslated");
		}
	}

	float w = textureWidth;
	float h = textureHeight;

	float minX = realSrc.x / w;
	float minY = realSrc.y / h;
	float maxX = (realSrc.x + realSrc.w) / w;
	float maxY = (realSrc.y + realSrc.h) / h;

	glBegin(GL_QUADS);
	glTexCoord2f(minX, minY); glVertex2f(dst->x, dst->y);
	glTexCoord2f(minX, maxY); glVertex2f(dst->x, dst->y + dst->h);
	glTexCoord2f(maxX, maxY); glVertex2f(dst->x + dst->w, dst->y + dst->h);
	glTexCoord2f(maxX, minY); glVertex2f(dst->x + dst->w, dst->y);
	glEnd();
	if (check) {
		checkErrors("renderWithOpenGL", "glBegin(GL_QUADS)/glTexCoord2f/glVertex2f/glEnd");
	}

	if (angle) {
		glPopMatrix();
		if (check) {
			checkErrors("renderWithOpenGL", "glPopMatrix");
		}
	}
}


static SDL_Renderer *renderer = nullptr;

static SDL_Texture *tmpTexture = nullptr;
static SurfacePtr toScaleSurface;
static SurfacePtr scaledSurface;
static bool scaled = true;
static int scaleWidth = 0;
static int scaleHeight = 0;
static std::mutex scaledMutex;

SurfacePtr Renderer::getScaled(const SurfacePtr &src, int width, int height) {
	std::lock_guard g(scaledMutex);

	toScaleSurface = src;
	scaleWidth = width;
	scaleHeight = height;

	scaled = false;
	while (!scaled) {
		Utils::sleep(1);
	}
	toScaleSurface.reset();

	return scaledSurface;
}
static void scale() {
	int textureWidth, textureHeight;
	SDL_QueryTexture(tmpTexture, nullptr, nullptr, &textureWidth, &textureHeight);

	if (scaleWidth > textureWidth || scaleHeight > textureHeight) {
		SDL_DestroyTexture(tmpTexture);

		textureWidth  = int(std::ceil(scaleWidth / 256.0)) * 256;
		textureHeight = int(std::ceil(scaleHeight / 256.0)) * 256;

		tmpTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET,
									   textureWidth, textureHeight);
		if (!tmpTexture) {
			Utils::outMsg("SDL_CreateTexture", SDL_GetError());
		}
	}

	SDL_Rect dstRect = {0, 0, scaleWidth, scaleHeight};
	TexturePtr toRender = ImageCaches::getTexture(renderer, toScaleSurface);

	if (SDL_SetRenderTarget(renderer, tmpTexture)) {
		Utils::outMsg("Renderer::scale, SDL_SetRenderTarget", SDL_GetError());
	}
	if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0)) {
		Utils::outMsg("Renderer::scale, SDL_SetRenderDrawColor", SDL_GetError());
	}
	if (SDL_RenderClear(renderer)) {
		Utils::outMsg("Renderer::scale, SDL_RenderClear", SDL_GetError());
	}
	if (SDL_RenderCopy(renderer, toRender.get(), nullptr, &dstRect)) {
		Utils::outMsg("Renderer::scale, SDL_RenderCopy", SDL_GetError());
	}

	scaledSurface = ImageManipulator::getNewNotClear(scaleWidth, scaleHeight);
	if (SDL_RenderReadPixels(renderer, &dstRect, SDL_PIXELFORMAT_RGBA32, scaledSurface->pixels, scaledSurface->pitch)) {
		Utils::outMsg("Renderer::scale, SDL_RenderReadPixels", SDL_GetError());
	}

	if (SDL_SetRenderTarget(renderer, nullptr)) {
		Utils::outMsg("Renderer::scale, SDL_SetRenderTarget(nullptr)", SDL_GetError());
	}

	scaled = true;
}



static SurfacePtr screenshot;
static bool screenshoted = true;

void Renderer::needMakeScreenshot() {
	screenshoted = false;
	needToRender = needToRedraw = true;
}
SurfacePtr Renderer::getScreenshot() {
	while (!screenshoted) {
		Utils::sleep(1);
	}
	return screenshot;
}

static void readPixels() {
	int w, h;
	SDL_GetRendererOutputSize(renderer, &w, &h);

	screenshot = ImageManipulator::getNewNotClear(w, h);
	if (SDL_RenderReadPixels(renderer, &screenshot->clip_rect,
							 SDL_PIXELFORMAT_RGBA32, screenshot->pixels, screenshot->pitch))
	{
		Utils::outMsg("SDL_RenderReadPixels", SDL_GetError());
	}
}


static void loop() {
	if (fastOpenGL) {
		checkOpenGlErrors = Config::get("check_fast_opengl_errors") == "True";
	}

	static std::vector<RenderStruct> toRender;
	static std::vector<RenderStruct> prevToRender;

	static std::vector<TexturePtr> textures;
	static std::vector<TexturePtr> prevTextures;


	while (!GV::exit) {
		if (!scaled) {
			scale();
			continue;
		}

		if (!Renderer::needToRender || GV::minimized) {
			Utils::sleep(1, false);
			continue;
		}


		{
			std::lock_guard g(Renderer::toRenderMutex);
			toRender.clear();
			Renderer::toRender.swap(toRender);
		}
		Renderer::needToRender = false;

		if (!Renderer::needToRedraw && toRender == prevToRender) continue;
		Renderer::needToRedraw = false;


		std::lock_guard g(Renderer::renderMutex);

		{
			std::map<SurfacePtr, TexturePtr> cache;
			textures.clear();
			textures.reserve(toRender.size());

			for (const RenderStruct &rs : toRender) {
				auto it = cache.find(rs.surface);
				if (it != cache.end()) {
					textures.push_back(it->second);
				}else {
					const TexturePtr texture = ImageCaches::getTexture(renderer, rs.surface);
					textures.push_back(texture);
					cache[rs.surface] = texture;
				}
			}
		}

		if (fastOpenGL) {
			checkErrors("loop", "Start");

			glViewport(0, 0, GV::width, GV::height);
			checkErrors("loop", "glViewport");
			glMatrixMode(GL_PROJECTION);
			checkErrors("loop", "glMatrixMode");
			glLoadIdentity();
			glOrtho(0, GV::width, GV::height, 0, 0.0, 1.0);
			checkErrors("loop", "glOrtho");
			glMatrixMode(GL_MODELVIEW);
			checkErrors("loop", "glMatrixMode");
		}
		if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)) {
			Utils::outMsg("SDL_SetRenderDrawColor", SDL_GetError());
		}
		if (SDL_RenderClear(renderer)) {
			Utils::outMsg("SDL_RenderClear", SDL_GetError());
		}
		if (fastOpenGL) {
			glEnable(GL_TEXTURE_2D);
			checkErrors("loop", "glEnable(GL_TEXTURE_2D)");

			glEnable(GL_BLEND);
			checkErrors("loop", "glEnable(GL_BLEND)");
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			checkErrors("loop", "glBlendFunc");
		}

		for (size_t i = 0; i < textures.size(); ++i) {
			if (!GV::inGame) break;

			const RenderStruct &rs = toRender[i];
			const TexturePtr &texture = textures[i];

			SDL_RenderSetClipRect(renderer, rs.clip ? &rs.clipRect : nullptr);

			if (fastOpenGL) {
				renderWithOpenGL(texture.get(),
				                 rs.angle, rs.alpha,
				                 &rs.srcRect,
				                 &rs.dstRect,
				                 &rs.center);
			}else {
				if (SDL_SetTextureAlphaMod(texture.get(), rs.alpha)) {
					Utils::outMsg("SDL_SetTextureAlphaMod", SDL_GetError());
				}

				if (SDL_RenderCopyEx(renderer, texture.get(),
				                     &rs.srcRect,
				                     &rs.dstRect,
				                     rs.angle,
				                     &rs.center,
				                     SDL_FLIP_NONE))
				{
					Utils::outMsg("SDL_RenderCopyEx", SDL_GetError());
				}
			}
		}

		if (GV::screens) {
			int w = GV::screens->getX();
			int h = GV::screens->getY();
			SDL_Rect empty = {0, 0, GV::width, GV::height};
			if (!h) {
				empty.w = w;
			}else {
				empty.h = h;
			}

			if (empty.w && empty.h) {
				if (fastOpenGL) {
					glColor4f(0, 0, 0, 1);
					checkErrors("loop", "glColor4f");
					glRecti(0, 0, empty.w, empty.h);
					checkErrors("loop", "glRecti");
				}else {
					if (SDL_RenderFillRect(renderer, &empty)) {
						Utils::outMsg("SDL_RenderFillRect", SDL_GetError());
					}
				}
			}

			if (!screenshoted) {
				readPixels();
				Renderer::needToRedraw = true;
				screenshoted = true;
			}else {
				SDL_RenderPresent(renderer);
			}
		}

		prevToRender.swap(toRender);
		prevTextures.swap(textures);
	}

	SDL_DestroyRenderer(renderer);
}

static void initImpl(bool *inited, bool *error) {
	int renderDriver = -1;

	Uint32 flags;
	if (Config::get("software_renderer") == "True") {
		flags = SDL_RENDERER_SOFTWARE;
	}else {
		flags = SDL_RENDERER_ACCELERATED;
		if (Config::get("opengl_vsync") == "True") {
			flags |= SDL_RENDERER_PRESENTVSYNC;
		}

		std::string opengl = "opengl";
		int countRenderDrivers = SDL_GetNumRenderDrivers();
		for (int i = 0; i < countRenderDrivers; ++i) {
			SDL_RendererInfo info;
			SDL_GetRenderDriverInfo(i, &info);
			if (info.name == opengl) {
				renderDriver = i;
				break;
			}
		}
		if (renderDriver == -1) {
			Utils::outMsg("Renderer::init", "OpenGL driver not found");
		}else {
			fastOpenGL = Config::get("fast_opengl") == "True";
			if (fastOpenGL) {
				SDL_SetHint(SDL_HINT_RENDER_BATCHING, "false");
			}
		}
	}

	renderer = SDL_CreateRenderer(GV::mainWindow, -1, flags);
	if (!renderer) {
		Utils::outMsg("SDL_CreateRenderer", SDL_GetError());

		if (flags & SDL_RENDERER_ACCELERATED) {
			renderer = SDL_CreateRenderer(GV::mainWindow, -1, SDL_RENDERER_SOFTWARE);
			if (!renderer) {
				Utils::outMsg("SDL_CreateRenderer", SDL_GetError());
			}
		}

		if (!renderer) {
			*error = true;
			*inited = true;
			return;
		}
	}

	SDL_GetRendererInfo(renderer, &Renderer::info);
	if (std::string(Renderer::info.name) == "opengl") {
		size_t countErrors = 0;
		const size_t maxCountErrors = 10;

		while (++countErrors < maxCountErrors && glGetError() != GL_NO_ERROR) {}
		if (countErrors == maxCountErrors) {
			Utils::outMsg("Renderer::init", "Using OpenGL failed");
			fastOpenGL = false;
		}
	}

	*inited = true;
	loop();
}
bool Renderer::init() {
	bool inited = false;
	bool error = false;

	std::thread(initImpl, &inited, &error).detach();
	while (!inited) {
		Utils::sleep(1, false);
	}

	return error;
}
