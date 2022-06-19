/*

Homebrew Browser -- a simple way to view and install Homebrew releases via the Wii

Author: teknecal

Using some source from ftpii v0.0.5
ftpii Source Code Copyright (C) 2008 Joseph Jordan <joe.ftpii@psychlaw.com.au>

*/
#include <errno.h>
#include <fat.h>
#include <math.h>
#include <wiisocket.h>
#include <curl/curl.h>
#include <asndlib.h>
#include <ogc/lwp_watchdog.h>
#include <ogcsys.h>
#include <ogc/pad.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <zlib.h>
#include "mp3player.h"
#include <mxml.h>
#include "unzip/unzip.h"
#include "unzip/miniunz.h"
#include "GRRLIB/GRRLIB.h"
#include "common.h"
#include "files.h"
#include "net_requests.h"
#include "settings.h"

#define FONTSIZE_SMALL 18

const char *CRLF = "\r\n";
const u32 CRLF_LENGTH = 2;
char esid[50];

static GXRModeObj *vmode;
int xfb_height = 0;

struct repo_struct repo_list[200];

struct updated_apps_struct updated_apps_list[1600];

// List to show
struct homebrew_struct homebrew_list[1600];
struct text_struct text_list[1600];

struct homebrew_struct emulators_list[300];
struct homebrew_struct games_list[600];
struct homebrew_struct media_list[300];
struct homebrew_struct utilities_list[300];
struct homebrew_struct demos_list[300];

// Total list
struct homebrew_struct total_list[600];

// Temp list
struct sort_homebrew_struct temp_list[600];
struct homebrew_struct temp_list2[600];
struct sort_homebrew_struct temp1_list[2];

// Temp list to use to download/extract/delete
struct homebrew_struct store_homebrew_list[2];

// Folders exist list
struct sort_homebrew_struct folders_list[500];

// Apps to not manage
struct sort_homebrew_struct no_manage_list[500];

static volatile u8 reset = 0;
static lwp_t reset_thread;
static lwp_t icons_thread;
static lwp_t download_thread;
static lwp_t delete_thread;
static lwp_t rating_thread;
static lwp_t update_rating_thread;
static lwp_t request_thread;

int category_old_selection = 2;

long remote_hb_size = 0;
char update_text[1000];

bool codemii_backup = false;
bool could_connect = false;

int download_in_progress = 0;
int extract_in_progress = 0;
int delete_in_progress = 0;
bool get_rating_in_progress = false;
char rating_number[5];
int selected_app = 0;
int total_list_count = 0;

bool show_updated_apps = false;
int updated_apps_count = 0;
int repo_count = 0;
int timeout_counter = 0;

bool download_icon_sleeping = false;

int download_part_size = 0;
long download_progress_counter = 0;

int error_number = 0;
bool cancel_download = false;
bool cancel_delete = false;

bool sd_card_update = true;
long long sd_card_free = 0;
int sd_free = 0;

int load_icon = 0;
int download_icon = 0;

int sort_up_down = 0;
bool in_menu = false;

bool cancel_confirmed = false;

int updating = -1;
int new_updating = 0;
int updating_total_size = 0;
int updating_part_size = 0;
long updating_current_size = 0;

int progress_number = 0;
int progress_size = 0;
bool changing_cat = false;
bool exiting = false;
bool hbb_app_about = false;
int retry = 0;
int update_xml = 0;

char temp_name[100];
char uppername[100];

int hbb_string_len = 0;
int hbb_null_len = 0;

bool list_received = false;
int no_manage_count = 0;

// Directory vars
char filename[MAXPATHLEN];
char foldername[MAXPATHLEN];
DIR* dir;
struct dirent *dent;
struct stat st;

// Time info
struct tm * timeinfo;
time_t app_time;

static void reset_called() {
	reset = 1;
}

static void *run_reset_thread(void *arg) {
	while (!reset && !(WPAD_ButtonsHeld(0) & WPAD_BUTTON_HOME) && !(PAD_ButtonsDown(0) & PAD_BUTTON_START) ) {
		sleep(1);
		WPAD_ScanPads();
		// Go offline
		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_1 || PAD_ButtonsDown(0) & PAD_BUTTON_Y) {
			setting_online = false;
			printf("\nNow working in offline mode.\n");
			//add_to_log("Now working in offline mode.");
		}
		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_2 || PAD_ButtonsDown(0) & PAD_BUTTON_X) {
			setting_repo_revert = true;
			setting_repo = 0;
			printf("\nReverting to OSCWii.org repository.\n");
			//add_to_log("Reverting to CodeMii.com repository.");
		}
		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_B || PAD_ButtonsDown(0) & PAD_BUTTON_B) {
			cancel_download = true;
			cancel_extract = true;
			//add_to_log("Cancelling download/extract.");
		}
	}
	printf("\nHomebrew Browser shutting down...\n");
	exit(0);
}

u8 initialise_reset_button() {
	s32 result = LWP_CreateThread(&reset_thread, run_reset_thread, NULL, NULL, 0, 80);
	if (result == 0) SYS_SetResetCallback(reset_called);
	return !result;
}

static void *run_icons_thread(void *arg) {
	sleep(5);

	int x;
	for (x = 0; x < array_length(total_list); x++) {

		while (changing_cat == true) {
			download_icon_sleeping = true;
			sleep(3);
		}

		while (download_in_progress == true || extract_in_progress == true || delete_in_progress == true || in_menu == true) {
			download_icon_sleeping = true;
			sleep(3);
		}

		download_icon_sleeping = false;

		bool update_img_file = false;

		char img_path[150];
		strcpy(img_path, "apps/homebrew_browser/temp/");
		strcat(img_path, total_list[x].name);
		strcat(img_path, ".png");

		FILE *f = hbb_fopen(img_path, "rb");

		// If file doesn't exist or can't open it then we can grab the latest file
		if (f == NULL) {
			update_img_file = true;
		}
		// Open file and check file length for changes
		else {
			fseek (f , 0, SEEK_END);
			int local_img_size = ftell (f);
			rewind(f);
			fclose(f);

			// Check if remote image size doesn't match local image size
			if (local_img_size != total_list[x].img_size) {
				update_img_file = true;
			}
			else {
				// Load image into memory if image size matches
				if (local_img_size == total_list[x].img_size) {
					total_list[x].file_found = load_file_to_memory(img_path, &total_list[x].content);
					if (total_list[x].file_found == 1) {
						total_list[x].file_found = 2;
					}
				}
				else {
					total_list[x].file_found = -1;
				}

				// Load text to memory
				//if (total_list[x].str_name == NULL && total_list[x].str_short_description == NULL) {
					//total_list[x].str_name = GRRLIB_TextToTexture(total_list[x].app_name, FONTSIZE_SMALL, 0x575757);
					//total_list[x].str_short_description = GRRLIB_TextToTexture(total_list[x].app_short_description, FONTSIZE_SMALL, 0x575757);
				//}


				int y;
				for (y = 0; y < array_length (homebrew_list); y++) {
					if (strcmp (total_list[x].name, homebrew_list[y].name) == 0) {
						homebrew_list[y].file_found = total_list[x].file_found;
						//homebrew_list[y].str_name = total_list[x].str_name;
						//homebrew_list[y].str_short_description = total_list[x].str_short_description;
						homebrew_list[y].content = total_list[x].content;
					}
				}

				for (y = 0; y < array_length (emulators_list); y++) {
					if (strcmp (total_list[x].name, emulators_list[y].name) == 0) {
						emulators_list[y].file_found = total_list[x].file_found;
						//emulators_list[y].str_name = total_list[x].str_name;
						//emulators_list[y].str_short_description = total_list[x].str_short_description;
						emulators_list[y].content = total_list[x].content;
					}
				}

				for (y = 0; y < array_length (games_list); y++) {
					if (strcmp (total_list[x].name, games_list[y].name) == 0) {
						games_list[y].file_found = total_list[x].file_found;
						//games_list[y].str_name = total_list[x].str_name;
						//games_list[y].str_short_description = total_list[x].str_short_description;
						games_list[y].content = total_list[x].content;
					}
				}

				for (y = 0; y < array_length (media_list); y++) {
					if (strcmp (total_list[x].name, media_list[y].name) == 0) {
						media_list[y].file_found = total_list[x].file_found;
						//media_list[y].str_name = total_list[x].str_name;
						//media_list[y].str_short_description = total_list[x].str_short_description;
						media_list[y].content = total_list[x].content;
					}
				}

				for (y = 0; y < array_length (utilities_list); y++) {
					if (strcmp (total_list[x].name, utilities_list[y].name) == 0) {
						utilities_list[y].file_found = total_list[x].file_found;
						//utilities_list[y].str_name = total_list[x].str_name;
						//utilities_list[y].str_short_description = total_list[x].str_short_description;
						utilities_list[y].content = total_list[x].content;
					}
				}

				for (y = 0; y < array_length (demos_list); y++) {
					if (strcmp (total_list[x].name, demos_list[y].name) == 0) {
						demos_list[y].file_found = total_list[x].file_found;
						//demos_list[y].str_name = total_list[x].str_name;
						//demos_list[y].str_short_description = total_list[x].str_short_description;
						demos_list[y].content = total_list[x].content;
					}
				}
			}
		}

		// Grab the new img file
		if (update_img_file == true && setting_online == true) {
			s32 create_result;

			char temp_path[200];
			strcpy(temp_path, rootdir);
			strcat(temp_path, "apps/homebrew_browser/temp/");

			create_result = create_and_request_file(temp_path, total_list[x].name, ".png");

			if (create_result == 1) {

				// Make sure that image file matches the correct size
				f = fopen(img_path, "rb");

				// If file doesn't exist or can't open it then we can grab the latest file
				if (f == NULL) {
					update_img_file = true;
				}
				// Open file and check file length for changes
				else {
					fseek (f , 0, SEEK_END);
					int local_img_size = ftell (f);
					rewind(f);
					fclose(f);

					if (local_img_size == total_list[x].img_size) {
						total_list[x].file_found = load_file_to_memory(img_path, &total_list[x].content);
						if (total_list[x].file_found == 1) {
							total_list[x].file_found = 2;
						}
					}
					else {
						total_list[x].file_found = -1;
					}

					// Load text to memory
					//if (total_list[x].str_name == NULL && total_list[x].str_short_description == NULL) {
						//total_list[x].str_name = GRRLIB_TextToTexture(total_list[x].app_name, FONTSIZE_SMALL, 0x575757);
						//total_list[x].str_short_description = GRRLIB_TextToTexture(total_list[x].app_short_description, FONTSIZE_SMALL, 0x575757);
					//}


					int y;
					for (y = 0; y < array_length (homebrew_list); y++) {
						if (strcmp (total_list[x].name, homebrew_list[y].name) == 0) {
							homebrew_list[y].file_found = total_list[x].file_found;
							//homebrew_list[y].str_name = total_list[x].str_name;
							//homebrew_list[y].str_short_description = total_list[x].str_short_description;
							homebrew_list[y].content = total_list[x].content;
						}
					}
					while (changing_cat == true) {
						download_icon_sleeping = true;
						sleep(3);
					}
					for (y = 0; y < array_length (emulators_list); y++) {
						if (strcmp (total_list[x].name, emulators_list[y].name) == 0) {
							emulators_list[y].file_found = total_list[x].file_found;
							//emulators_list[y].str_name = total_list[x].str_name;
							//emulators_list[y].str_short_description = total_list[x].str_short_description;
							emulators_list[y].content = total_list[x].content;
						}
					}
					while (changing_cat == true) {
						download_icon_sleeping = true;
						sleep(3);
					}
					for (y = 0; y < array_length (games_list); y++) {
						if (strcmp (total_list[x].name, games_list[y].name) == 0) {
							games_list[y].file_found = total_list[x].file_found;
							//games_list[y].str_name = total_list[x].str_name;
							//games_list[y].str_short_description = total_list[x].str_short_description;
							games_list[y].content = total_list[x].content;
						}
					}
					while (changing_cat == true) {
						download_icon_sleeping = true;
						sleep(3);
					}
					for (y = 0; y < array_length (media_list); y++) {
						if (strcmp (total_list[x].name, media_list[y].name) == 0) {
							media_list[y].file_found = total_list[x].file_found;
							//media_list[y].str_name = total_list[x].str_name;
							//media_list[y].str_short_description = total_list[x].str_short_description;
							media_list[y].content = total_list[x].content;
						}
					}
					while (changing_cat == true) {
						download_icon_sleeping = true;
						sleep(3);
					}
					for (y = 0; y < array_length (utilities_list); y++) {
						if (strcmp (total_list[x].name, utilities_list[y].name) == 0) {
							utilities_list[y].file_found = total_list[x].file_found;
							//utilities_list[y].str_name = total_list[x].str_name;
							//utilities_list[y].str_short_description = total_list[x].str_short_description;
							utilities_list[y].content = total_list[x].content;
						}
					}
					while (changing_cat == true) {
						download_icon_sleeping = true;
						sleep(3);
					}
					for (y = 0; y < array_length (demos_list); y++) {
						if (strcmp (total_list[x].name, demos_list[y].name) == 0) {
							demos_list[y].file_found = total_list[x].file_found;
							//demos_list[y].str_name = total_list[x].str_name;
							//demos_list[y].str_short_description = total_list[x].str_short_description;
							demos_list[y].content = total_list[x].content;
						}
					}
				}
			}
		}

		// Sleep longer if downloaded image file
		if (update_img_file == true) {
			sleep(2);
		}
		else {
			//usleep(50000);
		}
		download_icon = x;
	}
	download_icon = 0;
	return 0;
}

