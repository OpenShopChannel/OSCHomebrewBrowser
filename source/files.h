/* Manager for file/directory operations in the Homebrew Browser. */
#include <stdio.h>

bool remove_file(char *path);
bool remove_dir(char *path);
bool delete_dir_files(char *path);
bool create_dir(char *path);
bool create_parent_dirs(char *path);
char *app_path(char *app_name, char *app_file);
FILE *hbb_fopen(char *filename, const char *mode);
char *device_name();
char *temp_path();
