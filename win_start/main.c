#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <process.h> //execl
#include <errno.h>

int main(int argc, char **argv) {
	if (argc > 1) {
		printf("Expected 0 args\n");
		return 1;
	}
	
	const char *dirPath = "./Ren-Engine";
	
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
	
	char exePath[300];
	strcpy(exePath, dirPath);
	exePath[strlen(dirPath)] = '/';
	strcpy(exePath + strlen(dirPath) + 1, fileName);
	
	char exePathQ[300];
	exePathQ[0] = '"';
	strcpy(exePathQ + 1, exePath);
	size_t len = strlen(exePathQ);
	exePathQ[len] = '"';
	exePathQ[len + 1] = 0;
	
	if (execl(exePath, exePathQ, (char*)0)) {
		printf("Error on execl <%s>: %s\n", exePath, strerror(errno));
		return 1;
	}
	return 0;
}
