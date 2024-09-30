#include "file_system.h"

#include <algorithm>

#include <filesystem>
namespace fs = std::filesystem;

#include <SDL2/SDL.h>

#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/utils.h"


static std::string clear(const std::string &path) {
	if (path.empty() || (path.back() != '/' && path.back() != '\\')) {
		return path;
	}
	return path.substr(0, path.size() - 1);
}


#ifdef __CYGWIN__
#include <Python.h>

#include <windef.h>
#include <winbase.h>
#include <winuser.h>
#include <shellapi.h>
#endif

std::string FileSystem::getCurrentPath() {
	std::string res;
#ifndef __CYGWIN__
	res = fs::current_path().string();
#else
	int len = GetCurrentDirectoryW(0, nullptr);
	std::wstring cwd;
	cwd.resize(len);
	GetCurrentDirectoryW(len, cwd.data());
	
	char *cwd_utf8 = SDL_iconv_wchar_utf8(cwd.data());
	res = cwd_utf8;
	if (cwd_utf8) {
		SDL_free(cwd_utf8);
	}else {
		Utils::outMsg("FileSystem::getCurrentPath, SDL_iconv_wchar_utf8", SDL_GetError());
	}
#endif
	String::replaceAll(res, "\\", "/");
	return res;
}
std::string FileSystem::setCurrentPath(std::string path) {
	path = clear(path);

	std::error_code ec;
	fs::current_path(path.c_str(), ec);

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

	if (res.empty()) {
		res = "./";
	}else
	if (res.back() == '\\') {
		res.back() = '/';
	}else
	if (res.back() != '/') {
		res.push_back('/');
	}

	return res;
}
std::string FileSystem::getFileName(const std::string &path) {
	return fs::path(clear(path)).lexically_normal().filename().string();
}



std::vector<std::string> FileSystem::getDirectories(const std::string &path) {
	std::vector<std::string> res;
	if (!exists(path)) return res;

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
	if (!exists(path)) return res;

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
	if (!exists(path)) return res;

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


int64_t FileSystem::getFileTime(const std::string &path) {
	std::error_code ec;
	auto time = fs::last_write_time(path, ec);
	if (ec.value()) {
		Utils::outMsg("FileSystem::getFileTime", ec.message() + "\n  path: <" + path + ">");
		return 0;
	}

	auto dur = time.time_since_epoch();
	auto durSec = std::chrono::duration_cast<std::chrono::seconds>(dur);
	return durSec.count();
}

bool FileSystem::startFile_win32([[maybe_unused]] std::string path, [[maybe_unused]] PyObject *vars) {
#ifndef __CYGWIN__
	return false;
#else
	wchar_t *wpath = reinterpret_cast<wchar_t*>(SDL_iconv_string("UTF-16LE", "UTF-8", path.c_str(), path.size() + 1));
	if (!wpath) {
		Utils::outMsg("FileSystem::startFile_win32, SDL_iconv_string", SDL_GetError());
		return false;
	}

	wchar_t *wenv = nullptr;

	ScopeExit se([&]() {
		SDL_free(wpath);
		SDL_free(wenv);
	});

	//ShellExecuteW uses default environment - without setting and inheritting (because python and cygwin problems)
	//  => use CreateProcessW for *.exe
	if (!String::endsWith(path, ".exe")) {
		HINSTANCE code = ShellExecuteW(nullptr, nullptr, wpath, nullptr, nullptr, SW_SHOWNORMAL);
		return reinterpret_cast<DWORD>(code) > 32;
	}

	std::vector<char> v;
	v.reserve(1024);

	if (!PyList_CheckExact(vars)) {
		std::string type = vars->ob_type->tp_name;
		Utils::outMsg("FileSystem::startFile_win32", "Expected type(vars) is list, got " + type);
		return false;
	}

	long size = Py_SIZE(vars);
	for (long i = 0; i < size; ++i) {
		PyObject *pyVar = PyList_GET_ITEM(vars, i);
		if (!PyUnicode_CheckExact(pyVar)) {
			std::string type = pyVar->ob_type->tp_name;
			Utils::outMsg("FileSystem::startFile_win32", "Expected type(vars[i]) is str, got " + type);
			return false;
		}

		const char *var = PyUnicode_AsUTF8(pyVar);
		long varSize = Py_SIZE(pyVar);
		v.insert(v.end(), var, var + varSize);
		v.push_back(0);
	}
	v.push_back(0);

	wenv = reinterpret_cast<wchar_t*>(SDL_iconv_string("UTF-16LE", "UTF-8", v.data(), v.size()));
	if (!wenv) {
		Utils::outMsg("FileSystem::startFile_win32, SDL_iconv_string", SDL_GetError());
		return false;
	}

	DWORD flags = CREATE_UNICODE_ENVIRONMENT;
	STARTUPINFOW info = {};
	info.cb = sizeof(STARTUPINFOW);
	PROCESS_INFORMATION procInfo;

	BOOL code = CreateProcessW(wpath, nullptr, nullptr, nullptr, FALSE, flags, wenv, nullptr, &info, &procInfo);
	return code != FALSE;
#endif
}
