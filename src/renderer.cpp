#include "renderer.h"

#include <thread>

#include <SDL2/SDL_opengl.h>

#include "config.h"
#include "gui/group.h"
#include "media/image_manipulator.h"

#include "utils/math.h"
#include "utils/image_caches.h"
#include "utils/utils.h"



int Renderer::maxTextureWidth = 0;
int Renderer::maxTextureHeight = 0;

bool Renderer::needToRender = false;
bool Renderer::needToRedraw = false;

std::mutex Renderer::renderMutex;

std::mutex Renderer::toRenderMutex;
std::vector<RenderStruct> Renderer::toRender;


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

	struct Texture {
		const void *magic;
		Uint32 format;              /**< The pixel format of the texture */
		int access;                 /**< SDL_TextureAccess */
		int w;                      /**< The width of the texture */
		int h;                      /**< The height of the texture */
		int modMode;                /**< The texture modulation mode */
		SDL_BlendMode blendMode;    /**< The texture blend mode */
		Uint8 r, g, b, a;           /**< Texture modulation values */

		SDL_Renderer *renderer;

		/* Support for formats not supported directly by the renderer */
		SDL_Texture *native;
		SDL_SW_YUVTexture *yuv;
		void *pixels;
		int pitch;
		SDL_Rect locked_rect;

		void *driverdata;           /**< Driver specific texture representation */

		SDL_Texture *prev;
		SDL_Texture *next;
	};

	struct TextureData {
		GLuint texture;
		GLenum type;
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
			Utils::outMsg("Renderer::" + String(from) + ", " + funcName, "Using OpenGL failed");
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

		Utils::outMsg("Renderer::" + String(from) + ", " + funcName, str);
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
	queryTexture(tmpTexture, textureWidth, textureHeight);

	if (scaleWidth > textureWidth || scaleHeight > textureHeight) {
		SDL_DestroyTexture(tmpTexture);

		textureWidth  = int(std::ceil(scaleWidth / 256.0)) * 256;
		textureHeight = int(std::ceil(scaleHeight / 256.0)) * 256;

		tmpTexture = SDL_CreateTexture(GV::mainRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET,
									   textureWidth, textureHeight);
		if (!tmpTexture) {
			Utils::outMsg("SDL_CreateTexture", SDL_GetError());
		}
	}

	SDL_Rect dstRect = {0, 0, scaleWidth, scaleHeight};
	TexturePtr toRender = ImageCaches::getTexture(toScaleSurface);

	SDL_SetRenderTarget(GV::mainRenderer, tmpTexture);
	SDL_SetRenderDrawColor(GV::mainRenderer, 0, 0, 0, 0);
	SDL_RenderFillRect(GV::mainRenderer, &dstRect);
	SDL_RenderCopy(GV::mainRenderer, toRender.get(), nullptr, &dstRect);

	scaledSurface = ImageManipulator::getNewNotClear(scaleWidth, scaleHeight);
	SDL_RenderReadPixels(GV::mainRenderer, &dstRect, SDL_PIXELFORMAT_RGBA32, scaledSurface->pixels, scaledSurface->pitch);

	SDL_SetRenderTarget(GV::mainRenderer, nullptr);

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
	SDL_GetRendererOutputSize(GV::mainRenderer, &w, &h);

	screenshot = ImageManipulator::getNewNotClear(w, h);
	if (SDL_RenderReadPixels(GV::mainRenderer, &screenshot->clip_rect,
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


	auto changedToRender = [&]() -> bool {
		if (prevToRender.size() != toRender.size()) return true;
		for (size_t i = 0; i < toRender.size(); ++i) {
			if (prevToRender[i] != toRender[i]) return true;
		}
		return false;
	};

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

		if (!Renderer::needToRedraw && !changedToRender()) {
			continue;
		}
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
					const TexturePtr texture = ImageCaches::getTexture(rs.surface);
					textures.push_back(texture);
					cache[rs.surface] = texture;
				}
			}
		}

		if (fastOpenGL) {
			checkErrors("loop", "Start");
		}

		if (SDL_SetRenderDrawColor(GV::mainRenderer, 0, 0, 0, 255)) {
			Utils::outMsg("SDL_SetRenderDrawColor", SDL_GetError());
		}
		if (SDL_RenderClear(GV::mainRenderer)) {
			Utils::outMsg("SDL_RenderClear", SDL_GetError());
		}
		if (SDL_SetRenderDrawColor(GV::mainRenderer, 0, 0, 0, 255)) {
			Utils::outMsg("SDL_SetRenderDrawColor", SDL_GetError());
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
			if (!GV::inGame || GV::exit) {
				break;
			}

			const RenderStruct &rs = toRender[i];
			const TexturePtr &texture = textures[i];

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

				if (SDL_RenderCopyEx(GV::mainRenderer, texture.get(),
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
		if (fastOpenGL) {
			glDisable(GL_TEXTURE_2D);
			checkErrors("loop", "glDisables(GL_TEXTURE_2D)");
			glDisable(GL_BLEND);
			checkErrors("loop", "glDisable(GL_BLEND)");
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
					if (SDL_RenderFillRect(GV::mainRenderer, &empty)) {
						Utils::outMsg("SDL_RenderFillRect", SDL_GetError());
					}
				}
			}

			if (!screenshoted) {
				readPixels();
				Renderer::needToRedraw = true;
				screenshoted = true;
			}else {
				SDL_RenderPresent(GV::mainRenderer);
			}
		}

		prevToRender.swap(toRender);
		prevTextures.swap(textures);
	}

	SDL_DestroyRenderer(GV::mainRenderer);
}


