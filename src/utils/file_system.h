#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <vector>
#include <string>

class FileSystem {
public:
	static std::string getCurrentPath();
	static std::string setCurrentPath(std::string path);

	static bool exists(const std::string &path);
	static bool isDirectory(const std::string &path);
	static size_t getFileSize(const std::string &path);

	static void createDirectory(const std::string &path);
	static void remove(const std::string &path);
	static void rename(const std::string &oldPath, const std::string &newPath);

	static std::string getParentDirectory(const std::string &path);
	static std::string getFileName(const std::string &path);

	static std::vector<std::string> getDirectories(const std::string &path);

	static std::vector<std::string> getFiles(const std::string &path);
	static std::vector<std::string> getFilesRecursive(const std::string &path);
	
	static int64_t getFileTime(const std::string &path);

	static bool startFile_win32(std::string path);
};

#endif // FILE_SYSTEM_H
