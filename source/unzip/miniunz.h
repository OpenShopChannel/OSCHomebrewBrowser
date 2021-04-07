
#ifndef _miniunz_H
#define _miniunz_H

#ifdef __cplusplus
extern "C" {
#endif

int extractZip(unzFile uf,int opt_extract_without_path,int opt_overwrite,const char* password);
int extractZipOnefile(unzFile uf,const char* filename,int opt_extract_without_path,int opt_overwrite,const char* password);
int makedir(char *newdir);
int zipInfo(unzFile uf);

extern int zip_size;
extern long zip_progress;
extern long extract_part_size;
extern bool cancel_extract;
extern bool hbb_updating;
extern int unzip_file_counter;
extern int unzip_file_count;
extern char no_unzip_list[10][300];
extern int no_unzip_count;

#ifdef __cplusplus
}
#endif

#endif
