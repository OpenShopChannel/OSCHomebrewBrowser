
#ifndef _miniunz_H
#define _miniunz_H

#ifdef __cplusplus
extern "C" {
#endif

int extractZip(unzFile uf,int opt_extract_without_path,int opt_overwrite,const char* password);
int extractZipOnefile(unzFile uf,const char* filename,int opt_extract_without_path,int opt_overwrite,const char* password);
int makedir(char *newdir);
int zipInfo(unzFile uf);

int zip_size;
long zip_progress;
long extract_part_size;
bool cancel_extract;
bool hbb_updating;
int unzip_file_counter;
int unzip_file_count;
char no_unzip_list[10][300];
int no_unzip_count;

#ifdef __cplusplus
}
#endif

#endif
