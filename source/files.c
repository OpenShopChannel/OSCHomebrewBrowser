/* Manager for file/directory operations in the Homebrew Browser. */
#include <stdio.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/unistd.h>

// Removes a file
int remove_file(char *path) {
	FILE *f = fopen(path, "rb");

	// File not found
	if (f == NULL) {
		return 1;
	}
	fclose(f);
	unlink(path);

	// Check file was removed
	f = fopen(path, "rb");
	if (f == NULL) {
		return 1;
	}

	fclose(f);
	return -1;
}

// Removes a directory
int remove_dir(char *path) {

	if (opendir(path)) {
		unlink(path);
		if (opendir(path)) {
			return -1;
		}
	}

	return 1;
}

// Delete all files in a directory
int delete_dir_files(char *path) {
	struct dirent *dent = NULL;
	struct stat st;

	DIR *dir = opendir(path);
	if (dir != NULL) {
		char temp_path[MAXPATHLEN];
		while ((dent = readdir(dir)) != NULL) {
			strcpy(temp_path, path);
			strcat(temp_path, "/");
			strcat(temp_path, dent->d_name);
			stat(temp_path, &st);

			if (!(S_ISDIR(st.st_mode))) {
				remove_file(temp_path);
			}
		}
	}
	closedir(dir);

	return 1;
}

// Creates a directory
int create_dir(char *path) {
	if (!opendir(path)) {
		mkdir(path, 0777);
		if (!opendir(path)) {
			return -1;
		}
	}

	return 1;
}