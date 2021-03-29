/*
   miniunz.c
   Version 1.01e, February 12th, 2005

   Copyright (C) 1998-2005 Gilles Vollant
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include "unzip.h"
#include "common.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

int zip_size = 0;
long extract_part_size = 0;
long zip_progress = 0;
bool cancel_extract = false;
bool hbb_updating = false;
int unzip_file_counter = 0;
int unzip_file_count = 0;
char no_unzip_list[10][300];
int no_unzip_count = 0;

static int mymkdir(const char* dirname)
{
    int ret=0;
    ret = mkdir (dirname,0775);
    return ret;
}

int makedir (char *newdir)
{
  char *buffer ;
  char *p;
  int  len = (int)strlen(newdir);

  if (len <= 0)
    return 0;

  buffer = (char*)malloc(len+1);
  strcpy(buffer,newdir);

  if (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  if (mymkdir(buffer) == 0)
    {
      free(buffer);
      return 1;
    }

  p = buffer+1;
  while (1)
    {
      char hold;

      while(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          free(buffer);
          return 0;
        }
      if (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  return 1;
}

static int do_extract_currentfile(unzFile uf,const int* popt_extract_without_path,int* popt_overwrite,const char* password)
{
    char filename_inzip[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info file_info;
    err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

    if (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    if ((*filename_withoutpath)=='\0')
    {
        if ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    else
    {
        char* write_filename;
        int skip=0;

        if ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        else
            write_filename = filename_withoutpath;
			
		bool ok_to_unzip = true;
			
		int d;
		for (d = 0; d < no_unzip_count; d++) {
			//printf("SECOND = %s\n", no_unzip_list[d]);
			if (strcmp(no_unzip_list[d], write_filename) == 0) {
				FILE *f = fopen(write_filename, "rb");
				if (f != NULL) {
					ok_to_unzip = false;
				}
			}
		}
		
		if (ok_to_unzip == true) {

			err = unzOpenCurrentFilePassword(uf,password);
			if (err!=UNZ_OK)
			{
				printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
			}

			if (((*popt_overwrite)==0) && (err==UNZ_OK))
			{
				char rep=0;
				FILE* ftestexist;
				ftestexist = fopen(write_filename,"rb");
				if (ftestexist!=NULL)
				{
					fclose(ftestexist);
					do
					{
						char answer[128];
						int ret;

						printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
						ret = scanf("%1s",answer);
						if (ret != 1)
						{
						   exit(EXIT_FAILURE);
						}
						rep = answer[0] ;
						if ((rep>='a') && (rep<='z'))
							rep -= 0x20;
					}
					while ((rep!='Y') && (rep!='N') && (rep!='A'));
				}

				if (rep == 'N')
					skip = 1;

				if (rep == 'A')
					*popt_overwrite=1;
			}

			if ((skip==0) && (err==UNZ_OK))
			{
				fout=fopen(write_filename,"wb");

				/* some zipfile don't contain directory alone before file */
				if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
									(filename_withoutpath!=(char*)filename_inzip))
				{
					char c=*(filename_withoutpath-1);
					*(filename_withoutpath-1)='\0';
					makedir(write_filename);
					*(filename_withoutpath-1)=c;
					fout=fopen(write_filename,"wb");
				}

				if (fout==NULL)
				{
					printf("error opening %s\n",write_filename);
				}
			}
			
			if (fout!=NULL)
			{
				//printf(" extracting: %s\n",write_filename);
				
				int temp_size = 0;
				do
				{
					err = unzReadCurrentFile(uf,buf,size_buf);
					
					if (err<0)
					{
						printf("error %d with zipfile in unzReadCurrentFile\n",err);
						break;
					}
					else if (err>0)
					{

						if (fwrite(buf,err,1,fout)!=1)
						{
							printf("error in writing extracted file\n");
							err=UNZ_ERRNO;
							break;
						}
						
						zip_progress += size_buf;
						temp_size += size_buf;
						//updating_current_size += size_buf;
						
						if (cancel_extract == true && setting_prompt_cancel == false) {
							err = -1;
						}
						else if ((cancel_download == true || cancel_extract == true) && setting_prompt_cancel == true && cancel_confirmed == true) {
							cancel_download = false;
							cancel_extract = true;
							err = -1;
						}
						if (hbb_updating == true && cancel_extract == true) {
							err = -1;
						}
					}
						
				}
				while (err>0);
				
				
				if (temp_size > file_info.uncompressed_size) {
					zip_progress -= temp_size;
					zip_progress += file_info.uncompressed_size;
					//updating_current_size -= temp_size;
					//updating_current_size += file_info.uncompressed_size;
				}
				
				if (fout)
						fclose(fout);
				}

			if (err==UNZ_OK)
			{
				err = unzCloseCurrentFile (uf);
				if (err!=UNZ_OK)
				{
					printf("error %d with zipfile in unzCloseCurrentFile\n",err);
				}
			}
			else
				unzCloseCurrentFile(uf); /* don't lose the error */
		}
    }

    free(buf);
    return err;
}

int zipInfo(unzFile uf)
{
    uLong i;
    unz_global_info gi;
    int err;
	uLong totalSize = 0;
	unz_file_info file_info;
	char filename_inzip[256];

    err = unzGetGlobalInfo (uf,&gi);

    for (i=0;i<gi.number_entry;i++)
    {
		err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
		
		if (err!=UNZ_OK)
			return err;
		
		totalSize += file_info.uncompressed_size;

        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
				return err;
        }
    }
	
	err = unzGoToFirstFile(uf);
		if (err!=UNZ_OK)
			return err;

    return totalSize;
}


int extractZip(unzFile uf,int opt_extract_without_path,int opt_overwrite,const char* password)
{
    zip_progress = 0;
	zip_size = zipInfo(uf);
	extract_part_size = (int) (zip_size / 100);
	//printf("Zipped up total size: %i\n",zip_size);
	
	uLong i;
    unz_global_info gi;
    int err;

    err = unzGetGlobalInfo (uf,&gi);
    if (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);
    }
		
	unzip_file_count = gi.number_entry;
	
    for (i=0;i<gi.number_entry;i++)
    {
		unzip_file_counter = (i+1);
		
		//printf("%li / %li done, zip progress = %li / %i\n", (i+1), gi.number_entry, zip_progress, zip_size);
		printf(".");
        if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password) != UNZ_OK)
            break;

        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}

int extractZipOnefile(unzFile uf,const char* filename,int opt_extract_without_path,int opt_overwrite,const char* password)
{
    if (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        return 2;
    }

    if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password) == UNZ_OK)
        return 0;
    else
        return 1;
}


