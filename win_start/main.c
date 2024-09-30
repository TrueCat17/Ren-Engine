#include <libloaderapi.h> //GetModuleFileNameA
#include <dirent.h>
#include <errno.h>
#include <locale.h>
#include <process.h> //execl
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
	if (argc > 1) {
		printf("Expected 0 args\n");
		return 1;
	}
	
	const size_t BUFFER_SIZE = 300;
	
	const char *RE_LANG = getenv("RE_LANG");
	if (!RE_LANG || RE_LANG[0] == 0) {
		setlocale(LC_ALL, "");
		const char *locale = setlocale(LC_ALL, NULL);
		
		char envVar[BUFFER_SIZE];
		snprintf(envVar, BUFFER_SIZE, "RE_LANG=%s", locale);
		putenv(envVar);
	}
	
	char path[BUFFER_SIZE];
	size_t size = GetModuleFileNameA(NULL, path, BUFFER_SIZE);
	size_t lastSep = 0;
	for (size_t i = 0; i < size; ++i) {
		if (path[i] == '\\') {
			path[i] = '/';
		}
		if (path[i] == '/') {
			lastSep = i;
		}
	}
	path[lastSep] = 0;
	
	char dirPath[BUFFER_SIZE];
	snprintf(dirPath, BUFFER_SIZE, "%s/Ren-Engine", path);
	
	DIR *dir = opendir(dirPath);
	if (!dir) {
		printf("Error on open directory <%s>: %s\n", dirPath, strerror(errno));
		return 1;
	}
	
	const char *fileName = NULL;
	struct dirent *ent;
	while (ent = readdir(dir)) {
		const char *name = ent->d_name;
		size_t len = strlen(name);
		if (len < 5 || len > 260) continue;
		
		if (name[len - 1] != 'e') continue;
		if (name[len - 2] != 'x') continue;
		if (name[len - 3] != 'e') continue;
		if (name[len - 4] != '.') continue;
		
		fileName = name;
		break;
	}
	closedir(dir);
	
	if (!fileName) {
		printf("*.exe not found in <%s>\n", dirPath);
		return 1;
	}
	
	char exePath[BUFFER_SIZE];
	snprintf(exePath, BUFFER_SIZE, "%s/%s", dirPath, fileName);
	
	if (execl(exePath, "Ren-Engine", NULL)) {
		printf("Error on execl <%s>: %s\n", exePath, strerror(errno));
		return 1;
	}
	return 0;
}
