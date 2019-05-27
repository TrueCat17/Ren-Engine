#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <vector>
#include <string>
#include <stdint.h>

class FileSystem {
public:
	static std::string getCurrentPath();
	static std::string setCurrentPath(std::string path);

	static bool exists(std::string path);
	static bool isDirectory(std::string path);
	static uintmax_t getFileSize(std::string path);

	static std::vector<std::string> getDirectories(std::string path);
	static void createDirectory(std::string path);

	static std::vector<std::string> getFilesRecursive(std::string path);
};

#endif // FILE_SYSTEM_H
