#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <map>
#include <deque>
#include <functional>

#include "utils/utils.h"

class Image {
private:
	static std::map<String, std::function<SurfacePtr(const std::vector<String>&)>> functions;

	static std::deque<String> toLoadImages;
	static std::mutex toLoadMutex;
	static void preloadThread();

	static size_t countThreads;
	static const size_t partsOnThreads = 4;
	static std::deque<std::pair<size_t, std::function<void(size_t)>>> partsToProcessing;
	static std::mutex processingMutex;
	static void processingThread();

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
	static void save(const std::string &imageStr, const std::string &path, const std::string &width, const std::string &height);

	static void loadImage(const std::string &desc);
	static SurfacePtr getImage(String desc);
};

#endif // IMAGE_H