u8 load_icons() {
	s32 result = LWP_CreateThread(&icons_thread, run_icons_thread, NULL, NULL, 0, 80);
	return result;
}


static void *run_download_thread(void *arg) {

	clear_store_list();
	store_homebrew_list[0] = homebrew_list[selected_app];

	if (download_icon > 0) {
		while (download_icon_sleeping != true) {
			usleep(10000);
		}
	}

	bool download_status = true;
	download_progress_counter = 0;

	// Check if there is enough space on SD card
	if (setting_check_size == true) {
		int check_size = ((store_homebrew_list[0].total_app_size / 1024) / 1024) + ((store_homebrew_list[0].app_total_size / 1024) / 1024);
		if (sd_card_free <= check_size) {
			download_status = false;
			error_number = 9;
		}
	}

	// Check we are still connected to the Wireless
	if (ensure_wifi() == false) {
		download_status = false;
		error_number = 10;
	}

	if (download_status == true) {
		download_part_size = (int) (store_homebrew_list[0].total_app_size / 100);

		// Download zip file
		char zipfile[512];
		strcpy(zipfile, "/");
		strcat(zipfile, store_homebrew_list[0].name);
		strcat(zipfile, ".zip");

		char temp_path[200];
		strcpy(temp_path, rootdir);
		strcat(temp_path, "apps/");

		if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {

			if (create_and_request_file(temp_path, store_homebrew_list[0].user_dirname, zipfile) != 1) {
				download_status = false;
				error_number = 1;
			}

		}
		else {
			if (create_and_request_file(temp_path, store_homebrew_list[0].name, zipfile) != 1) {
				download_status = false;
				error_number = 1;
			}
		}

		// Check downloaded file size
		/*char extractzipfile[200];
		strcpy(extractzipfile, "sd:/apps/");
		if (setting_use_sd == false) {
			strcpy(extractzipfile, "usb:/apps/");
		}
		if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
			strcat(extractzipfile, store_homebrew_list[0].user_dirname);
		}
		else {
			strcat(extractzipfile, store_homebrew_list[0].name);
		}

		strcat(extractzipfile, "/");
		strcat(extractzipfile, store_homebrew_list[0].name);
		strcat(extractzipfile, ".zip");

		FILE *fcheck = fopen(extractzipfile, "rb");
		if (fcheck != NULL) {
			fseek (fcheck , 0, SEEK_END);
			int local_fcheck = ftell (fcheck);
			rewind(fcheck);
			fclose(fcheck);

			if (local_fcheck != store_homebrew_list[0].total_app_size) {
				download_status = false;
				error_number = 1;
			}
		}
		else {
			download_status = false;
			error_number = 1;
		}*/
	}

	// Directories to create
	if (store_homebrew_list[0].folders != NULL && download_status == true) {
		char folders[1000];
		strcpy(folders, store_homebrew_list[0].folders);

		char *split_tok;
		split_tok = strtok (folders,";");

		while (split_tok != NULL && download_status == true) {
			char temp_create[150] = "sd:";
			if (setting_use_sd == false) {
				strcpy(temp_create,"usb:");
			}
			strcat(temp_create, split_tok);
			if (create_dir(temp_create) == false) {
				download_status = false;
				error_number = 2;
			}
			split_tok = strtok (NULL, ";");
		}
	}

	// Download the icon.png
	if (download_status == true) {

		char icon_path[100] = "sd:/apps/";
		if (setting_use_sd == false) {
			strcpy(icon_path,"usb:/apps/");
		}

		// Only download the icon file if user hasn't got this app installed or doesn't have the icon.png
		if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
			strcat(icon_path, store_homebrew_list[0].user_dirname);
		}
		else {
			strcat(icon_path, store_homebrew_list[0].name);
		}
		strcat(icon_path, "/icon.png");

		FILE *f = fopen(icon_path, "rb");

		// Problems opening the file? Then download the icon.png
		if (f == NULL) {
			if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
				if (download_file(store_homebrew_list[0].user_dirname, "icon.png") != 1) {
					download_status = false;
					error_number = 3;
				}
			}
			else {
				if (download_file(store_homebrew_list[0].name, "icon.png") != 1) {
					download_status = false;
					error_number = 3;
				}
			}

			usleep(150000);

		}
		else {
			fclose(f);

			if (setting_update_icon == true) {
				if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
					if (download_file(store_homebrew_list[0].user_dirname, "icon.png") != 1) {
						download_status = false;
						error_number = 3;
					}
				}
				else {
					if (download_file(store_homebrew_list[0].name, "icon.png") != 1) {
						download_status = false;
						error_number = 3;
					}
				}
			}

			usleep(150000);
		}
	}

	// Failed or success?
	if (download_status == false) {
		download_in_progress = -1;
		download_progress_counter = 0;

		// Remove incomplete zip file
		char extractzipfile[512];
		strcpy(extractzipfile, "sd:/apps/");
		if (setting_use_sd == false) {
			strcpy(extractzipfile, "usb:/apps/");
		}
		if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
			strcat(extractzipfile, store_homebrew_list[0].user_dirname);
		}
		else {
			strcat(extractzipfile, store_homebrew_list[0].name);
		}

		strcat(extractzipfile, "/");
		strcat(extractzipfile, store_homebrew_list[0].name);
		strcat(extractzipfile, ".zip");

		remove_file(extractzipfile);

		if (hbb_app_about == false && updating == -1) {
			download_in_progress = false;
			extract_in_progress = false;
			delete_in_progress = false;
			cancel_download = false;
			cancel_extract = false;
		}

		return 0;
	}
	else {
		extract_in_progress = true;
		download_in_progress = false;
		download_progress_counter = 0;
	}

	bool extract_status = true;

	// Delete boot.elf file
	char del_file[150] = "sd:/apps/";
	if (setting_use_sd == false) {
		strcpy(del_file,"usb:/apps/");
	}
	strcat(del_file, store_homebrew_list[0].name);
	strcat(del_file, "/boot.elf");
	remove_file(del_file);

	// Delete boot.dol file or theme.zip
	char del_file1[150] = "sd:/apps/";
	char del_file2[150];
	if (setting_use_sd == false) {
		strcpy(del_file1,"usb:/apps/");
	}
	//strcat(del_file1, store_homebrew_list[0].name);
	if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
		strcat(del_file1, store_homebrew_list[0].user_dirname);
	}
	else {
		strcat(del_file1, store_homebrew_list[0].name);
	}
	strcpy(del_file2, del_file1);
	strcat(del_file1, "/boot.dol");
	strcat(del_file2, "/theme.zip");
	remove_file(del_file1);
	remove_file(del_file2);


	// Put files not to extract in array
	no_unzip_count = 0;

	char folders[300];
	strcpy(folders, store_homebrew_list[0].files_no_extract);
	char *split_tok;
	split_tok = strtok (folders,";");

	while (split_tok != NULL) {
		strcpy(no_unzip_list[no_unzip_count], split_tok);
		no_unzip_count++;
		split_tok = strtok (NULL, ";");
	}

	int extract_attempt = 0;
	char extractzipfile[512];
	strcpy(extractzipfile, "sd:/apps/");
	if (setting_use_sd == false) {
		strcpy(extractzipfile, "usb:/apps/");
	}
	//strcat(extractzipfile, store_homebrew_list[0].name);
	if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
		strcat(extractzipfile, store_homebrew_list[0].user_dirname);
	}
	else {
		strcat(extractzipfile, store_homebrew_list[0].name);
	}

	strcat(extractzipfile, "/");
	strcat(extractzipfile, store_homebrew_list[0].name);
	strcat(extractzipfile, ".zip");

	while (extract_attempt < 3) {
		if (setting_use_sd == true) {
			if (unzipArchive(extractzipfile, "sd:/") == true) {

				if (strcmp(store_homebrew_list[0].name,"ftpii") == 0 && strcmp(store_homebrew_list[0].user_dirname,"ftpii") != 0) {
					char renamed[100] = "sd:/apps/";
					strcat(renamed,store_homebrew_list[0].user_dirname);
					strcat(renamed,"/boot.dol");

					char renamed2[100] = "sd:/apps/";
					strcat(renamed2,store_homebrew_list[0].user_dirname);
					strcat(renamed2,"/meta.xml");

					remove_file(renamed);
					rename("sd:/apps/ftpii/boot.dol",renamed);
					remove_file(renamed2);
					rename("sd:/apps/ftpii/meta.xml",renamed2);
					remove_dir("sd:/apps/ftpii/");
				}

				break;
			}
			else {
				extract_attempt++;
			}
		}
		else {
			if (unzipArchive(extractzipfile, "usb:/") == true) {

				if (strcmp(store_homebrew_list[0].name,"ftpii") == 0 && strcmp(store_homebrew_list[0].user_dirname,"ftpii") != 0) {
					char renamed[100] = "usb:/apps/";
					strcat(renamed,store_homebrew_list[0].user_dirname);
					strcat(renamed,"/boot.dol");

					char renamed2[100] = "usb:/apps/";
					strcat(renamed2,store_homebrew_list[0].user_dirname);
					strcat(renamed2,"/meta.xml");

					remove_file(renamed);
					rename("usb:/apps/ftpii/boot.dol",renamed);
					remove_file(renamed2);
					rename("usb:/apps/ftpii/meta.xml",renamed2);
					remove_dir("usb:/apps/ftpii/");
				}

				break;
			}
			else {
				extract_attempt++;
			}
		}
	}

	if (cancel_extract == true) {
		extract_status = false;
		extract_in_progress = -1;
		error_number = 4;
	}

	if (extract_attempt >= 3) {
		extract_status = false;
		extract_in_progress = -1;
		error_number = 4;
	}

	remove_file(extractzipfile);
	usleep(200000);

	// We need to recheck the boot.dol/elf file size as the file has changed
	char boot_path[100] = "sd:/apps/";
	char theme_path[100];
	if (setting_use_sd == false) {
		strcpy(boot_path,"usb:/apps/");
	}
	if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
		strcat(boot_path, store_homebrew_list[0].user_dirname);
	}
	else {
		strcat(boot_path, store_homebrew_list[0].name);
	}
	strcpy(theme_path, boot_path);
	strcat(theme_path, "/theme.zip");
	strcat(boot_path, "/boot.");
	strcat(boot_path, store_homebrew_list[0].boot_ext);

	FILE *f = fopen(boot_path, "rb");

	// Problems opening the file?
	if (f == NULL) {

		FILE *ftheme = fopen(theme_path, "rb");

		if (ftheme == NULL) {
			if (extract_status == true) {
				extract_status = false;
				error_number = 5;
			}
		}
		else {
			// Open file and get the file size
			fseek (ftheme , 0, SEEK_END);
			store_homebrew_list[0].local_app_size = ftell (ftheme);
			//homebrew_list[selected_app].local_app_size = store_homebrew_list[0].local_app_size;
			rewind (ftheme);
			fclose(ftheme);
		}
	}
	else {
		// Open file and get the file size
		fseek (f , 0, SEEK_END);
		store_homebrew_list[0].local_app_size = ftell (f);
		//homebrew_list[selected_app].local_app_size = store_homebrew_list[0].local_app_size;
		rewind (f);
		fclose(f);

		// Rename it to what they had before (hb sorter)
		if (store_homebrew_list[0].boot_bak == true) {
			char boot_path_bak[100];
			strcpy(boot_path_bak, boot_path);
			strcat(boot_path_bak, ".bak");
			rename(boot_path, boot_path_bak);
		}

	}

	// Failed or success?
	if (extract_status == false) {
		extract_in_progress = -1;
	}
	else {
		store_homebrew_list[0].in_download_queue = false;
		extract_in_progress = false;
		sd_card_update = true;

		store_update_lists();

		if (updating >= 0 && updating < array_length (homebrew_list)) {
			new_updating++;
		}
		if (update_xml == 1) {
			update_xml = 2;
		}
	}

	if (hbb_app_about == false) {
		download_in_progress = false;
		if (extract_in_progress != -1) {
			extract_in_progress = false;
		}
		delete_in_progress = false;
		cancel_download = false;
		cancel_extract = false;
	}

	// Update the homebrew list
	store_update_lists();

	return 0;
}

u8 initialise_download() {
	s32 result = LWP_CreateThread(&download_thread, run_download_thread, NULL, NULL, 16384, 80);
	return result;
}

