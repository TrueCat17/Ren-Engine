#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <map>
#include <functional>

#include "utils/utils.h"

class Image {
private:
	static std::map<String, std::function<SurfacePtr(const std::vector<String>&)>> functions;

	static SurfacePtr scale(const std::vector<String> &args);
	static SurfacePtr factorScale(const std::vector<String> &args);
	static SurfacePtr crop(const std::vector<String> &args);
	static SurfacePtr composite(const std::vector<String> &args);
	static SurfacePtr flip(const std::vector<String> &args);
	static SurfacePtr matrixColor(const std::vector<String> &args);
	static SurfacePtr reColor(const std::vector<String> &args);
	static SurfacePtr rotozoom(const std::vector<String> &args);
	static SurfacePtr mask(const std::vector<String> &args);

public:
	static void init();

	static void loadImage(const std::string &desc);
	static SurfacePtr getImage(String desc);
};

#endif // IMAGE_H
