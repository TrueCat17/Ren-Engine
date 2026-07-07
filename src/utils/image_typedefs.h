#ifndef IMAGETYPEDEFS_H
#define IMAGETYPEDEFS_H

#include "SDL3/SDL_render.h"

#include "gv.h"


template <typename T, void (*deleter)(T*), std::mutex *mutex>
class SmartPtr {
private:
	//extra constant, because false warnings in old gcc
	static constexpr bool USE_MUTEX = (mutex != nullptr);

	static inline
	void lock() {
		if constexpr (USE_MUTEX) {
			mutex->lock();
		}
	}

	static inline
	void unlock() {
		if constexpr (USE_MUTEX) {
			mutex->unlock();
		}
	}


	T *ptr = nullptr;
	int *counter = nullptr;

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
		if (ptr == pointer.ptr) return;

		lock();

		clear();

		ptr = pointer.ptr;
		counter = pointer.counter;
		if (counter) {
			++*counter;
		}

		unlock();
	}
	void operator=(T *pointer) {
		if (ptr == pointer) return;

		lock();

		clear();

		ptr = pointer;
		if (ptr) {
			counter = new int(1);
		}else {
			counter = nullptr;
		}

		unlock();
	}


	SmartPtr() {}
	SmartPtr(const SmartPtr &pointer) {
		*this = pointer;
	}
	SmartPtr(T *pointer) {
		*this = pointer;
	}

	~SmartPtr() {
		lock();

		clear();

		unlock();
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
		return ptr == pointer.ptr;
	}
	bool operator!=(const SmartPtr &pointer) const {
		return !(*this == pointer);
	}
	bool operator<(const SmartPtr &pointer) const {
		return size_t(ptr) < size_t(pointer.ptr);
	}
};

using SurfacePtr = SmartPtr<SDL_Surface, SDL_DestroySurface, &GV::mutexForSurfacePtr>;
using TexturePtr = SmartPtr<SDL_Texture, SDL_DestroyTexture, nullptr>;

#endif // IMAGETYPEDEFS_H