static void *run_delete_thread(void *arg) {

	if (error_number == 0) {
		clear_store_list();
		store_homebrew_list[0] = homebrew_list[selected_app];
	}

	bool delete_status = true;
	bool ok_to_del = true;
	char no_del_list[50][300];
	int no_del_count = 0;
	//error_number = 7;

	// Directories to delete all files from
	if (store_homebrew_list[0].folders != NULL && delete_status == true && cancel_delete == false) {
		char folders[1000];
		strcpy(folders, store_homebrew_list[0].folders_no_del);
		char *split_tok;
		split_tok = strtok (folders,";");

		while (split_tok != NULL) {
			strcpy(no_del_list[no_del_count], split_tok);
			no_del_count++;
			split_tok = strtok (NULL, ";");
		}

		char folders1[1000];
		strcpy(folders1, store_homebrew_list[0].folders);

		char *split_tok1;
		split_tok1 = strtok (folders1,";");

		while (split_tok1 != NULL) {

			//printf("FIRST = %s\n", split_tok1);

			ok_to_del = true;

			int d;
			for (d = 0; d < no_del_count; d++) {
				//printf("SECOND = %s\n", no_del_list[d].folders);
				if (strcmp(no_del_list[d], split_tok1) == 0) {
					ok_to_del = false;
				}
			}

			if (ok_to_del == true) {
				//printf("OK TO DEL\n");
				char temp_del[200] = "sd:";
				if (setting_use_sd == false) {
					strcpy(temp_del,"usb:");
				}
				strcat(temp_del, split_tok1);

				delete_dir_files(temp_del);
			}

			split_tok1 = strtok (NULL, ";");

		}

		/*char folders[1000];
		strcpy(folders, store_homebrew_list[0].folders);

		char *split_tok;
		split_tok = strtok (folders,";");

		while (split_tok != NULL && delete_status == true) {

			// NEW DELETE METHOD
			bool ok_to_del = true;
			char folders1[1000];
			strcpy(folders1, store_homebrew_list[0].folders_no_del);
			char *split_tok1;
			split_tok1 = strtok (folders1,";");

			while (split_tok1 != NULL) {
				if (strcmp(split_tok1, split_tok) == 0) {
					ok_to_del = false;
				}
				split_tok1 = strtok (NULL, ";");
			}

			if (ok_to_del == true) {
				char temp_del[200] = "sd:";
				if (setting_use_sd == false) {
					strcpy(temp_del,"usb:");
				}
				strcat(temp_del, split_tok);

				delete_dir_files(temp_del);
			}


			split_tok = strtok (NULL, ";");*/

			/*if (!strstr(split_tok, "rom") && !strstr(split_tok, "save") && !strstr(split_tok, "ROM") && !strstr(split_tok, "SAVE") && !strstr(split_tok, "frodo/images")) {
				char temp_del[200] = "sd:";
				if (setting_use_sd == false) {
					strcpy(temp_del,"usb:");
				}
				strcat(temp_del, split_tok);

				delete_dir_files(temp_del);
			}
			split_tok = strtok (NULL, ";");
		}*/
	}

	// Delete the icon.png, meta.xml and boot.dol/elf
	if (delete_status == true) {
		char del_file[150] = "sd:/apps/";
		if (setting_use_sd == false) {
			strcpy(del_file,"usb:/apps/");
		}
		//strcat(del_file, store_homebrew_list[0].name);

		if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
			strcat(del_file, store_homebrew_list[0].user_dirname);
		}
		else {
			strcat(del_file, store_homebrew_list[0].name);
		}

		strcat(del_file, "/icon.png");

		if (remove_file(del_file) != 1) {
			delete_status = false;
			error_number = 6;
		}
	}
	if (delete_status == true) {
		char del_file[150] = "sd:/apps/";
		if (setting_use_sd == false) {
			strcpy(del_file,"usb:/apps/");
		}

		//strcat(del_file, store_homebrew_list[0].name);
		if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
			strcat(del_file, store_homebrew_list[0].user_dirname);
		}
		else {
			strcat(del_file, store_homebrew_list[0].name);
		}

		strcat(del_file, "/meta.xml");

		if (remove_file(del_file) != 1) {
			delete_status = false;
			error_number = 6;
		}
	}
	if (delete_status == true) {
		char del_file[150] = "sd:/apps/";
		if (setting_use_sd == false) {
			strcpy(del_file,"usb:/apps/");
		}
		//strcat(del_file, store_homebrew_list[0].name);
		if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
			strcat(del_file, store_homebrew_list[0].user_dirname);
		}
		else {
			strcat(del_file, store_homebrew_list[0].name);
		}

		strcat(del_file, "/boot.");
		strcat(del_file, store_homebrew_list[0].boot_ext);

		// hb sorter
		if (store_homebrew_list[0].boot_bak == true) {
			strcat(del_file, ".bak");
		}

		if (remove_file(del_file) != 1) {
			delete_status = false;
			error_number = 6;
		}

		// Theme.zip
		char del_file1[150] = "sd:/apps/";
		if (setting_use_sd == false) {
			strcpy(del_file1,"usb:/apps/");
		}
		//strcat(del_file1, store_homebrew_list[0].name);
		if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
			strcat(del_file1, store_homebrew_list[0].user_dirname);
		}
		else {
			strcat(del_file1, store_homebrew_list[0].name);
		}

		strcat(del_file1, "/theme.zip");

		if (remove_file(del_file) != 1) {
			delete_status = false;
			error_number = 6;
		}
	}

	usleep(100000);

	// Delete all created folders
	char directory[150][300];
	int remove_count = 0;

	if (store_homebrew_list[0].folders != NULL && delete_status == true && cancel_delete == false) {
		char folders[1000];
		strcpy(folders, store_homebrew_list[0].folders);

		char *split_tok;
		split_tok = strtok (folders,";");

		while (split_tok != NULL && delete_status == true) {

			strcpy(directory[remove_count],split_tok);
			remove_count++;
			split_tok = strtok (NULL, ";");
		}

		int x = 0;

		// Delete each folder, starting from the last to the first
		for (x = remove_count-1; x >= 0; x--) {

			ok_to_del = true;

			int d;
			for (d = 0; d < no_del_count; d++) {
				//printf("SECOND = %s\n", no_del_list[d].folders);
				if (strcmp(no_del_list[d], directory[x]) == 0) {
					ok_to_del = false;
				}
			}

			if (delete_status == true && ok_to_del == true) {
				char temp_del[200] = "sd:";
				if (setting_use_sd == false) {
					strcpy(temp_del,"usb:");
				}
				strcat(temp_del, directory[x]);

				if (remove_dir(temp_del) != 1) {
					// Shouldn't really matter
					//delete_status = false;
					//error_number = 7;
				}
			}
		}
	}

	// Delete all files from main folder
	if (delete_status == true) {
		char main_folder[300] = "/apps/";
		strcat(main_folder, store_homebrew_list[0].name);

		ok_to_del = true;

		int d;
		for (d = 0; d <= no_del_count; d++) {
			//printf("SECOND = %s\n", no_del_list[d].folders);
			if (strcmp(no_del_list[d], main_folder) == 0) {
				ok_to_del = false;
			}
		}

		if (ok_to_del == true) {
			char app_dir[100] = "sd:/apps/";
			if (setting_use_sd == false) {
				strcpy(app_dir,"usb:/apps/");
			}
			//strcat(app_dir, store_homebrew_list[0].name);
			if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
				strcat(app_dir, store_homebrew_list[0].user_dirname);
			}
			else {
				strcat(app_dir, store_homebrew_list[0].name);
			}

			delete_dir_files(app_dir);
		}
	}

	// Standard directory to delete
	if (delete_status == true) {
		char app_dir[100] = "sd:/apps/";
		if (setting_use_sd == false) {
			strcpy(app_dir,"usb:/apps/");
		}
		//strcat(app_dir, store_homebrew_list[0].name);
		if (strcmp(store_homebrew_list[0].name,"ftpii") == 0) {
			strcat(app_dir, store_homebrew_list[0].user_dirname);
		}
		else {
			strcat(app_dir, store_homebrew_list[0].name);
		}

		if (remove_dir(app_dir) != 1) {
			// Shouldn't really matter
			//delete_status = false;
			//error_number = 8;
		}
	}

	// Failed or success?
	if (delete_status == false) {
		delete_in_progress = -1;
		cancel_delete = false;
	}
	else {
		store_homebrew_list[0].local_app_size = 0;
		store_homebrew_list[0].in_download_queue = false;
		delete_in_progress = false;
		sd_card_update = true;
		cancel_delete = false;

		if (updating >= 0 && updating < array_length (homebrew_list)) {
			new_updating++;
		}
	}

	// Update the homebrew list
	store_update_lists();

	return 0;
}

u8 initialise_delete() {
	s32 result = LWP_CreateThread(&delete_thread, run_delete_thread, NULL, NULL, 0, 80);
	return result;
}

static void *run_rating_thread(void *arg) {
	CURLU* url = create_repo_url("/hbb/get_rating.php");
	curl_set_query(url, "esid", esid);
	curl_set_query(url, "name", homebrew_list[selected_app].name);

	// TODO: In the future, we should probably care if this fails.
	// However, the original code did not, returning 0 regardless.
	char* rating_response = (char *)handle_get_request(url , NULL);
	if (rating_response == NULL) {
		return 0;
	}

	if ((strcmp(rating_response, "1") == 0) || (strcmp(rating_response, "2") == 0) || (strcmp(rating_response, "3") == 0) || (strcmp(rating_response, "4") == 0) || (strcmp(rating_response, "5") == 0)) {
		strcpy(homebrew_list[selected_app].user_rating, rating_response);
	} else {
		strcpy(homebrew_list[selected_app].user_rating, "-1");
	}

	free(rating_response);
	get_rating_in_progress = false;

	return 0;
}

u8 initialise_rating() {
	s32 result = LWP_CreateThread(&rating_thread, run_rating_thread, NULL, NULL, 0, 80);
	return result;
}


static void *run_update_rating_thread(void *arg) {
	// Formulate request URL
	CURLU* url = create_repo_url("/hbb/update_rating.php");
	curl_set_query(url, "esid", esid);
	curl_set_query(url, "name", homebrew_list[selected_app].name);
	curl_set_query(url, "rate", rating_number);

	// TODO: In the future, we should probably care if this fails.
	// However, the original code did not, returning 0 regardless.
	char* rating_response = (char *)handle_get_request(url , NULL);
	if (rating_response == NULL) {
		return 0;
	}

	if ((strcmp(rating_response, "1") == 0) || (strcmp(rating_response, "2") == 0) || (strcmp(rating_response, "3") == 0) || (strcmp(rating_response, "4") == 0) || (strcmp(rating_response, "5") == 0)) {
		strcpy(homebrew_list[selected_app].user_rating, rating_response);
	} else {
		strcpy(homebrew_list[selected_app].user_rating, "-1");
	}

	free(rating_response);

	return 0;
}

u8 initialise_update_rating() {
	s32 result = LWP_CreateThread(&update_rating_thread, run_update_rating_thread, NULL, NULL, 0, 80);
	return result;
}

static void *run_request_thread(void *arg) {
	if (setting_online == true) {
		if (download_file("homebrew_browser", "listv036.txt") == 1) {
			printf("Homebrew List received.\n");
			list_received = true;
		} else {
			if (request_list_file("external_repo_list.txt", repo_list[setting_repo].list_file) == 1) {
				printf("Homebrew List received.\n");
				list_received = true;
			}
		}
	} else if (setting_repo == 0) {
		printf("Using Homebrew List on file.\n");
		list_received = true;
	} else {
		printf("Can't use Homebrew List on file.\n");
	}

	return 0;
}

u8 initialise_request() {
	s32 result = LWP_CreateThread(&request_thread, run_request_thread, NULL, NULL, 0, 80);
	return result;
}


// Suspend reset thread
void suspend_reset_thread() {
	LWP_SuspendThread(reset_thread);
}


// Return the array length by counting the characters in the name
int array_length (struct homebrew_struct array[400]) {
	int x = 0;
	while (strlen(array[x].name) >= 2) {
		x++;
	}
	return x;
}

int sort_array_length (struct sort_homebrew_struct array[400]) {
	int x = 0;
	while (strlen(array[x].name) >= 2) {
		x++;
	}
	return x;
}

// Save no manage list
void save_no_manage_list() {

	// Read all homebrew array for no managed
	char no_manage_file[50];
	strcpy(no_manage_file, rootdir);
	strcat(no_manage_file, "/apps/homebrew_browser/unmanaged_apps.txt");

	FILE *fu = fopen(no_manage_file, "wb");

	if (fu != NULL) {
		int x = 0;
		for (x = 0; x < total_list_count; x++) {
			if (total_list[x].local_app_size > 0 && total_list[x].no_manage == true) {
				char testname[100];
				strcpy(testname, total_list[x].name);
				int leng=strlen(testname);
				int z;
				for(z=0; z<leng; z++)
					if (97<=testname[z] && testname[z]<=122)//a-z
						testname[z]-=32;
				fputs (testname, fu);
				fputs ("\r\n", fu);
			}
		}

		fclose(fu);
	}
}

// Load no manage list to array
void load_no_manage_list() {

	char current_line[100];
	char no_manage_file[50];
	strcpy(no_manage_file, rootdir);
	strcat(no_manage_file, "/apps/homebrew_browser/unmanaged_apps.txt");
	FILE *fl = fopen(no_manage_file, "rb");

	if (fl != NULL) {
		while (fgets (current_line, 100, fl)) {
			//printf("%s\n",current_line);
			current_line[(strlen(current_line) - 2)] = '\0';
			strcpy(no_manage_list[no_manage_count].name, current_line);
			no_manage_count++;
		}
	}

	fclose(fl);
}

