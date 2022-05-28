/* Manager for file/directory operations in the Homebrew Browser. */

int remove_file(char* path);
int remove_dir(char* path);
int delete_dir_files(char* path);
int create_dir(char* path);