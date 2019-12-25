#include "file_system.h"

#include <algorithm>

#ifdef __cpp_lib_filesystem
	#include <filesystem>
	namespace fs = std::filesystem;
#elif 1
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#else
	#error "Header <filesystem> not found"
#endif

#include <SDL2/SDL.h>


static std::string clear(std::string path) {
	if (path.empty() || (path.back() != '/' && path.back() != '\\')) {
		return path;
	}
	return path.substr(0, path.size() - 1);
}


std::string FileSystem::getCurrentPath() {
	return fs::current_path().string();
}
std::string FileSystem::setCurrentPath(std::string path) {
	path = clear(path);

	std::error_code ec;
#ifdef __WIN32__
	auto buf = (wchar_t*)SDL_iconv_utf8_ucs2(path.c_str());
	fs::current_path(buf, ec);
	SDL_free(buf);
#else
	fs::current_path(path.c_str(), ec);
#endif

	if (!ec.value()) return "";
	return "Set current path to <" + path + "> failed\n" + ec.message();
}

bool FileSystem::exists(std::string path) {
	return fs::exists(clear(path));
}
bool FileSystem::isDirectory(std::string path) {
	return fs::is_directory(clear(path));
}
uintmax_t FileSystem::getFileSize(std::string path) {
	return fs::file_size(path);
}

std::vector<std::string> FileSystem::getDirectories(std::string path) {
	std::vector<std::string> res;

	for (fs::directory_iterator it(clear(path)), end; it != end; ++it) {
		const std::string pathStr = clear(it->path().string());
		if (FileSystem::isDirectory(pathStr)) {
			res.push_back(pathStr);
		}
	}
	return res;
}

void FileSystem::createDirectory(std::string path) {
	fs::create_directory(path);
}

std::vector<std::string> FileSystem::getFilesRecursive(std::string path) {
	std::vector<std::string> res;

	for (fs::recursive_directory_iterator it(clear(path)), end; it != end; ++it) {
		const std::string pathStr = clear(it->path().string());
		if (fs::is_regular_file(pathStr)) {
			if (pathStr.find("_SL_FILE_") == size_t(-1)) {
				res.push_back(pathStr);
			}
		}
	}

	std::sort(res.begin(), res.end());
	return  res;
}