// Add text to log
void add_to_log(char* text, ...) {

	va_list args;
	va_start (args, text);

	char log_file[60];
	strcpy(log_file, rootdir);
	strcat(log_file, "/apps/homebrew_browser/log.txt");
	FILE *flog = fopen(log_file, "a");
	vfprintf (flog, text, args);
	fputs ("\r\n", flog);
	fclose(flog);

	va_end (args);
}

// Save the meta.xml "name" element
void save_xml_name() {

	char savexml[150] = "sd:/apps/";
	if (setting_use_sd == false) {
		strcpy(savexml,"usb:/apps/");
	}
	if (strcmp(homebrew_list[selected_app].name,"ftpii") == 0) {
		strcat(savexml, homebrew_list[selected_app].user_dirname);
	}
	else {
		strcat(savexml, homebrew_list[selected_app].name);
	}
	strcat(savexml, "/meta.xml");

	FILE *fx = fopen(savexml, "rb");
	if (fx != NULL) {
		mxml_node_t *tree;
		tree = mxmlLoadFile(NULL, fx, MXML_OPAQUE_CALLBACK);
		fclose(fx);

		mxml_node_t *thename = mxmlFindElement(tree, tree, "name", NULL, NULL, MXML_DESCEND);
		if (thename == NULL) { strcpy(temp_name, "0"); }
		else { strcpy(temp_name, thename->child->value.opaque); }

		mxmlDelete(tree);
	}

	fclose(fx);
}

void copy_xml_name() {

	if (strcmp(temp_name,"0") != 0) {

		char savexml[150] = "sd:/apps/";
		if (setting_use_sd == false) {
			strcpy(savexml,"usb:/apps/");
		}
		if (strcmp(homebrew_list[selected_app].name,"ftpii") == 0) {
			strcat(savexml, homebrew_list[selected_app].user_dirname);
		}
		else {
			strcat(savexml, homebrew_list[selected_app].name);
		}
		strcat(savexml, "/meta.xml");

		FILE *fx = fopen(savexml, "rb");
		if (fx != NULL) {
			mxml_node_t *tree1;
			tree1 = mxmlLoadFile(NULL, fx, MXML_OPAQUE_CALLBACK);
			fclose(fx);

			mxml_node_t *thename1 = mxmlFindElement(tree1, tree1, "name", NULL, NULL, MXML_DESCEND);
			mxmlSetOpaque(thename1->child, temp_name);

			fx = fopen(savexml, "wb");
			if (fx != NULL) {
				mxmlSaveFile(tree1, fx, 0);
			}
			fclose(fx);

			mxmlDelete(tree1);
		}
	}
}

void sort_by_downloads (bool min_to_max) {

	clear_temp_list();

	int i;
	for (i = 0; i < array_length (homebrew_list); i++) {
		strcpy(temp_list[i].name, homebrew_list[i].name);
		temp_list[i].app_downloads = atoi(homebrew_list[i].app_downloads);
		//printf("%s\n",homebrew_list[i].app_downloads);
	}

	int now;
	int next;
	int x;
	int final = 0;

	while (final != (sort_array_length(temp_list) - 1)) {
		final = 0;
		for (x = 0; x < sort_array_length(temp_list) - 1; x++) {

			now = temp_list[x].app_downloads;
			next = temp_list[x+1].app_downloads;

			if (min_to_max == true) {
				if (next < now) {
					temp1_list[0] = temp_list[x+1];
					temp_list[x+1] = temp_list[x];
					temp_list[x] = temp1_list[0];
				}
				else {
					final++;
					//printf("Test = %i   %i    %i\n",final, now, next);
				}
			}
			else {
				if (now < next) {
					temp1_list[0] = temp_list[x+1];
					temp_list[x+1] = temp_list[x];
					temp_list[x] = temp1_list[0];
				}
				else {
					final++;
					//printf("Test = %i   %i    %i\n",final, now, next);
				}
			}
		}
	}

	for (x = 0; x < array_length (homebrew_list); x++) {
		int y;
		for (y = 0; y < array_length (homebrew_list); y++) {
			if (strcmp (homebrew_list[x].name, temp_list[y].name) == 0) {
				temp_list2[y] = homebrew_list[x];
			}
		}
	}

	clear_list();

	for (i = 0; i < 4; i++) {
		strcpy(homebrew_list[i].name,"000");
	}

	x = 0;
	for (i = 0; i < array_length (temp_list2); i++) {
		if (strcmp(temp_list2[i].name,"000") != 0) {
			homebrew_list[x] = temp_list2[i];
			x++;
		}
		//printf("%s\n",temp_list[i].app_downloads);
	}
}


void sort_by_rating (bool min_to_max) {

	clear_temp_list();

	int i;
	for (i = 0; i < array_length (homebrew_list); i++) {
		strcpy(temp_list[i].name, homebrew_list[i].name);
		temp_list[i].app_downloads = homebrew_list[i].app_rating;
		//printf("%s\n",homebrew_list[i].app_downloads);
	}

	int now;
	int next;
	int x;
	int final = 0;

	while (final != (sort_array_length(temp_list) - 1)) {
		final = 0;
		for (x = 0; x < sort_array_length(temp_list) - 1; x++) {

			now = temp_list[x].app_downloads;
			next = temp_list[x+1].app_downloads;

			if (min_to_max == true) {
				if (next < now) {
					temp1_list[0] = temp_list[x+1];
					temp_list[x+1] = temp_list[x];
					temp_list[x] = temp1_list[0];
				}
				else {
					final++;
					//printf("Test = %i   %i    %i\n",final, now, next);
				}
			}
			else {
				if (now < next) {
					temp1_list[0] = temp_list[x+1];
					temp_list[x+1] = temp_list[x];
					temp_list[x] = temp1_list[0];
				}
				else {
					final++;
					//printf("Test = %i   %i    %i\n",final, now, next);
				}
			}
		}
	}

	for (x = 0; x < array_length (homebrew_list); x++) {
		int y;
		for (y = 0; y < array_length (homebrew_list); y++) {
			if (strcmp (homebrew_list[x].name, temp_list[y].name) == 0) {
				temp_list2[y] = homebrew_list[x];
			}
		}
	}

	clear_list();

	for (i = 0; i < 4; i++) {
		strcpy(homebrew_list[i].name,"000");
	}

	x = 0;
	for (i = 0; i < array_length (temp_list2); i++) {
		if (strcmp(temp_list2[i].name,"000") != 0) {
			homebrew_list[x] = temp_list2[i];
			x++;
		}
		//printf("%s\n",temp_list[i].app_downloads);
	}
}

void sort_by_name (bool min_to_max) {

	clear_temp_list();

	int i;
	for (i = 0; i < array_length (homebrew_list); i++) {
		strcpy(temp_list[i].name, homebrew_list[i].name);
		strcpy(temp_list[i].app_name, homebrew_list[i].app_name);

		int leng=strlen(temp_list[i].app_name);
		int z;
		for(z=0; z<leng; z++)
			if (97<=temp_list[i].app_name[z] && temp_list[i].app_name[z]<=122)//a-z
				temp_list[i].app_name[z]-=32;
		//printf("%s\n",homebrew_list[i].app_downloads);
	}


	int now;
//	int next;
	int x;
	int final = 0;

	while (final != (sort_array_length(temp_list) - 1)) {
		final = 0;
		for (x = 0; x < sort_array_length(temp_list) - 1; x++) {

			//now = temp_list[x].app_name;
			//next = temp_list[x+1].app_name;
			now = strcmp(temp_list[x].app_name,temp_list[x+1].app_name);

			if (min_to_max == true) {
				if (now < 0) {
					temp1_list[0] = temp_list[x+1];
					temp_list[x+1] = temp_list[x];
					temp_list[x] = temp1_list[0];
				}
				else {
					final++;
					//printf("Test = %s & %s =  %i\n", temp_list[x].app_name, temp_list[x+1].app_name, now);
				}
			}
			else {
				if (now > 0) {
					//printf("Test = %s & %s =  %i\n", temp_list[x].app_name, temp_list[x+1].app_name, now);
					temp1_list[0] = temp_list[x+1];
					temp_list[x+1] = temp_list[x];
					temp_list[x] = temp1_list[0];
				}
				else {
					final++;
					//printf("Test = %s & %s =  %i\n", temp_list[x].app_name, temp_list[x+1].app_name, now);
				}
			}
			//sleep(1);
		}
	}

	/*for (x = 0; x < sort_array_length(temp_list) - 1; x++) {
		printf("Test = %s\n", temp_list[x].app_name);
		sleep(1);
	}*/


	for (x = 0; x < array_length (homebrew_list); x++) {
		int y;
		for (y = 0; y < array_length (homebrew_list); y++) {
			if (strcmp (homebrew_list[x].name, temp_list[y].name) == 0) {
				temp_list2[y] = homebrew_list[x];
			}
		}
	}

	clear_list();

	for (i = 0; i < 4; i++) {
		strcpy(homebrew_list[i].name,"000");
	}

	x = 0;
	for (i = 0; i < array_length (temp_list2); i++) {
		if (strcmp(temp_list2[i].name,"000") != 0) {
			homebrew_list[x] = temp_list2[i];
			x++;
		}
		//printf("%s\n",temp_list2[i].app_name);
		//sleep(1);
	}
}

void sort_by_date (bool min_to_max) {

	clear_temp_list();

	int i;
	for (i = 0; i < array_length (homebrew_list); i++) {
		strcpy(temp_list[i].name, homebrew_list[i].name);
		temp_list[i].app_downloads = homebrew_list[i].app_time;
		//printf("%s\n",homebrew_list[i].app_downloads);
	}

	int now;
	int next;
	int x;
	int final = 0;

	while (final != (sort_array_length(temp_list) - 1)) {
		final = 0;
		for (x = 0; x < sort_array_length(temp_list) - 1; x++) {

			now = temp_list[x].app_downloads;
			next = temp_list[x+1].app_downloads;

			if (min_to_max == true) {
				if (next < now) {
					temp1_list[0] = temp_list[x+1];
					temp_list[x+1] = temp_list[x];
					temp_list[x] = temp1_list[0];
				}
				else {
					final++;
					//printf("Test = %i   %i    %i\n",final, now, next);
				}
			}
			else {
				if (now < next) {
					temp1_list[0] = temp_list[x+1];
					temp_list[x+1] = temp_list[x];
					temp_list[x] = temp1_list[0];
				}
				else {
					final++;
					//printf("Test = %i   %i    %i\n",final, now, next);
				}
			}
		}
	}

	for (x = 0; x < array_length (homebrew_list); x++) {
		int y;
		for (y = 0; y < array_length (homebrew_list); y++) {
			if (strcmp (homebrew_list[x].name, temp_list[y].name) == 0) {
				temp_list2[y] = homebrew_list[x];
			}
		}
	}

	clear_list();

	for (i = 0; i < 4; i++) {
		strcpy(homebrew_list[i].name,"000");
	}

	x = 0;
	for (i = 0; i < array_length (temp_list2); i++) {
		if (strcmp(temp_list2[i].name,"000") != 0) {
			homebrew_list[x] = temp_list2[i];
			x++;
		}
		//printf("%s\n",temp_list[i].app_downloads);
	}
}

void update_lists() {
	int y;

	if (category_old_selection == 0) {
		for (y = 0; y < array_length (demos_list); y++) {
			int x;
			for (x = 0; x < array_length (homebrew_list); x++) {
				if (strcmp (homebrew_list[x].name, demos_list[y].name) == 0) {
					demos_list[y] = homebrew_list[x];
					break;
				}
			}
		}
	}

	if (category_old_selection == 1) {
		for (y = 0; y < array_length (emulators_list); y++) {
			int x;
			for (x = 0; x < array_length (homebrew_list); x++) {
				if (strcmp (homebrew_list[x].name, emulators_list[y].name) == 0) {
					emulators_list[y] = homebrew_list[x];
					break;
				}
			}
		}
	}

	if (category_old_selection == 2) {

		for (y = 0; y < array_length (games_list); y++) {
			int x;
			for (x = 0; x < array_length (homebrew_list); x++) {
				if (strcmp (homebrew_list[x].name, games_list[y].name) == 0) {
					games_list[y] = homebrew_list[x];
					break;
				}
			}
		}
	}

	if (category_old_selection == 3) {
		for (y = 0; y < array_length (media_list); y++) {
			int x;
				for (x = 0; x < array_length (homebrew_list); x++) {
				if (strcmp (homebrew_list[x].name, media_list[y].name) == 0) {
					media_list[y] = homebrew_list[x];
					break;
				}
			}
		}
	}

	if (category_old_selection == 4) {
		for (y = 0; y < array_length (utilities_list); y++) {
			int x;
				for (x = 0; x < array_length (homebrew_list); x++) {
				if (strcmp (homebrew_list[x].name, utilities_list[y].name) == 0) {
					utilities_list[y] = homebrew_list[x];
					break;
				}
			}
		}
	}

	int x = 0;
	for (x = 0; x < array_length (homebrew_list); x++) {
		int y;
		for (y = 0; y < array_length (total_list); y++) {
			if (strcmp (homebrew_list[x].name, total_list[y].name) == 0) {
				total_list[y] = homebrew_list[x];
			}
		}
	}

}

