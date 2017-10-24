#include "renderer.h"

#include <thread>

#include "utils/utils.h"


bool Renderer::needToRender = false;
bool Renderer::needToRedraw = false;

std::mutex Renderer::renderMutex;

std::mutex Renderer::toRenderMutex;
std::vector<RenderStruct> Renderer::toRender;


GLuint Renderer::getTextureId(SDL_Texture *texture) {
	if (!texture) return 0;

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

	return data->texture;
}

void Renderer::renderWithOpenGL(SDL_Texture *texture, Uint8 alpha,
								int angle, const SDL_Point *center,
								const SDL_Rect *src, const SDL_Rect *dst)
{
	if (!alpha || !dst->w || !dst->h) return;

	int textureWidth;
	int textureHeight;
	SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);

	SDL_Rect realSrc;
	if (src) {
		if (!src->w || !src->h) return;
		realSrc = {src->x, src->y, src->w, src->h};
	}else {
		realSrc = {0, 0, textureWidth, textureHeight};
	}

	GLuint textureId = getTextureId(texture);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(1, 1, 1, float(alpha) / 255);

	if (angle) {
		int dX = dst->x + (center ? center->x : 0);
		int dY = dst->y + (center ? center->y : 0);

		glPushMatrix();
		glTranslated(dX, dY, 0);
		glRotated(angle, 0, 0, 1);
		glTranslated(-dX, -dY, 0);
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

	if (angle) {
		glPopMatrix();
	}
}

void Renderer::renderThreadFunc() {
	std::vector<RenderStruct> toRender;
	std::vector<RenderStruct> prevToRender;

	auto changedToRender = [&]() -> bool {
		if (prevToRender.size() != toRender.size()) return true;
		for (size_t i = 0; i < toRender.size(); ++i) {
			if (prevToRender[i] != toRender[i]) return true;
		}
		return false;
	};

	while (!GV::exit) {
		if (!needToRender) {
			Utils::sleep(1, false);
		}else {
			{
				std::lock_guard<std::mutex> trg(Renderer::toRenderMutex);
				toRender.clear();
				Renderer::toRender.swap(toRender);
			}
			needToRender = false;

			if (!needToRedraw && !changedToRender()) {
				continue;
			}

			/* Рендер идёт в отдельном потоке, а обновление объектов - в основном
			 * Но при обновлении иногда используется рендер, поэтому
			 * чтобы не ждать окончания рендера, он разбит на части,
			 * и нужно ждать лишь завершения текущей части
			 */

			const size_t COUNT_IN_PART = 50;
			size_t len = toRender.size() / COUNT_IN_PART + 1;

			//Part 0
			{
				std::lock_guard<std::mutex> g(Renderer::renderMutex);

				SDL_SetRenderDrawColor(GV::mainRenderer, 0, 0, 0, 255);
				SDL_RenderClear(GV::mainRenderer);
			}

			//Parts 1..L
			for (size_t i = 0; i < len; ++i) {
				if (!GV::inGame || GV::exit) {
					break;
				}

				Utils::sleepMicroSeconds(10);
				std::lock_guard<std::mutex> g(Renderer::renderMutex);
				if (!GV::inGame || GV::exit) {
					break;
				}

				for (size_t j = i * COUNT_IN_PART;
					 j < (i + 1) * COUNT_IN_PART && j < toRender.size();
					 ++j)
				{
					const RenderStruct &rs = toRender[j];

					if (GV::isOpenGL) {
						renderWithOpenGL(rs.texture.get(), rs.alpha,
										 rs.angle, rs.centerIsNull ? nullptr : &rs.center,
										 rs.srcRectIsNull ? nullptr : &rs.srcRect,
										 rs.dstRectIsNull ? nullptr : &rs.dstRect);
					}else {
						if (SDL_SetTextureAlphaMod(rs.texture.get(), rs.alpha)) {
							Utils::outMsg("SDL_SetTextureAlphaMod", SDL_GetError());
						}

						if (SDL_RenderCopyEx(GV::mainRenderer, rs.texture.get(),
											 rs.srcRectIsNull ? nullptr : &rs.srcRect,
											 rs.dstRectIsNull ? nullptr : &rs.dstRect,
											 rs.angle,
											 rs.centerIsNull ? nullptr : &rs.center,
											 SDL_FLIP_NONE))
						{
							Utils::outMsg("SDL_RenderCopyEx", SDL_GetError());
						}
					}
				}
			}

			//Part L+1
			{
				std::lock_guard<std::mutex> g(Renderer::renderMutex);
				SDL_RenderPresent(GV::mainRenderer);
			}

			needToRedraw = false;
			prevToRender.swap(toRender);
		}
	}
}
