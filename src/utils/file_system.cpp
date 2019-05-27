#include "file_system.h"

#include <algorithm>
#include <filesystem>


static std::string clear(std::string path) {
	if (path.empty() || (path.back() != '/' && path.back() != '\\')) {
		return  path;
	}
	return path.substr(0, path.size() - 1);
}


std::string FileSystem::getCurrentPath() {
	return std::filesystem::current_path().string();
}
std::string FileSystem::setCurrentPath(std::string path) {
	path = clear(path);

	std::error_code ec;
	std::filesystem::current_path(path, ec);

	if (!ec.value()) return "";
	return "Set current path to <" + path + "> failed\n" + ec.message();
}

bool FileSystem::exists(std::string path) {
	return std::filesystem::exists(clear(path));
}
bool FileSystem::isDirectory(std::string path) {
	return std::filesystem::is_directory(clear(path));
}
uintmax_t FileSystem::getFileSize(std::string path) {
	return std::filesystem::file_size(path);
}

std::vector<std::string> FileSystem::getDirectories(std::string path) {
	std::vector<std::string> res;

	for (std::filesystem::directory_iterator it(clear(path)), end; it != end; ++it) {
		const std::string pathStr = clear(it->path().string());
		if (FileSystem::isDirectory(pathStr)) {
			res.push_back(pathStr);
		}
	}
	return res;
}

void FileSystem::createDirectory(std::string path) {
	std::filesystem::create_directory(path);
}

std::vector<std::string> FileSystem::getFilesRecursive(std::string path) {
	std::vector<std::string> res;

	for (std::filesystem::recursive_directory_iterator it(clear(path)), end; it != end; ++it) {
		const std::string pathStr = clear(it->path().string());
		if (std::filesystem::is_regular_file(pathStr)) {
			if (pathStr.find("_SL_FILE_") == size_t(-1)) {
				res.push_back(pathStr);
			}
		}
	}

	std::sort(res.begin(), res.end());
	return  res;
}