void store_update_lists() {
	int y = 0;
	for (y = 0; y < array_length (demos_list); y++) {
		if (strcmp (store_homebrew_list[0].name, demos_list[y].name) == 0) {
			demos_list[y] = store_homebrew_list[0];
		}
	}

	for (y = 0; y < array_length (emulators_list); y++) {
		if (strcmp (store_homebrew_list[0].name, emulators_list[y].name) == 0) {
			emulators_list[y] = store_homebrew_list[0];
		}
	}

	for (y = 0; y < array_length (games_list); y++) {
		if (strcmp (store_homebrew_list[0].name, games_list[y].name) == 0) {
			games_list[y] = store_homebrew_list[0];
		}
	}

	for (y = 0; y < array_length (media_list); y++) {
		if (strcmp (store_homebrew_list[0].name, media_list[y].name) == 0) {
			media_list[y] = store_homebrew_list[0];
		}
	}

	for (y = 0; y < array_length (utilities_list); y++) {
		if (strcmp (store_homebrew_list[0].name, utilities_list[y].name) == 0) {
			utilities_list[y] = store_homebrew_list[0];
		}
	}

	for (y = 0; y < array_length (total_list); y++) {
		if (strcmp (store_homebrew_list[0].name, total_list[y].name) == 0) {
			total_list[y] = store_homebrew_list[0];
		}
	}

	for (y = 0; y < array_length (total_list); y++) {
		if (strcmp (store_homebrew_list[0].name, homebrew_list[y].name) == 0) {
			homebrew_list[y] = store_homebrew_list[0];
		}
	}
}


void hide_apps_installed() {

	clear_temp_list();

	int x;
	for (x = 0; x < array_length (homebrew_list); x++) {
		temp_list2[x] = homebrew_list[x];
	}

	clear_list();

	int i;
	int j = 0;
	for (i = 0; i < array_length (temp_list2); i++) {
		if (temp_list2[i].local_app_size == 0) {
			homebrew_list[j] = temp_list2[i];
			j++;
		}
	}
	if (j == 0) {
		strcpy(homebrew_list[0].name,"000");
	}
}

bool hide_apps_updated() {

	clear_temp_list();

	int x;
	for (x = 0; x < array_length (homebrew_list); x++) {
		temp_list2[x] = homebrew_list[x];
	}

	clear_list();

	int i;
	int j = 0;
	for (i = 0; i < array_length (temp_list2); i++) {
		if (temp_list2[i].local_app_size == temp_list2[i].app_size && temp_list2[i].in_download_queue >= 1) {
			homebrew_list[j] = temp_list2[i];
		}
		else if (temp_list2[i].local_app_size != temp_list2[i].app_size && temp_list2[i].in_download_queue == true) {
			homebrew_list[j] = temp_list2[i];
		}
		else {
			homebrew_list[j] = temp_list2[i];
			homebrew_list[j].in_download_queue = false;
		}
		j++;
	}

	update_lists();

	clear_list();

	j = 0;
	for (i = 0; i < array_length (temp_list2); i++) {
		if (temp_list2[i].local_app_size == temp_list2[i].app_size && temp_list2[i].in_download_queue >= 1) {
			homebrew_list[j] = temp_list2[i];
			j++;
		}
		else if (temp_list2[i].local_app_size != temp_list2[i].app_size && temp_list2[i].in_download_queue == true) {
			homebrew_list[j] = temp_list2[i];
			j++;
		}
	}

	if (j >= 1) {
		return 1;
	}

	return 0;
}

void die(char *msg) {
	printf(msg);
	sleep(5);
	fatUnmount("sd:");
	fatUnmount("usb:");
	exit(1);
}

