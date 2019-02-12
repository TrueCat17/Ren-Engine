#ifndef IMAGETYPEDEFS_H
#define IMAGETYPEDEFS_H

#include <SDL2/SDL_surface.h>
typedef struct SDL_Texture SDL_Texture;

//from <memory>:
#include <typeinfo>
#include <ext/concurrence.h>
#include <bits/unique_ptr.h>
#include <bits/shared_ptr.h>

typedef std::shared_ptr<SDL_Surface> SurfacePtr;
typedef std::shared_ptr<SDL_Texture> TexturePtr;

#endif // IMAGETYPEDEFS_H
