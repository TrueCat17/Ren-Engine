#ifndef IMAGETYPEDEFS_H
#define IMAGETYPEDEFS_H

#include "gv.h"


template <typename T>
class SmartPtr {
private:
	mutable int *counter = nullptr;

	T *ptr = nullptr;
	void(*deleter)(T*) = nullptr;

	void clear();

public:
	void reset();
	void reset(const SmartPtr<T> &pointer);
	void reset(T *pointer, void(*deleter)(T*) = nullptr);


	SmartPtr() {}
	SmartPtr(const SmartPtr<T> &pointer) {
		reset(pointer);
	}
	SmartPtr(T *pointer, void(*deleter)(T*) = nullptr) {
		reset(pointer, deleter);
	}

	~SmartPtr() {
		std::lock_guard g(GV::mutexForSmartPtr);
		clear();
	}


	T* get() const {
		return ptr;
	}
	int use_count() const {
		return *counter;
	}

	operator bool() const {
		return ptr != nullptr;
	}
	bool operator==(const SmartPtr<T> &pointer) const {
		return counter == pointer.counter;
	}
	bool operator!=(const SmartPtr<T> &pointer) const {
		return counter != pointer.counter;
	}
	bool operator<(const SmartPtr<T> &pointer) const {
		return counter < pointer.counter;
	}
	T* operator->() const {
		return get();
	}

	void operator=(const SmartPtr<T> &pointer) {
		reset(pointer);
	}
};

template<typename T>
void SmartPtr<T>::clear() {
	if (!counter || GV::exit) return;

	if (--*counter) return;

	delete counter;
	if (ptr) {
		deleter(ptr);
	}
}

template<typename T>
void SmartPtr<T>::reset() {
	std::lock_guard g(GV::mutexForSmartPtr);

	clear();

	ptr = nullptr;
	counter = nullptr;
	deleter = nullptr;
}

template<typename T>
void SmartPtr<T>::reset(const SmartPtr<T> &pointer) {
	std::lock_guard g(GV::mutexForSmartPtr);

	clear();

	ptr = pointer.ptr;
	counter = pointer.counter;
	deleter = pointer.deleter;

	++*counter;
}

template<typename T>
void SmartPtr<T>::reset(T *pointer, void(*deleter)(T*)) {
	if (pointer && !deleter) {
		throw "pointer != nullptr, deleter == nullptr";
	}

	std::lock_guard g(GV::mutexForSmartPtr);
	clear();

	ptr = pointer;
	counter = new int(1);
	this->deleter = deleter;
}


#include <SDL2/SDL_surface.h>
typedef struct SDL_Texture SDL_Texture;

typedef SmartPtr<SDL_Surface> SurfacePtr;
typedef SmartPtr<SDL_Texture> TexturePtr;

#endif // IMAGETYPEDEFS_H