void initialise() {
	VIDEO_Init();
	WPAD_Init();
	PAD_Init();
	WPAD_SetVRes(0, 640, 480);
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	vmode = VIDEO_GetPreferredMode(NULL);
	xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
	xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
	console_init(xfb[0],20,20,vmode->fbWidth,vmode->xfbHeight,vmode->fbWidth*VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(vmode);
	VIDEO_SetNextFramebuffer(xfb[0]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(vmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	MP3Player_Init();
	// Initialise the audio
	ASND_Init();
	//AESND_Init(NULL);
	//MODPlay_Init(&play);
	xfb_height = vmode->xfbHeight;
	//add_to_log("Initialise complete.");
}

bool initialize_networking() {
	// Prepare libcurl's internals.
	curl_global_init(CURL_GLOBAL_ALL);

	// Attempt three times to initialize libwiisocket.
	int times = 0;
	int result = 0;
	while (times < 3) {
		result = wiisocket_init();
		if (result == 0) {
			// Success!
			break;
		}
		printf("Unable to initialize network, retrying...\n");
	}

	if (result != 0) {
		printf("Failed to initialize network. (wiisocket init error %d)\n", result);
	}

	// Ensure networking is available.
	return ensure_wifi();
}

bool ensure_wifi() {
	u32 ip = 0;
	int attempts = 0;

	if (attempts < 3) {
		do {
			ip = gethostid();
			if (!ip) printf("Unable to obtain IP, retrying...\n");
			attempts++;
		} while (!ip && attempts < 3);
	}

	if (ip) {
		return true;
	} else {
		printf("Unable to obtain this console's IP address.\n");
		return false;
	}
}

int load_file_to_memory(const char *filename, unsigned char **result) {
	int size = 0;
	FILE *f = fopen(filename, "rb");
	if (f == NULL) {
		*result = NULL;
		return -1; // -1 means file opening fail
	}
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	*result = (unsigned char *)malloc(size+1);
	if (size != fread(*result, sizeof(char), size, f)) {
		free(*result);
		return -1; // -2 means file reading fail
	}
	fclose(f);
	(*result)[size] = 0;
	return 1;
}

// Clear list
void clear_list() {
	int c;
	for (c = 0; c < 400; c++) {
		homebrew_list[c].name[0] = 0;
		homebrew_list[c].app_size = 0;
		homebrew_list[c].app_time = 0;
		homebrew_list[c].img_size = 0;
		homebrew_list[c].local_app_size = 0;
		homebrew_list[c].total_app_size = 0;
		homebrew_list[c].in_download_queue = 0;
		homebrew_list[c].user_dirname[0] = 0;
		homebrew_list[c].folders[0] = 0;
		homebrew_list[c].boot_ext[0] = 0;
		homebrew_list[c].boot_bak = 0;
		homebrew_list[c].no_manage = 0;

		homebrew_list[c].about_loaded = 0;
		homebrew_list[c].app_name[0] = 0;
		homebrew_list[c].app_short_description[0] = 0;
		homebrew_list[c].app_description[0] = 0;
		homebrew_list[c].app_author[0] = 0;
		homebrew_list[c].app_version[0] = 0;
		homebrew_list[c].app_total_size = 0;
		homebrew_list[c].app_downloads[0] = 0;
		homebrew_list[c].app_controllers[0] = 0;
		homebrew_list[c].app_rating = 0;
		homebrew_list[c].user_rating[0] = 0;

		//free(homebrew_list[c].str_name);
//		homebrew_list[c].str_name = NULL;
//		homebrew_list[c].str_short_description = NULL;
		homebrew_list[c].file_found = 0;
		homebrew_list[c].content = NULL;
	}
}

// Clear list
void clear_temp_list() {
	int c;
	for (c = 0; c < 400; c++) {
		temp_list2[c].name[0] = 0;
		temp_list2[c].app_size = 0;
		temp_list2[c].app_time = 0;
		temp_list2[c].img_size = 0;
		temp_list2[c].local_app_size = 0;
		temp_list2[c].total_app_size = 0;
		temp_list2[c].in_download_queue = 0;
		temp_list2[c].user_dirname[0] = 0;
		temp_list2[c].folders[0] = 0;
		temp_list2[c].boot_ext[0] = 0;
		temp_list2[c].boot_bak = 0;
		temp_list2[c].no_manage = 0;

		temp_list2[c].about_loaded = 0;
		temp_list2[c].app_name[0] = 0;
		temp_list2[c].app_short_description[0] = 0;
		temp_list2[c].app_description[0] = 0;
		temp_list2[c].app_author[0] = 0;
		temp_list2[c].app_version[0] = 0;
		temp_list2[c].app_total_size = 0;
		temp_list2[c].app_downloads[0] = 0;
		temp_list2[c].app_controllers[0] = 0;
		temp_list2[c].app_rating = 0;
		temp_list2[c].user_rating[0] = 0;

		//temp_list2[c].str_name = NULL;
		//temp_list2[c].str_short_description = NULL;
		temp_list2[c].file_found = 0;
		temp_list2[c].content = NULL;

		temp_list[c].name[0] = 0;
		temp_list[c].app_downloads = 0;
	}
}

// Clear list
void clear_store_list() {
	int c;
	for (c = 0; c < 1; c++) {
		store_homebrew_list[c].name[0] = 0;
		store_homebrew_list[c].app_size = 0;
		store_homebrew_list[c].app_time = 0;
		store_homebrew_list[c].img_size = 0;
		store_homebrew_list[c].local_app_size = 0;
		store_homebrew_list[c].total_app_size = 0;
		store_homebrew_list[c].in_download_queue = 0;
		store_homebrew_list[c].user_dirname[0] = 0;
		store_homebrew_list[c].folders[0] = 0;
		store_homebrew_list[c].boot_ext[0] = 0;
		store_homebrew_list[c].boot_bak = 0;
		store_homebrew_list[c].no_manage = 0;

		store_homebrew_list[c].about_loaded = 0;
		store_homebrew_list[c].app_name[0] = 0;
		store_homebrew_list[c].app_short_description[0] = 0;
		store_homebrew_list[c].app_description[0] = 0;
		store_homebrew_list[c].app_author[0] = 0;
		store_homebrew_list[c].app_version[0] = 0;
		store_homebrew_list[c].app_total_size = 0;
		store_homebrew_list[c].app_downloads[0] = 0;
		store_homebrew_list[c].app_controllers[0] = 0;
		store_homebrew_list[c].app_rating = 0;
		store_homebrew_list[c].user_rating[0] = 0;

//		store_homebrew_list[c].str_name = NULL;
//		store_homebrew_list[c].str_short_description = NULL;
		store_homebrew_list[c].file_found = 0;
		store_homebrew_list[c].content = NULL;
	}
}

// Unzip Archive
bool unzipArchive(char * zipfilepath, char * unzipfolderpath) {
	unzFile uf = unzOpen(zipfilepath);

	if (uf==NULL) {
		printf("Cannot open %s, aborting\n",zipfilepath);
		return false;
	}
	//printf("%s opened\n",zipfilepath);

	if (chdir(unzipfolderpath)) { // can't access dir
		makedir(unzipfolderpath); // attempt to make dir
		if (chdir(unzipfolderpath)) { // still can't access dir
			printf("Error changing into %s, aborting\n", unzipfolderpath);
			return false;
		}
	}

	extractZip(uf,0,1,0);

	unzCloseCurrentFile(uf);
	return true;
}


void download_queue_size() {

	updating_current_size = 0;
	updating_total_size = 0;

	int x;
	for (x = 0; x < array_length (homebrew_list); x++) {
		if (strlen(homebrew_list[x].name) >= 3 && strcmp(homebrew_list[x].name,"000") != 0 && homebrew_list[x].in_download_queue != 2) {
			updating_total_size += homebrew_list[x].total_app_size;
			printf("%i\n",updating_total_size);
		}
	}

	updating_part_size = updating_total_size / 100;
}


// Check if icon.png or meta.xml is missing and if so download them
void check_missing_files() {
	FILE* icon = hbb_fopen("icon.png", "rb");
	if (icon == NULL) {
		download_file("homebrew_browser", "icon.png");
	} else {
		fclose(icon);
	}

	FILE* meta = hbb_fopen("meta.xml", "rb");
	if (meta == NULL) {
		download_file("homebrew_browser", "meta.xml");
	} else {
		fclose(meta);
	}

	FILE* loop = hbb_fopen("loop.mod", "rb");
	if (loop == NULL) {
		download_file("homebrew_browser", "loop.mod");
	} else {
		fclose(loop);
	}
}


void check_temp_files() {
	int x = 0;


	char* temp_files_zip = app_path("homebrew_browser", "temp_files.zip");
	char* temp_files_dir = app_path("homebrew_browser", "temp");

	dir = opendir(temp_files_dir);

	if (dir != NULL) {
		char temp_path[MAXPATHLEN];
		while ((dent=readdir(dir)) != NULL) {
			strcpy(temp_path, temp_files_dir);
			strcat(temp_path, "/");
			strcat(temp_path, dent->d_name);
			stat(temp_path, &st);

			if(!(S_ISDIR(st.st_mode))) {
				x++;
				if (x > 200) {
					break;
				}
			}
		}
	}
	closedir(dir);

	if (x < 200) {
		printf("\n\nDownloading zip file containing current image files. \nYou can skip this by holding down the B button but it's highly recommended that you don't.\n");

		hbb_updating = true;
		remote_hb_size = 1874386;

		if (download_file("homebrew_browser", "temp_files.zip") != 1) {
			printf("\n\nFailed to download zip file.\n");
		} else {
			printf("\n\nExtracting image files...\n");
			if (unzipArchive(temp_files_zip, temp_files_dir) == true) {
				if (cancel_extract == false) {
					printf("\nDownloaded and Extracted images successfully.\n\n");
				}
				else {
					printf("\n");
				}
			}

		}

		remove_file(temp_files_zip);
		hbb_updating = false;

		if (cancel_download == true || cancel_extract == true) {
			printf("\nCancelled download and extracting of image files.");
			cancel_download = false;
			cancel_extract = false;
		}

		sleep(2);
	}

	free(temp_files_zip);
	free(temp_files_dir);
}

void add_to_stats() {
	// Make a request to note the download count.
	// TODO: we ignore the result - is this expected.
	CURLU* request = create_repo_url("/hbb_download.php");
	curl_set_query(request, "name", homebrew_list[selected_app].name);

	// Actually perform the request
	void* ignored = handle_get_request(request, NULL);
	
	// Free if possible
	if (ignored != NULL) {
		free(ignored);
	}
}


// Check for new applications
void apps_check() {
	long setting_last_boot_num = atoi(setting_last_boot);

	// If able to read settings
	if (setting_last_boot_num > 0 && setting_show_updated == true) {
		printf("Checking for new/updated applications... ");

		// Request this time from server
		CURLU* request = create_repo_url("/hbb/apps_check_new.php");
		curl_set_query(request, "t", setting_last_boot);

		// TODO: handle errors while checking for updates
		char* check_response = (char *)handle_get_request(request, NULL);
		if (check_response == NULL) {
			return;
		}

		long temp_time = 0;
		char app_text[100];
		char app_version_from[100];
		char app_version[100];
		char app_text_total[200];
		char timebuf[50];

		// Parse repository data
		char* pos = 0;
		int line_count = 0;
		bool first_time = false;

		while (pos != NULL) {
			// Get new line
			char* old_pos = pos;
			pos = strstr(pos, CRLF);
			if (pos == NULL) {
				// No more lines to read!
				break;
			}

			// Replace the \r in \r\n with a null terminator.
			pos[0] = '\0';
			char* current_line = old_pos;

			if (line_count == 0) {
				// Use server time
				temp_time = atoi(current_line);

				if (first_time == false) {
					if (temp_time > 120000) {
						char set1[14];
						sprintf(set1, "%li", temp_time + 1);
						strcpy(setting_last_boot, set1);
					}
					first_time = true;
				}

				app_time = temp_time;
				timeinfo = localtime(&app_time);
				strftime(timebuf, 50, "%d %b %Y", timeinfo);
			}

			// Text
			if (line_count == 1) {
				strcpy(app_text, current_line);
			}

			// Version from
			if (line_count == 2) {
				strcpy(app_version_from, current_line);
			}

			// Version to
			if (line_count == 3) {
				strcpy(app_version, current_line);
			}

			// New or updated?
			if (line_count == 4) {
				if (strcmp(current_line, "a") == 0) {
					strcpy(app_text_total, timebuf);
					strcat(app_text_total, " - ");
					strcat(app_text_total, app_text);
					strcat(app_text_total, " ");
					strcat(app_text_total, app_version);
					strcat(app_text_total, " added");
				} else {
					strcpy(app_text_total, timebuf);
					strcat(app_text_total, " - ");
					strcat(app_text_total, app_text);
					strcat(app_text_total, " ");
					strcat(app_text_total, app_version_from);
					strcat(app_text_total, " -> ");
					strcat(app_text_total, app_version);
				}

				//printf("App = %s\n",app_text_total);

				// Add text
				strcpy(updated_apps_list[updated_apps_count].text, app_text_total);
				updated_apps_count++;
				line_count = 0;
			}
				
			// Our new string position is past our CRLF.
			pos = pos + CRLF_LENGTH;
		}

		// Clean up after ourselves.
		free(check_response);

		// Ensure we read content.
		if (first_time == true) {
			printf("Checked.\n\n");
		}
	
		if (updated_apps_count >= 1) {
			show_updated_apps = true;
		}
	}
}

void repo_check() {
	// Setting in settings which will just be a number, 0 for HBB, 1 for another one, etc.
	// Always grab the listing of other repos...
	printf("Requesting repositories list... ");

	// We cannot use other repositories for this request,
	// as other repository URLs are loaded by this request.
	CURLU* url = create_hardcoded_url("/hbb/repo_list.txt");

	// Perform a request.
	CURLcode error;
	char* repo_response = (char *)handle_get_request(url, &error);
	if (repo_response == NULL) {
		// Check to see why.
		if (error == CURLE_OPERATION_TIMEDOUT && codemii_backup == false) {
			// Attempt our backup server on the next attempt, as we are not currently.
			codemii_backup = true;
		}

		printf("Failed to receive Repositories list: %s\n", curl_easy_strerror(error));
		return;
	}

	// Parse repository data
	char* pos = repo_response;
	int line_type = 0;
	bool did_reset_line = false;
	bool seen_version = false;

	while (pos != NULL) {
		// Get new line
		char* old_pos = pos;
		pos = strstr(pos, CRLF);
		if (pos == NULL) {
			// No more lines to read!
			break;
		}

		// Replace the \r in \r\n with a null terminator.
		pos[0] = '\0';
		char* current_line = old_pos;

		// Our first line is expected to be the literal "1".
		if (seen_version == false) {
			if (strcmp(current_line, "1") == 0) {
				seen_version = true;
			}	else {
				// Seems we could not find the version.
			}

			pos = pos + CRLF_LENGTH;
			continue;
		}

		// We're now ready to read our repo info.
		if (line_type == 0) {
			strcpy(repo_list[repo_count].name, current_line);
		}

		if (line_type == 1) {
			strcpy(repo_list[repo_count].domain, current_line);
		}

		if (line_type == 2) {
			strcpy(repo_list[repo_count].list_file, current_line);
		}

		if (line_type == 3) {
			strcpy(repo_list[repo_count].apps_dir, current_line);
			repo_count++;

			line_type = 0;
			did_reset_line = true;
		}

		// Our new string position is past our CRLF.
		pos = pos + CRLF_LENGTH;

		// Move on to the next line, if necessary.
		if (did_reset_line) {
			did_reset_line = false;
		} else {
			line_type += 1;
		}
	}

	// Clean up after ourselves.
	free(repo_response);

	if (repo_count >= 1) {
		printf("Repositories list received.\n");

		// Now check which server to use
		if (setting_repo != 0) {
			printf("Using repository: %s\n", repo_list[setting_repo].name);
			printf("Press Wiimote '2' or Gamecube 'X' button to revert to the CodeMii Repo\n\n");
		}
	} else if (repo_count == 0) {
		printf("Failed to parse Repositories list.\n");
	}
}

// Check for updates to the Homebrew Browser
void update_check() {

	printf("\n\nChecking for Homebrew Browser updates...\n");

	return;

	char* boot_dol_path = app_path("homebrew_browser", "boot.dol");
	char* boot_bak_path = app_path("homebrew_browser", "boot.bak");
	char* meta_xml_path = app_path("homebrew_browser", "meta.xml");
	char* meta_bak_path = app_path("homebrew_browser", "meta.bak");

	// Open the file
	FILE *f = hbb_fopen("boot.dol", "rb");

	// Problems opening the file?
	if (f == NULL) {
		printf("\nCould not open /apps/homebrew_browser/ for reading.\n");

		// Install via Wiiload
		printf("Would you like to install the Homebrew Browser? Press A to install or B to skip this installation.\n");

		bool running_update = true;
		while (running_update == true) {
			WPAD_ScanPads();
			u32 pressed = WPAD_ButtonsDown(0);

			// Grab the latest HBB
			if (pressed & WPAD_BUTTON_A) {
				hbb_updating = true;
				printf("Retrieving the latest version of the Homebrew Browser...\n");

				s32 hbb_update = download_file("homebrew_browser", "boot.dol");
				if (hbb_update == -1) {
					printf("\nCould not install the Homebrew Browser.\n\n");
					running_update = false;
				}
				else {
					download_file("homebrew_browser", "meta.xml");
					download_file("homebrew_browser", "icon.png");
					download_file("homebrew_browser", "loop.mod");

					printf("\nThe Homebrew Browser has been installed.\n\n");
					hbb_updating = false;
					running_update = false;
				}

			}
			else if (pressed & WPAD_BUTTON_B) {
				printf("Skipping the installation of the Homebrew Browser.\n\n");
				running_update = false;
			}
		}
	}
	else {
		// Open file
		fseek (f , 0, SEEK_END);
		long local_hb_size = ftell (f);
		rewind (f);
		fclose(f);

		// Different size?
		if (local_hb_size != remote_hb_size) {

			printf("\n");

			// Update text
			char *split_update_text = strtok (update_text, " ");

			while (split_update_text != NULL) {

				if (strcmp(split_update_text,"|") == 0) {
					printf("\n");
				}
				else {
					printf("%s ",split_update_text);
				}
				split_update_text = strtok (NULL, " ");
			}

			printf("\nNew update available! Press A to update or B to skip this update.\n");


			bool running_update = true;
			while (running_update == true) {
				WPAD_ScanPads();
				u32 pressed = WPAD_ButtonsDown(0);

				if (pressed & WPAD_BUTTON_B) {
					printf("Skipping update of the Homebrew Browser.\n\n");
					running_update = false;
					break;
				}

				if (!(pressed & WPAD_BUTTON_A)) {
					// We expect A to continue with our update logic.
					break;
				}

				// Grab the latest HBB
				hbb_updating = true;

				printf("Retrieving the latest version of the Homebrew Browser...\n");

				// Delete boot.bak if it exists
				remove_file(boot_bak_path);

				// Rename the old HBB boot file to boot.bak
				if (rename(boot_dol_path, boot_bak_path) == 0) {
					// Download the new dol file
					download_file("homebrew_browser", "boot.dol");

					// Check to see if size matches, if so remove boot.bak, if not report error
					f = hbb_fopen("boot.dol", "rb");

					if (f == NULL) {
						printf("\n\nCould not open the updated Homebrew Browser boot.dol file.\n");
						remove_file(boot_dol_path);
						rename(boot_bak_path, boot_dol_path);
						printf("The previous version of Homebrew Browser has been restored.\n\n");
						sleep(5);
						running_update = false;
					} else {
						// Open file
						fseek (f , 0, SEEK_END);
						long local_hb_size = ftell (f);
						rewind (f);
						fclose(f);

						if (local_hb_size == remote_hb_size) {
							strcpy(temp_name, "0");

							FILE *fx = fopen(meta_xml_path, "rb");
							if (fx != NULL) {
								mxml_node_t *tree;
								tree = mxmlLoadFile(NULL, fx, MXML_OPAQUE_CALLBACK);
								fclose(fx);

								mxml_node_t *thename = mxmlFindElement(tree, tree, "name", NULL, NULL, MXML_DESCEND);
								if (thename == NULL) {
									strcpy(temp_name, "Homebrew Browser");
								} else {
									strcpy(temp_name, thename->child->value.opaque);
								}
								mxmlDelete(tree);
							}

							rename(meta_xml_path, meta_bak_path);
							download_file("homebrew_browser", "meta.xml");

							fx = fopen(meta_xml_path, "rb");
							if (fx != NULL) {
								mxml_node_t *tree1;
								tree1 = mxmlLoadFile(NULL, fx, MXML_OPAQUE_CALLBACK);
								fclose(fx);

								mxml_node_t *thename1 = mxmlFindElement(tree1, tree1, "name", NULL, NULL, MXML_DESCEND);
								mxmlSetOpaque(thename1->child, temp_name);

								fx = fopen(meta_xml_path, "wb");
								if (fx != NULL) {
									mxmlSaveFile(tree1, fx, 0);
								}
								fclose(fx);

								mxmlDelete(tree1);
							}

							download_file("homebrew_browser", "icon.png");

							printf("\nLatest version of the Homebrew Browser has been installed. Now returning you to the HBC.\n");
							die(0);
						} else {
							printf("\n\nThe updated Homebrew Browser boot.dol file is a different size(%li) than expected (%li).\n", local_hb_size, remote_hb_size);
							remove_file(boot_dol_path);
							rename(boot_bak_path, boot_dol_path);
							remove_file(meta_xml_path);
							rename(meta_bak_path, meta_xml_path);
							printf("The previous version of Homebrew Browser has been restored.\n\n");
							sleep(5);
							running_update = false;
						}
					}
				} else {
					printf("Couldn't rename boot.dol to boot.bak, can't update to latest version.\n\n");
					running_update = false;
				}
			}
		}
		else {
			printf("No updates available.\n");
		}
	}

	free(boot_dol_path);
	free(boot_bak_path);
	free(meta_xml_path);
	free(meta_bak_path);
}

// Request the homebrew list
s32 request_list() {

	printf("Requesting Homebrew List... ");
	initialise_request();
	int retry = 0;
	timeout_counter = 0;
	while (list_received != true) {
		timeout_counter++;
		usleep(500000);
		if (timeout_counter > 240) {
			printf("Failed to receive the Homebrew List.\n");
			printf("Requesting Homebrew List...");
			retry++;
			timeout_counter = 0;
			LWP_SuspendThread(request_thread);
			initialise_request();
		}
		if (retry >= 4) {
			die("Couldn't receive Homebrew List... returning you to HBC\n\n");
		}
	}

	FILE *f = NULL;
	if (setting_repo == 0) {
		f = hbb_fopen("listv036.txt", "rb");
	}	else {
		f = hbb_fopen("external_repo_list.txt", "rb");
	}

	// If file doesn't exist or can't open it then we can grab the latest file
	if (f == NULL) {
		printf("Homebrew List file can't be found.\n");
		sleep(3);
		return -1;
	}
	// Open file and check file length for changes
	else {
		fseek (f , 0, SEEK_END);
		long file_check = ftell (f);
		rewind (f);

		if (file_check == 0) {
			if (setting_online == true) {
				printf("Homebrew List file is 0 bytes, retrying...\n");
				printf("If this keeps occuring, you might have to try reloading the Homebrew Browser.\n");
			}
			else {
				die("Homebrew List file is 0 bytes. As you are in offline mode, HBB will now exit.\n Please go online to retreive the list file.");
			}
			sleep(3);
			return -1;
		}

		char cmd_line [5000];
		int array_count = 0;
		int line_number = -1;
		bool add_to_list = true;
		int print_dot = 0;

		// Grab all directories and put them in a list
		int app_directories = 0;
		char apps_dir[80];
		strcpy(apps_dir, rootdir);
		strcat(apps_dir, "/apps");

		dir = opendir(apps_dir);

		if (dir != NULL) {
			// Grab directory listing
			char temp_path[MAXPATHLEN];
			while ((dent=readdir(dir)) != NULL) {
				strcpy(temp_path, apps_dir);
				strcat(temp_path, "/");
				strcat(temp_path, dent->d_name);
				stat(temp_path, &st);

				strcpy(foldername, dent->d_name);
				// st.st_mode & S_IFDIR indicates a directory
				// Don't add homebrew sorter to the list
				if (S_ISDIR(st.st_mode) && strcmp(foldername,".") != 0 && strcmp(foldername,"..") != 0) {

					// Add folder to folder exists list
					int leng=strlen(foldername);
					int z;
					for(z=0; z<leng; z++)
						if (97<=foldername[z] && foldername[z]<=122)//a-z
							foldername[z]-=32;

					strcpy(folders_list[app_directories].name, foldername);
					app_directories++;
				}
			}
		}

		printf("Parsing Homebrew List");

		while (fgets (cmd_line, 2000, f)) {

			//printf("%s\n\n",cmd_line);

			if (strstr(cmd_line, "Homebrew") && line_number == -1) {

				// Remote Homebrew Browser file size
				char *hb_size;
				hb_size = strtok (cmd_line, " "); // Removes the "Homebrew" part
				hb_size = strtok (NULL, " ");
				remote_hb_size = atoi(hb_size);

				// Update text
				char *split_tok = strtok (NULL, " ");

				int fd = 0;

				while (split_tok != NULL && (strcmp(split_tok,".") != 0)) {
					int ff;
					for (ff = 0; ff < strlen(split_tok); ff++) {
						update_text[fd] = split_tok[ff];
						fd++;
					}

					split_tok = strtok (NULL, " ");

					if (split_tok != NULL && (strcmp(split_tok,".") != 0)) {
						update_text[fd] = ' ';
						fd++;
					}
				}
				line_number = 0;

				//printf("%s\n\n",update_text);
			}
			else {

				if (add_to_list == false) {
					int c;
					for (c = 0; c < 400; c++) {
						homebrew_list[c].name[0] = 0;
						homebrew_list[c].app_size = 0;
						homebrew_list[c].app_time = 0;
						homebrew_list[c].img_size = 0;
						homebrew_list[c].local_app_size = 0;
						homebrew_list[c].total_app_size = 0;
						homebrew_list[c].in_download_queue = 0;
						homebrew_list[c].user_dirname[0] = 0;
						homebrew_list[c].folders[0] = 0;
						homebrew_list[c].boot_ext[0] = 0;
						homebrew_list[c].boot_bak = 0;
						homebrew_list[c].no_manage = 0;

						homebrew_list[c].about_loaded = 0;
						homebrew_list[c].app_name[0] = 0;
						homebrew_list[c].app_short_description[0] = 0;
						homebrew_list[c].app_description[0] = 0;
						homebrew_list[c].app_author[0] = 0;
						homebrew_list[c].app_version[0] = 0;
						homebrew_list[c].app_total_size = 0;
						homebrew_list[c].app_downloads[0] = 0;
						homebrew_list[c].app_controllers[0] = 0;

						homebrew_list[c].app_rating = 0;
						homebrew_list[c].user_rating[0] = 0;
					}
					add_to_list = true;
				}

				if (strstr(cmd_line, "=Games=") && line_number == 0) {
					int i;
					for (i = 0; i < array_count; i++) {
						games_list[i] = homebrew_list[i];
						total_list[total_list_count] = homebrew_list[i];
						total_list_count++;
					}
					//free(homebrew_list);
					array_count = 0;
					add_to_list = false;
				}
				else if (strstr(cmd_line, "=Emulators=") && line_number == 0) {
					int i;
					for (i = 0; i < array_count; i++) {
						emulators_list[i] = homebrew_list[i];
						total_list[total_list_count] = homebrew_list[i];
						total_list_count++;
					}
					//free(homebrew_list);
					array_count = 0;
					add_to_list = false;
				}
				else if (strstr(cmd_line, "=Media=") && line_number == 0) {
					int i;
					for (i = 0; i < array_count; i++) {
						media_list[i] = homebrew_list[i];
						total_list[total_list_count] = homebrew_list[i];
						total_list_count++;
					}
					//free(homebrew_list);
					array_count = 0;
					add_to_list = false;
				}
				else if (strstr(cmd_line, "=Utilities=") && line_number == 0) {
					int i;
					for (i = 0; i < array_count; i++) {
						utilities_list[i] = homebrew_list[i];
						total_list[total_list_count] = homebrew_list[i];
						total_list_count++;
					}
					//free(homebrew_list);
					array_count = 0;
					add_to_list = false;
				}
				else if (strstr(cmd_line, "=Demos=") && line_number == 0) {
					int i;
					for (i = 0; i < array_count; i++) {
						demos_list[i] = homebrew_list[i];
						total_list[total_list_count] = homebrew_list[i];
						total_list_count++;
					}
					//free(homebrew_list);
					array_count = 0;
					add_to_list = false;
				}

				if (add_to_list == true) {
					if (line_number == 0) {

						if (hbb_string_len == 0) {
							int z = 0;
							for (z = 0; z < strlen(cmd_line); z++) {
								if (cmd_line[z] == '\n' || cmd_line[z] == '\r') {
									hbb_string_len = strlen(cmd_line) - z;
									//printf("RESULTS = %i\n", hbb_string_len);
									break;
								}
							}

						}

						char *split_tok;

						// Name
						split_tok = strtok (cmd_line, " ");

						// ftpii_ thing
						homebrew_list[array_count].user_dirname[0] = 0;
						if (strcmp(split_tok,"ftpii") == 0) {
							strcpy(homebrew_list[array_count].user_dirname,"ftpii");

							char apps_dir[80];
							strcpy(apps_dir, rootdir);
							strcat(apps_dir, "/apps");

							dir = opendir(apps_dir);
							if (dir != NULL) {
								char temp_path[MAXPATHLEN];
								while ((dent=readdir(dir)) != NULL) {
									strcpy(temp_path, apps_dir);
									strcat(temp_path, "/");
									strcat(temp_path, dent->d_name);
									stat(temp_path, &st);

									strcpy(filename, dent->d_name);
									if (S_ISDIR(st.st_mode)) {
										// st.st_mode & S_IFDIR indicates a directory
										//printf ("%s: %s\n", (st.st_mode & S_IFDIR ? " DIR" : "FILE"), filename);

										char * pch;
										pch = strstr (filename,"ftpii");

										if (pch != NULL) {
											int x;
											for (x = 0; x < strlen(filename); x++) {
												homebrew_list[array_count].user_dirname[x] = filename[x];
											}
											homebrew_list[array_count].user_dirname[strlen(filename)] = '\0';

											break;
										}

									}
								}
							}
							closedir(dir);
						}

						strcpy(homebrew_list[array_count].name,split_tok);

						// App Time
						split_tok = strtok (NULL, " ");
						homebrew_list[array_count].app_time = atoi(split_tok);

						// Img size
						split_tok = strtok (NULL, " ");
						homebrew_list[array_count].img_size = atoi(split_tok);

						// Remote boot.dol/elf file size
						split_tok = strtok (NULL, " ");
						homebrew_list[array_count].app_size = atoi(split_tok);

						// File extension, either .dol or .elf
						split_tok = strtok (NULL, " ");
						strcpy(homebrew_list[array_count].boot_ext,split_tok);

						// Try to open the local boot.elf file if it exists
						homebrew_list[array_count].local_app_size = 0;

						char boot_path[100] = "sd:/apps/";
						if (setting_use_sd == false) {
							strcpy(boot_path,"usb:/apps/");
						}
						//strcat(boot_path, homebrew_list[array_count].name);
						if (strcmp(homebrew_list[array_count].name,"ftpii") == 0) {
							strcat(boot_path, homebrew_list[array_count].user_dirname);
						}
						else {
							strcat(boot_path, homebrew_list[array_count].name);
						}

						// New directory finding
						bool dir_exists = false;

						int g;
						for (g = 0; g < app_directories; g++) {
							strcpy(uppername, homebrew_list[array_count].name);

							int leng=strlen(uppername);
								int z;
								for(z=0; z<leng; z++)
									if (97<=uppername[z] && uppername[z]<=122)//a-z
										uppername[z]-=32;

							if (strcmp(folders_list[g].name, uppername) == 0) {
								dir_exists = true;
								break;
							}
						}

						/*

						if (opendir(boot_path) == NULL) {
							//printf("%s nothing here", homebrew_list[array_count].name);
						}
						else {
							//printf("%s exists", homebrew_list[array_count].name);
							dir_exists = true;
						}*/

						strcat(boot_path, "/boot.");
						strcat(boot_path, homebrew_list[array_count].boot_ext);

						if (dir_exists == true) {
							//printf("Test\n");

							FILE *f = fopen(boot_path, "rb");

							// Problems opening the file?
							if (f == NULL) {

								// Try opening the .elf file instead?
								char boot_path2[200] = "sd:/apps/";
								if (setting_use_sd == false) {
									strcpy(boot_path2,"usb:/apps/");
								}
								//strcat(boot_path2, homebrew_list[array_count].name);

								if (strcmp(homebrew_list[array_count].name,"ftpii") == 0) {
									strcat(boot_path2, homebrew_list[array_count].user_dirname);
								}
								else {
									strcat(boot_path2, homebrew_list[array_count].name);
								}

								strcat(boot_path2, "/boot.elf");

								FILE *f1 = fopen(boot_path2, "rb");

								if (f1 == NULL) {
									// Try opening the .dol.bak file instead?
									char boot_path2[200] = "sd:/apps/";
									if (setting_use_sd == false) {
										strcpy(boot_path2,"usb:/apps/");
									}
									//strcat(boot_path2, homebrew_list[array_count].name);

									if (strcmp(homebrew_list[array_count].name,"ftpii") == 0) {
										strcat(boot_path2, homebrew_list[array_count].user_dirname);
									}
									else {
										strcat(boot_path2, homebrew_list[array_count].name);
									}

									strcat(boot_path2, "/boot.dol.bak");

									FILE *f2 = fopen(boot_path2, "rb");

									if (f2 == NULL) {
										// Try opening the .elf file instead?
										char boot_path2[200] = "sd:/apps/";
										if (setting_use_sd == false) {
											strcpy(boot_path2,"usb:/apps/");
										}
										//strcat(boot_path2, homebrew_list[array_count].name);

										if (strcmp(homebrew_list[array_count].name,"ftpii") == 0) {
											strcat(boot_path2, homebrew_list[array_count].user_dirname);
										}
										else {
											strcat(boot_path2, homebrew_list[array_count].name);
										}

										strcat(boot_path2, "/boot.elf.bak");

										FILE *f3 = fopen(boot_path2, "rb");

										if (f3 == NULL) {
											// Try opening the .elf file instead?
											char boot_path3[200] = "sd:/apps/";
											if (setting_use_sd == false) {
												strcpy(boot_path3,"usb:/apps/");
											}
											//strcat(boot_path3, homebrew_list[array_count].name);

											if (strcmp(homebrew_list[array_count].name,"ftpii") == 0) {
												strcat(boot_path3, homebrew_list[array_count].user_dirname);
											}
											else {
												strcat(boot_path3, homebrew_list[array_count].name);
											}

											strcat(boot_path3, "/theme.zip");

											FILE *f4 = fopen(boot_path3, "rb");

											if (f4 == NULL) {
											}
											else {
											// Open file and get the file size
											fseek (f4 , 0, SEEK_END);
											homebrew_list[array_count].local_app_size = ftell (f4);
											rewind (f4);
											fclose(f4);
											}
										}
										else {
											// Open file and get the file size
											fseek (f3 , 0, SEEK_END);
											homebrew_list[array_count].local_app_size = ftell (f3);
											rewind (f3);
											fclose(f3);
											homebrew_list[array_count].boot_bak = true;
										}
									}
									else {
										// Open file and get the file size
										fseek (f2 , 0, SEEK_END);
										homebrew_list[array_count].local_app_size = ftell (f2);
										rewind (f2);
										fclose(f2);
										homebrew_list[array_count].boot_bak = true;
									}
								}
								else {
									// Open file and get the file size
									fseek (f1, 0, SEEK_END);
									homebrew_list[array_count].local_app_size = ftell (f1);
									rewind (f1);
									fclose(f);
								}
							}
							else {
								// Open file and get the file size
								fseek (f , 0, SEEK_END);
								homebrew_list[array_count].local_app_size = ftell (f);
								rewind (f);
								fclose(f);
							}
						}

						// Check if in no managed list
						/*if (homebrew_list[array_count].local_app_size > 0) {
							int d = 0;
							for (d = 0; d < no_manage_count; d++) {
								char testname[100];
								strcpy(testname, homebrew_list[array_count].name);
								int leng=strlen(testname);
								int z;
								for(z=0; z<leng; z++)
									if (97<=testname[z] && testname[z]<=122)//a-z
										testname[z]-=32;

								if (strcmp(no_manage_list[d].name, testname) == 0) {
									homebrew_list[array_count].no_manage = true;
								}
							}
						}*/

						// Total app size
						split_tok = strtok (NULL, " ");
						homebrew_list[array_count].total_app_size = atoi(split_tok);

						// Downloads
						split_tok = strtok (NULL, " ");
						strcpy(homebrew_list[array_count].app_downloads, split_tok);

						// Rating
						split_tok = strtok (NULL, " ");
						homebrew_list[array_count].app_rating = atoi(split_tok);

						// Controllers
						split_tok = strtok (NULL, " ");
						strcpy(homebrew_list[array_count].app_controllers, split_tok);

						// Folders to create (if any), a dot means no folders needed
						split_tok = strtok (NULL, " ");
						if (split_tok != NULL) {
							//strncpy(homebrew_list[array_count].folders, split_tok, strlen(split_tok) - hbb_string_len);
							//homebrew_list[array_count].folders[strlen(split_tok) - hbb_string_len] = '\0';
							strcpy(homebrew_list[array_count].folders, split_tok);
						}

						// Folders to not delete files from
						split_tok = strtok (NULL, " ");
						if (split_tok != NULL) {
							strcpy(homebrew_list[array_count].folders_no_del, split_tok);
						}

						// Files to not extract
						split_tok = strtok (NULL, " ");
						if (split_tok != NULL) {
							//strcpy(homebrew_list[array_count].files_no_extract, split_tok);
							strncpy(homebrew_list[array_count].files_no_extract, split_tok, strlen(split_tok) - hbb_string_len);
							homebrew_list[array_count].files_no_extract[strlen(split_tok) - hbb_string_len] = '\0';
						}

						//printf("%s, %li, %li, %li, %s, %li, %s, %s %s %s\n", homebrew_list[array_count].name, homebrew_list[array_count].app_time, homebrew_list[array_count].img_size, homebrew_list[array_count].app_size, homebrew_list[array_count].boot_ext , homebrew_list[array_count].total_app_size, homebrew_list[array_count].app_controllers, homebrew_list[array_count].folders, homebrew_list[array_count].folders_no_del, homebrew_list[array_count].files_no_extract);
						//usleep(10000);

						line_number++;
					}


					// App name
					else if (line_number == 1) {
						if (hbb_string_len == 0) {
							int z = 0;
							for (z = 0; z < strlen(cmd_line); z++) {
								if (cmd_line[z] == '\n' || cmd_line[z] == '\r') {
									hbb_string_len = strlen(cmd_line) - z;
									//printf("RESULTS1 = %i\n", hbb_string_len);
									break;
								}
							}

						}

						hbb_null_len = (strlen(cmd_line) - hbb_string_len + 1) > sizeof(homebrew_list[array_count].app_name) ? sizeof(homebrew_list[array_count].app_name) : strlen(cmd_line) - hbb_string_len + 1;

						strncpy(homebrew_list[array_count].app_name, cmd_line, hbb_null_len);
						homebrew_list[array_count].app_name[hbb_null_len - 1] = '\0';
						//printf("%s\n", homebrew_list[array_count].app_name);
						line_number++;
					}

					// Author
					else if (line_number == 2) {
						hbb_null_len = (strlen(cmd_line) - hbb_string_len + 1) > sizeof(homebrew_list[array_count].app_author) ? sizeof(homebrew_list[array_count].app_author) : strlen(cmd_line) - hbb_string_len + 1;

						strncpy(homebrew_list[array_count].app_author, cmd_line, hbb_null_len);
						homebrew_list[array_count].app_author[hbb_null_len - 1] = '\0';
						//printf("%s\n", homebrew_list[array_count].app_author);
						line_number++;
					}

					// Version
					else if (line_number == 3) {
						hbb_null_len = (strlen(cmd_line) - hbb_string_len + 1) > sizeof(homebrew_list[array_count].app_version) ? sizeof(homebrew_list[array_count].app_version) : strlen(cmd_line) - hbb_string_len + 1;

						strncpy(homebrew_list[array_count].app_version, cmd_line, hbb_null_len);
						homebrew_list[array_count].app_version[hbb_null_len - 1] = '\0';
						//printf("%s\n", homebrew_list[array_count].app_version);
						line_number++;
					}

					// Size
					else if (line_number == 4) {
						homebrew_list[array_count].app_total_size = atoi(cmd_line);
						//printf("%s\n", homebrew_list[array_count].app_total_size);
						line_number++;
					}

					// Short Description
					else if (line_number == 5) {
						hbb_null_len = (strlen(cmd_line) - hbb_string_len + 1) > sizeof(homebrew_list[array_count].app_short_description) ? sizeof(homebrew_list[array_count].app_short_description) : strlen(cmd_line) - hbb_string_len + 1;

						strncpy(homebrew_list[array_count].app_short_description, cmd_line, hbb_null_len);
						homebrew_list[array_count].app_short_description[hbb_null_len - 1] = '\0';
						//printf("%s\n", homebrew_list[array_count].app_short_description);
						line_number++;
					}

					// Description
					else if (line_number == 6) {
						hbb_null_len = (strlen(cmd_line) - hbb_string_len + 1) > sizeof(homebrew_list[array_count].app_description) ? sizeof(homebrew_list[array_count].app_description) : strlen(cmd_line) - hbb_string_len;

						strncpy(homebrew_list[array_count].app_description, cmd_line, hbb_null_len);
						homebrew_list[array_count].app_description[hbb_null_len - 1] = '\0';
						//printf("%s\n", homebrew_list[array_count].app_description);
						line_number = 0;
						array_count++;

						// Print ever 10 apps
						if (print_dot == 10) {
							printf(".");
							print_dot = 0;
						}
						print_dot++;
					}
				}
			}
		}
		fclose(f);
	}

	return 1;
}

bool download_progress_handler(curl_off_t current_size, curl_off_t total_size) {
	download_progress_counter = current_size;
	updating_current_size = current_size;

	// If updating the HBB, we need to display dots for our the progress bar.
	if (hbb_updating == true) {
		progress_size = (int) (remote_hb_size / 29);

		int progress_count = (int) download_progress_counter / progress_size;
		if (progress_count > progress_number) {
			printf(".");
			progress_number = progress_count;
		}
	}

	// For some conditions, we need to cancel this request.
	if (cancel_download == true && setting_prompt_cancel == false) {
		return false;
	}
	else if (cancel_download == true && setting_prompt_cancel == true && cancel_confirmed == true) {
		return false;
	}
	else if (hbb_updating == true && cancel_download == true) {
		return false;
	}
	
	// Success!
	return true;
}

// Request a file from the server and store it
s32 request_file(char* path, FILE *f) {
	CURLU* url = create_repo_url(path);

	// Download!
	CURLcode res = handle_download_request(url, f, download_progress_handler);
	if (res == CURLE_ABORTED_BY_CALLBACK) {
		// The user pressed cancel.
		return -2;
	} else if (res == CURLE_WRITE_ERROR) {
		// An error occurred while writing the file.
		return -1;
	}

	// Success!
	return 1;
}


s32 request_list_file(char *filename, char *url) {
	s32 result = 0;
	int retry_times = 0;

	FILE *f;

	// Try to get the file, if failed for a 3rd time, then return -1
	while (result != 1) {

		// Sleep for a little bit so we don't do all 3 requests at once
		if (retry_times > 1) {
			sleep(2);
		}
		else if (retry_times > 3) {
			sleep(3);
		}

		// Open file
		char* localized_path = app_path("homebrew_browser", filename);
		f = fopen(localized_path, "wb");
		free(localized_path);

		// If file can't be created
		if (f == NULL) {
			printf("There was a problem accessing the file %s.\n", filename);
			return -1;
		}

		result = request_file(url, f);

		retry_times++;

		fclose(f);

		// User cancelled download
		if (result == -2) {
			return -1;
		}

		if (retry_times >= 6) {
			return -1;
		}
	}

	return 1;
}

// Create the path to the folder if needed, create the file and request the file from the server
s32 create_and_request_file(char* path1, char* appname, char *filename) {

	// Path
	char path[300];
	strcpy(path, path1);
	strcat(path, appname);

	// Create the folder if it's a request on a directory
	if (strcmp(filename, ".png") != 0) {
		if (!opendir(path)) {
			mkdir(path, 0777);
			if (!opendir(path)) {
				printf("Could not create %s directory.\n", path);
				return -1;
			}
		}
	}

	strcat(path, filename);


	s32 result = 0;
	int retry_times = 0;

	FILE *f;

	// Try to get the file, if failed for a 3rd time, then return -1
	while (result != 1) {

		// Sleep for a little bit so we don't do all 3 requests at once
		if (retry_times > 1) {
			sleep(2);
		}
		else if (retry_times > 3) {
			sleep(3);
		}

		// Open file
		f = fopen(path, "wb");

		// If file can't be created
		if (f == NULL) {
			printf("There was a problem accessing the file %s.\n", path);
			return -1;
		}

		char* subdirectory = NULL;
		if (setting_repo == 0) {
			// With the default servers, we should assume /hbb/.
			subdirectory = "/hbb/";
		} else {
			subdirectory = repo_list[setting_repo].apps_dir;
		}

		// subdirectory + app name + null terminator
		int length = strlen(subdirectory) + strlen(appname) + strlen(filename) + 1;
		char* url_path = (char *)malloc(length);
		strlcpy(url_path, subdirectory, length);
		strlcat(url_path, appname, length);
		strlcat(url_path, filename, length);

		result = request_file(url_path, f);
		
		free(url_path);

		retry_times++;

		fclose(f);

		// User cancelled download
		if (result == -2) {
			return -1;
		}

		if (retry_times >= 6) {
			return -1;
		}
	}

	//printf("Received %s%s.\n", appname, filename);

	return 1;
}

// Downloads the given filename for this Homebrew application.
s32 download_file(char* appname, char *filename) {
	char* path = app_path(appname, filename);

	bool success = create_parent_dirs(path);
	if (success == false) {
		// Failed to create parent dirs
		printf("Could not create directories for %s.\n", path);
		free(path);
		return -1; 
	}

	s32 result = 0;
	int retry_times = 0;

	FILE *f;

	// Try to get the file, if failed for a 3rd time, then return -1
	while (result != 1) {
		// Sleep for a little bit so we don't do all 3 requests at once
		if (retry_times > 1) {
			sleep(2);
		}
		else if (retry_times > 3) {
			sleep(3);
		}

		// Open file
		f = fopen(path, "wb");

		// If file can't be created
		if (f == NULL) {
			printf("There was a problem accessing the file %s.\n", path);
			return -1;
		}

		char* subdirectory = NULL;
		if (setting_repo == 0) {
			// With the default servers, we should assume /hbb/.
			subdirectory = "/hbb/";
		} else {
			subdirectory = repo_list[setting_repo].apps_dir;
		}

		// subdirectory + app name + slash (1) + filename + null terminator (1)
		int length = strlen(subdirectory) + strlen(appname) + strlen(filename) + 2;
		char* url_path = (char *)malloc(length);
		strlcpy(url_path, subdirectory, length);
		strlcat(url_path, appname, length);
		strlcat(url_path, "/", length);
		strlcat(url_path, filename, length);

		result = request_file(url_path, f);
		
		free(url_path);

		retry_times++;

		fclose(f);

		// User cancelled download
		if (result == -2) {
			free(path);
			return -1;
		}

		if (retry_times >= 6) {
			free(path);
			return -1;
		}
	}

	free(path);
	return 1;
}
