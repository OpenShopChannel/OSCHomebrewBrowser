/* Manager for file/directory operations in the Homebrew Browser. */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/unistd.h>

#include "settings.h"

// Removes a file at the device-localized path.
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

// Creates parent directories for the given path.
int create_parent_dirs(char *path) {
	char *temp_path = (char *)malloc(strlen(path) + 1);
	char *seperator = strchr(path, '/');

	while (seperator != NULL) {
		// Separate the current segment from the full path.
		int path_len = seperator - path;
		memcpy(temp_path, path, path_len);
		temp_path[path_len] = '\0';

		// Create!
		int result = create_dir(temp_path);
		if (result != 1) {
			free(temp_path);
			return -1;
		}

		// Find the next separator.
		seperator = strchr(seperator + 1, '/');
	}

	free(temp_path);
	return 0;
}

// Returns the given path with the device name appended.
char *device_path(char *path) {
	// "usb:/" is 5 characters at most, plus one for a null terminator.
	int length = 5 + strlen(path) + 1;
	char *string = (char *)malloc(length);

	if (setting_use_sd) {
		strlcpy(string, "sd:/", length);
	} else {
		strlcpy(string, "usb:/", length);
	}
	strlcpy(string, path, length);

	return string;
}

// Synthesizes a path to a Homebrew application's
// installation path based on the default device.
// e.g. app_path("testing", "hello.txt") becomes
// sd:/apps/testing/hello.txt
char *app_path(char *app_name, char *app_file) {
	// "apps/" (5) + len(app_name) + "/" (1) + len(app_file) + null (1)
	size_t length = 7 + strlen(app_name) + strlen(app_file);

	char *buf = (char *)malloc(length);
	snprintf(buf, length, "apps/%s/%s", app_name, app_file);

	// Create a device-relative path.
	char *path = device_path(buf);
	free(buf);

	return path;
}

// fopens a file based on the default device for the Homebrew Browser.
FILE *hbb_fopen(char *filename, const char *mode) {
	char *path = app_path("homebrew_browser", filename);

	FILE *f = fopen(path, mode);
	free(path);
	return f;
}