bool Renderer::init() {
	bool inited = false;
	bool error = false;

	auto initFunc = [&]() {
		int renderDriver = -1;

		Uint32 flags;
		if (Config::get("software_renderer") == "True") {
			flags = SDL_RENDERER_SOFTWARE;
		}else {
			flags = SDL_RENDERER_ACCELERATED;
			if (Config::get("opengl_vsync") == "True") {
				flags |= SDL_RENDERER_PRESENTVSYNC;
			}

			int countRenderDrivers = SDL_GetNumRenderDrivers();
			for (int i = 0; i < countRenderDrivers; ++i) {
				SDL_RendererInfo info;
				SDL_GetRenderDriverInfo(i, &info);
				if (String(info.name) == "opengl") {
					renderDriver = i;
					break;
				}
			}
			if (renderDriver == -1) {
				Utils::outMsg("Renderer::init", "OpenGL driver not found");
			}
		}

		GV::mainRenderer = SDL_CreateRenderer(GV::mainWindow, renderDriver, flags);
		if (!GV::mainRenderer) {
			Utils::outMsg("SDL_CreateRenderer", SDL_GetError());

			if (flags & SDL_RENDERER_ACCELERATED) {
				GV::mainRenderer = SDL_CreateRenderer(GV::mainWindow, -1, SDL_RENDERER_SOFTWARE);
			}
		}
		if (!GV::mainRenderer) {
			Utils::outMsg("SDL_CreateRenderer", SDL_GetError());
			error = true;
			inited = true;
			return;
		}

		SDL_RendererInfo info;
		SDL_GetRendererInfo(GV::mainRenderer, &info);
		if (String(info.name) == "opengl") {
			size_t countErrors = 0;
			const size_t maxCountErrors = 10;

			while (++countErrors < maxCountErrors && glGetError() != GL_NO_ERROR) {}
			if (countErrors == maxCountErrors) {
				Utils::outMsg("Renderer::init", "Using OpenGL failed");
			}else {
				fastOpenGL = Config::get("fast_opengl") == "True";
			}
		}
		maxTextureWidth = info.max_texture_width;
		maxTextureHeight = info.max_texture_height;

		inited = true;
		loop();
	};


	std::thread(initFunc).detach();
	while (!inited) {
		Utils::sleep(1, false);
	}

	return error;
}
