#ifndef IMAGETYPEDEFS_H
#define IMAGETYPEDEFS_H

#include "SDL2/SDL_render.h"

#include "gv.h"


template <typename T, void (*deleter)(T*)>
class SmartPtr {
private:
	T *ptr = nullptr;
	mutable int *counter = nullptr;

	void clear() {
		if (!counter || GV::exit) return;

		if (--*counter == 0) {
			delete counter;
			counter = nullptr;
			deleter(ptr);
			ptr = nullptr;
		}
	}

public:
	void operator=(const SmartPtr &pointer) {
		std::lock_guard g(GV::mutexForSmartPtr);
		clear();

		ptr = pointer.ptr;
		counter = pointer.counter;
		if (counter) {
			++*counter;
		}
	}
	void operator=(T *pointer) {
		std::lock_guard g(GV::mutexForSmartPtr);
		clear();

		ptr = pointer;
		if (ptr) {
			counter = new int(1);
		}else {
			counter = nullptr;
		}
	}


	SmartPtr() {}
	SmartPtr(const SmartPtr &pointer) {
		*this = pointer;
	}
	SmartPtr(T *pointer) {
		*this = pointer;
	}

	~SmartPtr() {
		std::lock_guard g(GV::mutexForSmartPtr);
		clear();
	}


	int use_count() const {
		return *counter;
	}

	T* get() const {
		return ptr;
	}
	T* operator->() const {
		return ptr;
	}

	operator bool() const {
		return ptr != nullptr;
	}
	bool operator==(const SmartPtr &pointer) const {
		return ptr == pointer.ptr && counter == pointer.counter;
	}
	bool operator!=(const SmartPtr &pointer) const {
		return !(*this == pointer);
	}
	bool operator<(const SmartPtr &pointer) const {
		return size_t(ptr) < size_t(pointer.ptr);
	}
};

using SurfacePtr = SmartPtr<SDL_Surface, SDL_FreeSurface>;
using TexturePtr = SmartPtr<SDL_Texture, SDL_DestroyTexture>;

#endif // IMAGETYPEDEFS_H
