#ifndef IMAGE_H
#define IMAGE_H

#include <memory>

#include "utils/utils.h"

class Image {
public:
	static void loadImage(const std::string &desc);
	static SurfacePtr getImage(String desc);
};

#endif // IMAGE_H
