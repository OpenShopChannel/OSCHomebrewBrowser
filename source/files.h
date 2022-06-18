/* Manager for file/directory operations in the Homebrew Browser. */
#include <stdio.h>

int remove_file(char *path);
int remove_dir(char *path);
int delete_dir_files(char *path);
int create_dir(char *path);
int create_parent_dirs(char *path);
char *app_path(char *app_name, char *app_file);
FILE *hbb_fopen(char *filename, const char *mode);
char *device_name();
char *temp_path();
