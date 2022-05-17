#include "file_system.h"

#include <algorithm>

#include <filesystem>
namespace fs = std::filesystem;

#include <SDL2/SDL.h>

#include "utils/string.h"
#include "utils/utils.h"


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



bool FileSystem::exists(const std::string &path) {
	return fs::exists(clear(path));
}
bool FileSystem::isDirectory(const std::string &path) {
	return fs::is_directory(clear(path));
}
size_t FileSystem::getFileSize(const std::string &path) {
	return fs::file_size(path);
}



void FileSystem::createDirectory(const std::string &path) {
	std::error_code ec;
	fs::create_directory(clear(path), ec);
	if (ec.value()) {
		Utils::outMsg("FileSystem::createDirectory", ec.message() + "\n  path: <" + path + ">");
	}
}

void FileSystem::remove(const std::string &path) {
	std::error_code ec;
	fs::remove_all(clear(path), ec);
	if (ec.value()) {
		Utils::outMsg("FileSystem::remove", ec.message() + "\n  path: <" + path + ">");
	}
}

void FileSystem::rename(const std::string &oldPath, const std::string &newPath) {
	std::error_code ec;
	fs::rename(clear(oldPath), clear(newPath), ec);
	if (ec.value()) {
		Utils::outMsg("FileSystem::rename", ec.message() + "\n  oldPath: <" + oldPath + ">\n  newPath: <" + newPath + ">");
	}
}



std::string FileSystem::getParentDirectory(const std::string &path) {
	std::string res = fs::path(clear(path)).lexically_normal().parent_path().string();

	if (String::endsWith(res, "\\")) {
		res.pop_back();
	}
	if (!res.empty() && !String::endsWith(res, "/")) {
		res.push_back('/');
	}

	return res;
}
std::string FileSystem::getFileName(const std::string &path) {
	return fs::path(clear(path)).lexically_normal().filename().string();
}



std::vector<std::string> FileSystem::getDirectories(const std::string &path) {
	std::vector<std::string> res;

	for (fs::directory_iterator it(clear(path)), end; it != end; ++it) {
		std::string pathStr = clear(it->path().string());
		if (FileSystem::isDirectory(pathStr)) {
			String::replaceAll(pathStr, "\\", "/");
			res.push_back(pathStr);
		}
	}
	return res;
}



std::vector<std::string> FileSystem::getFiles(const std::string &path) {
	std::vector<std::string> res;

	for (fs::directory_iterator it(clear(path)), end; it != end; ++it) {
		std::string pathStr = clear(it->path().string());
		if (fs::is_regular_file(pathStr)) {
			String::replaceAll(pathStr, "\\", "/");
			res.push_back(pathStr);
		}
	}

	std::sort(res.begin(), res.end());
	return res;
}

std::vector<std::string> FileSystem::getFilesRecursive(const std::string &path) {
	std::vector<std::string> res;

	for (fs::recursive_directory_iterator it(clear(path)), end; it != end; ++it) {
		std::string pathStr = clear(it->path().string());
		if (fs::is_regular_file(pathStr)) {
			String::replaceAll(pathStr, "\\", "/");
			res.push_back(pathStr);
		}
	}

	std::sort(res.begin(), res.end());
	return res;
}
