/*

Homebrew Browser -- a simple way to view and install Homebrew releases via the Wii
Version: 0.3.9e

Author: teknecal
Created: 24/06/2008
Last Modified: 31/10/2010

Using some source from ftpii v0.0.5
ftpii Source Code Copyright (C) 2008 Joseph Jordan <joe.ftpii@psychlaw.com.au>

*/
#include <errno.h>
#include <malloc.h>
#include <math.h>
#include <ogcsys.h>
#include <gccore.h>
#include <ogc/pad.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <time.h>
#include <wiiuse/wpad.h>
#include <unistd.h>
#include <fat.h>
#include <zlib.h>
#include "unzip/unzip.h"
#include "unzip/miniunz.h"
#include "common.h"
#include "net_requests.h"
#include "GRRLIB/GRRLIB.h"
#include "GRRLIB_font1_png.h"
#include <dirent.h>
#include <sys/statvfs.h>

#include "mouse_png.h"
#include "about_blank_png.h"
#include "rate_star_png.h"
#include "download_button_png.h"
#include "download_button_highlight_png.h"
#include "delete_button_png.h"
#include "delete_button_highlight_png.h"
#include "control_wiimote_png.h"
#include "control_wiimote_2_png.h"
#include "control_wiimote_3_png.h"
#include "control_wiimote_4_png.h"
#include "control_nunchuck_png.h"
#include "control_classic_png.h"
#include "control_gcn_png.h"
#include "control_keyboard_png.h"
#include "control_zapper_png.h"
#include "apps_table_png.h"
#include "apps_table_blank_png.h"
#include "apps_bottom_png.h"
#include "apps_top_png.h"
#include "apps_top2_png.h"
#include "background_png.h"
#include "blue_light_png.h"
#include "blue_light_small_png.h"
#include "blue_dark_png.h"
#include "blue_dark_small_png.h"
#include "blue_dark_small2_png.h"
#include "no_image_png.h"
#include "date_png.h"
#include "date_highlight_png.h"
#include "star_png.h"
#include "star_highlight_png.h"
#include "thumbs_up_png.h"
#include "thumbs_up_highlight_png.h"
#include "app_plus_png.h"
#include "app_question_png.h"
#include "app_tick_png.h"
#include "app_tick_small_png.h"
#include "app_minus_png.h"
#include "app_new_png.h"
#include "help_highlight_png.h"
#include "sd_card_highlight_png.h"
#include "cat_all_png.h"
#include "cat_demo_png.h"
#include "cat_demo_highlight_png.h"
#include "cat_emulator_png.h"
#include "cat_emulator_highlight_png.h"
#include "cat_games_png.h"
#include "cat_games_highlight_png.h"
#include "cat_media_png.h"
#include "cat_media_highlight_png.h"
#include "cat_utilities_png.h"
#include "cat_utilities_highlight_png.h"
#include "down_png.h"
#include "down_highlight_png.h"
#include "down_clear_png.h"
#include "down_clear_highlight_png.h"
#include "blue_progress_png.h"
#include "download_frame_png.h"
#include "blank_png.h"
#include "sort_arrow_down_png.h"
#include "sort_arrow_up_png.h"
#include "update_png.h"
#include "update_highlight_png.h"
//#include "blade_png.h"
#include "help_about_png.h"
#include "help_about_2_png.h"
#include "help_apps_information_png.h"
#include "help_apps_information_2_png.h"
#include "help_apps_listing_png.h"
#include "help_controller_png.h"
#include "help_download_update_png.h"
#include "help_download_update_2_png.h"
#include "help_download_update_3_png.h"
#include "help_queue_png.h"
#include "help_queue_2_png.h"
#include "help_queue_3_png.h"
#include "help_ratings_png.h"
#include "help_sorting_png.h"
#include "help_sorting_2_png.h"
#include "help_apps_installed_png.h"
#include "help_settings_png.h"
#include "help_settings_2_png.h"
#include "help_settings_3_png.h"
#include "help_settings_4_png.h"
#include "help_settings_5_png.h"
#include "help_settings_6_png.h"
#include "help_settings_7_png.h"
#include "gear_bg_png.h"
#include "guy_menu_png.h"
#include "guy_settings_png.h"
#include "guy_help_png.h"
#include "menu_loader_png.h"
#include "menu_loader_highlight_png.h"
#include "menu_reboot_png.h"
#include "menu_reboot_highlight_png.h"
#include "menu_settings_png.h"
#include "menu_settings_highlight_png.h"
#include "setting_cross_png.h"
#include "tooltip_help_png.h"
#include "help_bg_png.h"
#include "tool_tip_installedapps_png.h"
#include "tool_tip_queue_png.h"
#include "tool_tip_sort_date_png.h"
#include "tool_tip_sort_name_png.h"
#include "tool_tip_sort_downloads_png.h"
#include "tool_tip_sort_rating_png.h"
#include "version_png.h"
#include "tool_tip_download_apps_png.h"
#include "tool_tip_update_apps_png.h"
#include "cancel_download_prompt_png.h"
#include "button_no_png.h"
#include "button_no_highlight_png.h"
#include "button_yes_png.h"
#include "button_yes_highlight_png.h"
#include "app_download_png.h"
#include "apps_updated_png.h"
#include "updated_close_png.h"
#include "updated_close_highlight_png.h"
#include "name_png.h"
#include "name_highlight_png.h"
#include "apps_repo_png.h"
#include "apps_start_cat_png.h"
#include "apps_start_sort_png.h"
#include "arrow_png.h"

#include "GRRLIB/GRRLIB.h"

#define METHOD_SD 1
#define METHOD_USB 2

extern Mtx GXmodelView2D;

// Wiimote IR
ir_t ir;
orient_t orient;

float ir_x = 0;
float ir_y = 0;
float ir_pitch = 0;
float ir_rotation = 0;

// Time info
long current_time;
extern struct tm * timeinfo;
extern time_t app_time;

int category_selection = 2;
bool update_about = false;
bool free_about = false;
bool free_update = false;
bool free_sd_size = false;
bool updated_cat = false;
int current_app = 0;

char text_white[10] = " ";
int display_message_counter = 0;
bool rating_in_progress = false;

int wait_a_press = 0;
int held_counter = 0;
bool in_help = false;
int help_section = 0;
int menu_section = 0;
bool free_icon_info = false;
bool gc_control_used = false;
int cancel_wait = 0;
bool load_updated = false;
int icons_loaded = 0;

bool free_download_info = false;
bool select_repo = false;
bool select_category = false;
bool select_sort = false;
bool repo_texted = false;

int refresh_list = -1;
int download_arrow = 0;

/*struct blade_struct {
	int x;
	int y;
	int speed;
	float size;
	int angle;
};

struct blade_struct blade_list[10];

float ir_y_temp = 0;
bool enable_blades = false;*/

s8 HWButton = -1;
void WiimotePowerPressed(s32 chan) {
	if (chan == 0 && setting_power_off == true) {
		HWButton = SYS_POWEROFF_STANDBY;
	}
}

void close_windows() {
	show_updated_apps = false;
	hbb_app_about = false;
	select_repo = false;
	select_category = false;
	select_sort = false;
	if (in_menu == true) {
		in_menu = false;
		update_settings();
	}
	in_help = false;
	menu_section = 0;
	refresh_list = -1;
	current_app = 0;
}

int main(int argc, char **argv) {
	//add_to_log("Booting Homebrew Browser v0.3.9");
	initialise();
	initialise_reset_button();
	WPAD_SetPowerButtonCallback(WiimotePowerPressed);

	current_time = time(0);
	sprintf(setting_last_boot, "%li", current_time); // bug fix

	printf("\x1b[2;0H");
	printf("Homebrew Browser v0.4.0 Beta\n");

	u32 temp_esid;
	ES_GetDeviceID(&temp_esid);
	sprintf(esid, "%d", temp_esid);
	//printf("ES_GetDeviceID = %s\n",esid);
	if (esid <= 0) {
		printf("ESID error - You won't be able to rate applications.\n");
	}

	initialise_fat();
	load_settings();

	if (setting_online == true) {
		bool could_initialize = initialize_networking();
		if (could_initialize == false) {
			die("\nReturning you back to HBC. Please verify your Wii is connected to the internet.\n");
		}

		// If the user has setting_server set, we should not attempt the main server.
		codemii_backup = setting_server;

		int attempt_count = 0;

		while (could_connect == false && attempt_count < 3) {
			// We'll HEAD the repository list to test connectivity.
			CURLU* test_url = create_hardcoded_url("/hbb/repo_list.txt");

			// Notify the user of what's going on.
			char* domain = NULL;
			curl_url_get(test_url, CURLUPART_HOST, &domain, 0);
			printf("Attempting to connect to %s...\n", domain);
			curl_free(domain);

			// Here we go...
			CURLcode res = test_head_request(test_url);
			if (res == CURLE_OK) {
				// Success!
				could_connect = true;
				break;
			}

			// Otherwise, inform the user why.
			const char* reason = curl_easy_strerror(res);
			printf("Failed to connect to server: %s\n"
				"Retrying...\n", reason);
			
			attempt_count += 1;

			// If we aren't already, use the backup server.
			if (attempt_count == 3 && codemii_backup == false) {
				printf("\nOSCWii appears to be having issues, using OSCWii Backup Server.\n\n");
				codemii_backup = true;
				attempt_count = 0;
			}
		}

		if (could_connect == false) {
			die("\nReturning you back to HBC. Please check to see if " MAIN_DOMAIN " and " FALLBACK_DOMAIN " are working.\n");
		}

		printf("Connection established\n");
		//while (check_server() != true); Removed server check, there was no real protection
		repo_check();
	}

	// Grab the homebrew list and parse list
	while (request_list() != true);

	// Clear homebrew list
	clear_list();

	// List
	int set_cat = setting_category; // So we don't mess with the user's settings
	while (array_length (homebrew_list) == 0) {
		if (set_cat == 0) {
			int i;
			for (i = 0; i < array_length (demos_list); i++) {
				homebrew_list[i] = demos_list[i];
			}
			category_selection = 0;
			category_old_selection = 0;
		}
		if (set_cat == 1) {
			int i;
			for (i = 0; i < array_length (emulators_list); i++) {
				homebrew_list[i] = emulators_list[i];
			}
			category_selection = 1;
			category_old_selection = 1;
		}
		if (set_cat == 2) {
			int i;
			for (i = 0; i < array_length (games_list); i++) {
				homebrew_list[i] = games_list[i];
			}
			category_selection = 2;
			category_old_selection = 2;
		}
		if (set_cat == 3) {
			int i;
			for (i = 0; i < array_length (media_list); i++) {
				homebrew_list[i] = media_list[i];
			}
			category_selection = 3;
			category_old_selection = 3;
		}
		if (set_cat == 4) {
			int i;
			for (i = 0; i < array_length (utilities_list); i++) {
				homebrew_list[i] = utilities_list[i];
			}
			category_selection = 4;
			category_old_selection = 4;
		}
		if (array_length (homebrew_list) == 0) {
			if (set_cat == 4) {
				set_cat = 0;
			}
			else {
				set_cat++;
			}
		}
	}

	// Sort
	if (setting_sort == 0) { sort_by_name(0); }
	if (setting_sort == 1) { sort_by_date(0); }
	if (setting_sort == 2) { sort_by_rating(0); }
	if (setting_sort == 3) { sort_by_downloads(0); }

	if (setting_hide_installed == true) {
		hide_apps_installed();
	}

	if (setting_online == true && setting_repo == 0) {
		check_missing_files();
		if (setting_update == true) {
			update_check();
		} else { printf("\n"); }
		check_temp_files();
		apps_check();
		update_settings(); // Update last boot time
	}

	GRRLIB_Init();
	//GRRLIB_InitVideo();
	GRRLIB_InitFreetype();

	if (setting_music == true) {
		play_mod_music();
	}

	suspend_reset_thread();

	// Constant Text
	GRRLIB_texImg *str_author = GRRLIB_TextToTexture("Author:", FONTSIZE_SMALLER, 0x575757);
	GRRLIB_texImg *str_version = GRRLIB_TextToTexture("Version:", FONTSIZE_SMALLER, 0x575757);
	GRRLIB_texImg *str_downloads= GRRLIB_TextToTexture("Downloads:", FONTSIZE_SMALLER, 0x575757);
	GRRLIB_texImg *str_size= GRRLIB_TextToTexture("Size:", FONTSIZE_SMALLER, 0x575757);
	GRRLIB_texImg *str_rating = GRRLIB_TextToTexture("Rating:", FONTSIZE_SMALLER, 0x575757);
	GRRLIB_texImg *str_date = GRRLIB_TextToTexture("Date:", FONTSIZE_SMALLER, 0x575757);
	GRRLIB_texImg *str_controllers = GRRLIB_TextToTexture("Controllers:", FONTSIZE_SMALLER, 0x575757);
	GRRLIB_texImg *str_your_rating = GRRLIB_TextToTexture("Your Rating:", FONTSIZE_SMALLER, 0x575757);

	GRRLIB_texImg *str_rate_app = GRRLIB_TextToTexture("Please select a rating:", FONTSIZE_SMALL, 0x575757);

	GRRLIB_texImg *str_sdhc = GRRLIB_TextToTexture("SDHC:", FONTSIZE_SMALLER, 0x575757);
	GRRLIB_texImg *str_sdhc_yes = GRRLIB_TextToTexture("Yes", FONTSIZE_SMALLER, COLOUR_BLUE);
	GRRLIB_texImg *str_sdhc_no = GRRLIB_TextToTexture("No", FONTSIZE_SMALLER, COLOUR_RED);


	// App listing text
	GRRLIB_texImg *str_name = NULL; GRRLIB_texImg *str_name1 = NULL; GRRLIB_texImg *str_name2 = NULL; GRRLIB_texImg *str_name3 = NULL; GRRLIB_texImg *str_name4 = NULL;
	GRRLIB_texImg *str_short_desc = NULL; GRRLIB_texImg *str_short_desc1 = NULL; GRRLIB_texImg *str_short_desc2 = NULL; GRRLIB_texImg *str_short_desc3 = NULL; GRRLIB_texImg *str_short_desc4 = NULL;
	//GRRLIB_texImg *str_list_date = NULL; GRRLIB_texImg *str_list_date1 = NULL; GRRLIB_texImg *str_list_date2 = NULL; GRRLIB_texImg *str_list_date3 = NULL; GRRLIB_texImg *str_list_date4 = NULL;


	// About text
	GRRLIB_texImg *str_res_title = NULL;
	GRRLIB_texImg *str_res_author = NULL;
	GRRLIB_texImg *str_res_version = NULL;
	GRRLIB_texImg *str_res_downloads = NULL;
	GRRLIB_texImg *str_res_size = NULL;
	GRRLIB_texImg *str_res_date = NULL;

	// About app icon
	GRRLIB_texImg *app_icon_img = NULL;

	// About text description
	GRRLIB_texImg *string1 = NULL;
	GRRLIB_texImg *string2 = NULL;
	GRRLIB_texImg *string3 = NULL;
	GRRLIB_texImg *string4 = NULL;
	GRRLIB_texImg *string5 = NULL;

	GRRLIB_texImg *str_downloading = GRRLIB_TextToTexture("Downloading ...", FONTSIZE_SMALL, 0x575757);
	GRRLIB_texImg *str_downloading_small = GRRLIB_TextToTexture("Downloading in progress", FONTSIZE_SMALLER, 0x9d9d9d);
	GRRLIB_texImg *str_extracting = GRRLIB_TextToTexture("Extracting ...", FONTSIZE_SMALL, 0x575757);
	GRRLIB_texImg *str_extracting_small = GRRLIB_TextToTexture("Extracting in progress", FONTSIZE_SMALLER, 0x9d9d9d);

	GRRLIB_texImg *str_deleting = GRRLIB_TextToTexture("Deleting ...", FONTSIZE_SMALL, 0x575757);
	GRRLIB_texImg *str_download_zip_failed = GRRLIB_TextToTexture("Downloading zip file failed.", FONTSIZE_SMALL, 0x575757);
	GRRLIB_texImg *str_create_folder_failed = GRRLIB_TextToTexture("Creating folders failed.", FONTSIZE_SMALL, 0x575757);
	GRRLIB_texImg *str_download_icon_failed = GRRLIB_TextToTexture("Downloading icon file failed.", FONTSIZE_SMALL, 0x575757);
	GRRLIB_texImg *str_extract_zip_failed = GRRLIB_TextToTexture("Extracting zip file failed.", FONTSIZE_SMALL, 0x575757);
	GRRLIB_texImg *str_boot_file_failed = GRRLIB_TextToTexture("Problem checking boot.dol/elf file.", FONTSIZE_SMALL, 0x575757);
	GRRLIB_texImg *str_delete_file_failed = GRRLIB_TextToTexture("Deleting boot/meta/icon files failed.", FONTSIZE_SMALL, 0x575757);
	GRRLIB_texImg *str_delete_folder_failed = GRRLIB_TextToTexture("Deleting folders failed.", FONTSIZE_SMALL, 0x575757);
	GRRLIB_texImg *str_delete_app_failed = GRRLIB_TextToTexture("Deleting application folder failed.", FONTSIZE_SMALL, 0x575757);
	GRRLIB_texImg *str_free_space_failed = GRRLIB_TextToTexture("Not enough free space.", FONTSIZE_SMALL, 0x575757);
	GRRLIB_texImg *str_no_wifi = GRRLIB_TextToTexture("Unable to initialise network.", FONTSIZE_SMALL, 0x575757);


	GRRLIB_texImg *str_sd_card = NULL;
	GRRLIB_texImg *str_icon_info = NULL;
	GRRLIB_texImg *str_download_info = NULL;

	// Download queue
	GRRLIB_texImg *str_title_status = NULL;
	GRRLIB_texImg *str_overall_progress = GRRLIB_TextToTexture("Overall Progress", FONTSIZE_SMALL, 0x575757);

	// Settings
	GRRLIB_texImg *str_setting_card_space = GRRLIB_TextToTexture("Show free space", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_free_space = GRRLIB_TextToTexture("Check free space", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_hide_app = GRRLIB_TextToTexture("Hide installed apps", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_rating = GRRLIB_TextToTexture("Retrieve my ratings", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_bgmusic = GRRLIB_TextToTexture("Background music", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_rumble = GRRLIB_TextToTexture("Rumble", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_icon = GRRLIB_TextToTexture("Update icons", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_offline = GRRLIB_TextToTexture("Offline mode", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_tool_tip = GRRLIB_TextToTexture("Tool tips", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_prompt = GRRLIB_TextToTexture("Confirm cancelling", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_power = GRRLIB_TextToTexture("Wiimote Power off", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_updated = GRRLIB_TextToTexture("Updated Apps Pop-up", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_sd = GRRLIB_TextToTexture("Use SD card", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_repo = GRRLIB_TextToTexture("Use Other Repositories", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_cat = GRRLIB_TextToTexture("Custom category", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_sort = GRRLIB_TextToTexture("Custom sort", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_disusb = GRRLIB_TextToTexture("Disable USB mount", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_dischar = GRRLIB_TextToTexture("Disable character", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_wiiside = GRRLIB_TextToTexture("Use Wiimote sideways", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_update = GRRLIB_TextToTexture("Check for updates", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_setting_server = GRRLIB_TextToTexture("Use secondary server", FONTSIZE_SMALL1, 0x575757);

	// Settings
	GRRLIB_texImg *str_help_about = GRRLIB_TextToTexture("About / Credits", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_help_applisting = GRRLIB_TextToTexture("    Apps listings  ", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_help_appinfo = GRRLIB_TextToTexture("Apps information", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_help_appview = GRRLIB_TextToTexture("Viewing apps installed", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_help_downapp = GRRLIB_TextToTexture("Download/Update/Delete apps", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_help_settings = GRRLIB_TextToTexture("Settings", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_help_controller = GRRLIB_TextToTexture("Controller", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_help_ratings = GRRLIB_TextToTexture("Ratings", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_help_sort = GRRLIB_TextToTexture("Sorting the list", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_help_queue = GRRLIB_TextToTexture("Download/Update queue", FONTSIZE_SMALL1, 0x575757);

	GRRLIB_texImg *str_page1o1 = GRRLIB_TextToTexture("Page 1 / 1", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page1o2 = GRRLIB_TextToTexture("Page 1 / 2", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page2o2 = GRRLIB_TextToTexture("Page 2 / 2", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page1o3 = GRRLIB_TextToTexture("Page 1 / 3", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page2o3 = GRRLIB_TextToTexture("Page 2 / 3", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page3o3 = GRRLIB_TextToTexture("Page 3 / 3", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page1o5 = GRRLIB_TextToTexture("Page 1 / 5", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page2o5 = GRRLIB_TextToTexture("Page 2 / 5", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page3o5 = GRRLIB_TextToTexture("Page 3 / 5", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page4o5 = GRRLIB_TextToTexture("Page 4 / 5", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page5o5 = GRRLIB_TextToTexture("Page 5 / 5", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page1o7 = GRRLIB_TextToTexture("Page 1 / 7", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page2o7 = GRRLIB_TextToTexture("Page 2 / 7", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page3o7 = GRRLIB_TextToTexture("Page 3 / 7", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page4o7 = GRRLIB_TextToTexture("Page 4 / 7", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page5o7 = GRRLIB_TextToTexture("Page 5 / 7", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page6o7 = GRRLIB_TextToTexture("Page 6 / 7", FONTSIZE_SMALLER, 0x666768);
	GRRLIB_texImg *str_page7o7 = GRRLIB_TextToTexture("Page 7 / 7", FONTSIZE_SMALLER, 0x666768);

	GRRLIB_texImg *str_empty_queue = GRRLIB_TextToTexture("The download queue is empty. To add an", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_empty_queue1 = GRRLIB_TextToTexture("application to the queue, find one from", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_empty_queue2 = GRRLIB_TextToTexture("one of the above categories, point at it,", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_empty_queue3 = GRRLIB_TextToTexture("and press the (+) button", FONTSIZE_SMALL1, 0x575757);

	GRRLIB_texImg *str_empty_sd = GRRLIB_TextToTexture("The Homebrew Browser could not find any", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_empty_sd1 = GRRLIB_TextToTexture("recognised homebrew applications on", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_empty_sd2 = GRRLIB_TextToTexture("your SD/USB device.", FONTSIZE_SMALL1, 0x575757);

	GRRLIB_texImg *str_cat_1 = GRRLIB_TextToTexture("Demos", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_cat_2 = GRRLIB_TextToTexture("Emulators", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_cat_3 = GRRLIB_TextToTexture("Games", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_cat_4 = GRRLIB_TextToTexture("Media", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_cat_5 = GRRLIB_TextToTexture("Utilities", FONTSIZE_SMALL1, 0x575757);

	GRRLIB_texImg *str_sort_1 = GRRLIB_TextToTexture("Name", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_sort_2 = GRRLIB_TextToTexture("Date", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_sort_3 = GRRLIB_TextToTexture("Rating", FONTSIZE_SMALL1, 0x575757);
	GRRLIB_texImg *str_sort_4 = GRRLIB_TextToTexture("Downloads", FONTSIZE_SMALL1, 0x575757);

	GRRLIB_texImg *mouse_img=GRRLIB_LoadTexture(mouse_png);

	GRRLIB_texImg *about_blank_img=GRRLIB_LoadTexture(about_blank_png);
	GRRLIB_texImg *rate_star_img=GRRLIB_LoadTexture(rate_star_png);
	GRRLIB_texImg *download_button_img=GRRLIB_LoadTexture(download_button_png);
	GRRLIB_texImg *download_button_highlight_img=GRRLIB_LoadTexture(download_button_highlight_png);
	GRRLIB_texImg *delete_button_img=GRRLIB_LoadTexture(delete_button_png);
	GRRLIB_texImg *delete_button_highlight_img=GRRLIB_LoadTexture(delete_button_highlight_png);
	GRRLIB_texImg *control_wiimote_img=GRRLIB_LoadTexture(control_wiimote_png);
	GRRLIB_texImg *control_wiimote_2_img=GRRLIB_LoadTexture(control_wiimote_2_png);
	GRRLIB_texImg *control_wiimote_3_img=GRRLIB_LoadTexture(control_wiimote_3_png);
	GRRLIB_texImg *control_wiimote_4_img=GRRLIB_LoadTexture(control_wiimote_4_png);
	GRRLIB_texImg *control_nunchuck_img=GRRLIB_LoadTexture(control_nunchuck_png);
	GRRLIB_texImg *control_classic_img=GRRLIB_LoadTexture(control_classic_png);
	GRRLIB_texImg *control_gcn_img=GRRLIB_LoadTexture(control_gcn_png);
	GRRLIB_texImg *control_keyboard_img=GRRLIB_LoadTexture(control_keyboard_png);
	GRRLIB_texImg *control_zapper_img=GRRLIB_LoadTexture(control_zapper_png);
	GRRLIB_texImg *apps_table_img=GRRLIB_LoadTexture(apps_table_png);
	GRRLIB_texImg *apps_table_blank_img=GRRLIB_LoadTexture(apps_table_blank_png);
	GRRLIB_texImg *apps_bottom_img=GRRLIB_LoadTexture(apps_bottom_png);
	GRRLIB_texImg *apps_top_img=GRRLIB_LoadTexture(apps_top_png);
	GRRLIB_texImg *apps_top2_img=GRRLIB_LoadTexture(apps_top2_png);
	GRRLIB_texImg *background_img=GRRLIB_LoadTexture(background_png);
	GRRLIB_texImg *blue_light_img=GRRLIB_LoadTexture(blue_light_png);
	GRRLIB_texImg *blue_light_small_img=GRRLIB_LoadTexture(blue_light_small_png);
	GRRLIB_texImg *blue_dark_img=GRRLIB_LoadTexture(blue_dark_png);
	GRRLIB_texImg *blue_dark_small_img=GRRLIB_LoadTexture(blue_dark_small_png);
	GRRLIB_texImg *blue_dark_small2_img=GRRLIB_LoadTexture(blue_dark_small2_png);
	GRRLIB_texImg *date_img=GRRLIB_LoadTexture(date_png);
	GRRLIB_texImg *date_highlight_img=GRRLIB_LoadTexture(date_highlight_png);
	GRRLIB_texImg *star_img=GRRLIB_LoadTexture(star_png);
	GRRLIB_texImg *star_highlight_img=GRRLIB_LoadTexture(star_highlight_png);
	GRRLIB_texImg *name_img=GRRLIB_LoadTexture(name_png);
	GRRLIB_texImg *name_highlight_img=GRRLIB_LoadTexture(name_highlight_png);
	GRRLIB_texImg *thumbs_up_img=GRRLIB_LoadTexture(thumbs_up_png);
	GRRLIB_texImg *thumbs_up_highlight_img=GRRLIB_LoadTexture(thumbs_up_highlight_png);
	GRRLIB_texImg *app_plus_img=GRRLIB_LoadTexture(app_plus_png);
	GRRLIB_texImg *app_minus_img=GRRLIB_LoadTexture(app_minus_png);
	GRRLIB_texImg *app_question_img=GRRLIB_LoadTexture(app_question_png);
	GRRLIB_texImg *app_tick_img=GRRLIB_LoadTexture(app_tick_png);
	GRRLIB_texImg *app_tick_small_img=GRRLIB_LoadTexture(app_tick_small_png);
	GRRLIB_texImg *app_new_img=GRRLIB_LoadTexture(app_new_png);
	GRRLIB_texImg *help_highlight_img=GRRLIB_LoadTexture(help_highlight_png);
	GRRLIB_texImg *sd_card_highlight_img=GRRLIB_LoadTexture(sd_card_highlight_png);
	GRRLIB_texImg *down_img=GRRLIB_LoadTexture(down_png);
	GRRLIB_texImg *down_highlight_img=GRRLIB_LoadTexture(down_highlight_png);
	GRRLIB_texImg *down_clear_img=GRRLIB_LoadTexture(down_clear_png);
	GRRLIB_texImg *down_clear_highlight_img=GRRLIB_LoadTexture(down_clear_highlight_png);
	GRRLIB_texImg *blue_progress_img=GRRLIB_LoadTexture(blue_progress_png);
	GRRLIB_texImg *download_frame_img=GRRLIB_LoadTexture(download_frame_png);
	GRRLIB_texImg *sort_arrow_down_img=GRRLIB_LoadTexture(sort_arrow_down_png);
	GRRLIB_texImg *sort_arrow_up_img=GRRLIB_LoadTexture(sort_arrow_up_png);
	GRRLIB_texImg *update_img=GRRLIB_LoadTexture(update_png);
	GRRLIB_texImg *update_highlight_img=GRRLIB_LoadTexture(update_highlight_png);

	GRRLIB_texImg *cat_all_img=GRRLIB_LoadTexture(cat_all_png);
	GRRLIB_texImg *cat_demo_img=GRRLIB_LoadTexture(cat_demo_png);
	GRRLIB_texImg *cat_demo_highlight_img=GRRLIB_LoadTexture(cat_demo_highlight_png);
	GRRLIB_texImg *cat_emulator_img=GRRLIB_LoadTexture(cat_emulator_png);
	GRRLIB_texImg *cat_emulator_highlight_img=GRRLIB_LoadTexture(cat_emulator_highlight_png);
	GRRLIB_texImg *cat_games_img=GRRLIB_LoadTexture(cat_games_png);
	GRRLIB_texImg *cat_games_highlight_img=GRRLIB_LoadTexture(cat_games_highlight_png);
	GRRLIB_texImg *cat_media_img=GRRLIB_LoadTexture(cat_media_png);
	GRRLIB_texImg *cat_media_highlight_img=GRRLIB_LoadTexture(cat_media_highlight_png);
	GRRLIB_texImg *cat_utilities_img=GRRLIB_LoadTexture(cat_utilities_png);
	GRRLIB_texImg *cat_utilities_highlight_img=GRRLIB_LoadTexture(cat_utilities_highlight_png);

//	GRRLIB_texImg *blade_img=GRRLIB_LoadTexture(blade_png);
	GRRLIB_texImg *help_about_img=GRRLIB_LoadTexture(help_about_png);
	GRRLIB_texImg *help_about_2_img=GRRLIB_LoadTexture(help_about_2_png);
	GRRLIB_texImg *help_apps_information_img=GRRLIB_LoadTexture(help_apps_information_png);
	GRRLIB_texImg *help_apps_information_2_img=GRRLIB_LoadTexture(help_apps_information_2_png);
	GRRLIB_texImg *help_apps_listing_img=GRRLIB_LoadTexture(help_apps_listing_png);
	GRRLIB_texImg *help_controller_img=GRRLIB_LoadTexture(help_controller_png);
	GRRLIB_texImg *help_download_update_img=GRRLIB_LoadTexture(help_download_update_png);
	GRRLIB_texImg *help_download_update_2_img=GRRLIB_LoadTexture(help_download_update_2_png);
	GRRLIB_texImg *help_download_update_3_img=GRRLIB_LoadTexture(help_download_update_3_png);
	GRRLIB_texImg *help_queue_img=GRRLIB_LoadTexture(help_queue_png);
	GRRLIB_texImg *help_queue_2_img=GRRLIB_LoadTexture(help_queue_2_png);
	GRRLIB_texImg *help_queue_3_img=GRRLIB_LoadTexture(help_queue_3_png);
	GRRLIB_texImg *help_ratings_img=GRRLIB_LoadTexture(help_ratings_png);
	GRRLIB_texImg *help_sorting_img=GRRLIB_LoadTexture(help_sorting_png);
	GRRLIB_texImg *help_sorting_2_img=GRRLIB_LoadTexture(help_sorting_2_png);
	GRRLIB_texImg *help_apps_installed_img=GRRLIB_LoadTexture(help_apps_installed_png);
	GRRLIB_texImg *help_settings_img=GRRLIB_LoadTexture(help_settings_png);
	GRRLIB_texImg *help_settings_2_img=GRRLIB_LoadTexture(help_settings_2_png);
	GRRLIB_texImg *help_settings_3_img=GRRLIB_LoadTexture(help_settings_3_png);
	GRRLIB_texImg *help_settings_4_img=GRRLIB_LoadTexture(help_settings_4_png);
	GRRLIB_texImg *help_settings_5_img=GRRLIB_LoadTexture(help_settings_5_png);
	GRRLIB_texImg *help_settings_6_img=GRRLIB_LoadTexture(help_settings_6_png);
	GRRLIB_texImg *help_settings_7_img=GRRLIB_LoadTexture(help_settings_7_png);

	GRRLIB_texImg *gear_bg_img=GRRLIB_LoadTexture(gear_bg_png);
	GRRLIB_texImg *guy_menu_img=GRRLIB_LoadTexture(guy_menu_png);
	GRRLIB_texImg *guy_help_img=GRRLIB_LoadTexture(guy_help_png);
	GRRLIB_texImg *guy_settings_img=GRRLIB_LoadTexture(guy_settings_png);
	GRRLIB_texImg *menu_loader_img=GRRLIB_LoadTexture(menu_loader_png);
	GRRLIB_texImg *menu_loader_highlight_img=GRRLIB_LoadTexture(menu_loader_highlight_png);
	GRRLIB_texImg *menu_reboot_img=GRRLIB_LoadTexture(menu_reboot_png);
	GRRLIB_texImg *menu_reboot_highlight_img=GRRLIB_LoadTexture(menu_reboot_highlight_png);
	GRRLIB_texImg *menu_settings_img=GRRLIB_LoadTexture(menu_settings_png);
	GRRLIB_texImg *menu_settings_highlight_img=GRRLIB_LoadTexture(menu_settings_highlight_png);
	GRRLIB_texImg *setting_cross_img=GRRLIB_LoadTexture(setting_cross_png);
	GRRLIB_texImg *help_bg_img=GRRLIB_LoadTexture(help_bg_png);
	GRRLIB_texImg *tooltip_help_img=GRRLIB_LoadTexture(tooltip_help_png);

	GRRLIB_texImg *tool_tip_installedapps_img=GRRLIB_LoadTexture(tool_tip_installedapps_png);
	GRRLIB_texImg *tool_tip_queue_img=GRRLIB_LoadTexture(tool_tip_queue_png);
	GRRLIB_texImg *tool_tip_sort_date_img=GRRLIB_LoadTexture(tool_tip_sort_date_png);
	GRRLIB_texImg *tool_tip_sort_name_img=GRRLIB_LoadTexture(tool_tip_sort_name_png);
	GRRLIB_texImg *tool_tip_sort_downloads_img=GRRLIB_LoadTexture(tool_tip_sort_downloads_png);
	GRRLIB_texImg *tool_tip_sort_rating_img=GRRLIB_LoadTexture(tool_tip_sort_rating_png);
	GRRLIB_texImg *tool_tip_download_apps_img=GRRLIB_LoadTexture(tool_tip_download_apps_png);
	GRRLIB_texImg *tool_tip_update_apps_img=GRRLIB_LoadTexture(tool_tip_update_apps_png);
	GRRLIB_texImg *version_img=GRRLIB_LoadTexture(version_png);

	GRRLIB_texImg *cancel_download_prompt_img=GRRLIB_LoadTexture(cancel_download_prompt_png);
	GRRLIB_texImg *button_no_img=GRRLIB_LoadTexture(button_no_png);
	GRRLIB_texImg *button_no_highlight_img=GRRLIB_LoadTexture(button_no_highlight_png);
	GRRLIB_texImg *button_yes_img=GRRLIB_LoadTexture(button_yes_png);
	GRRLIB_texImg *button_yes_highlight_img=GRRLIB_LoadTexture(button_yes_highlight_png);
	GRRLIB_texImg *app_download_img=GRRLIB_LoadTexture(app_download_png);
	GRRLIB_texImg *apps_updated_img=GRRLIB_LoadTexture(apps_updated_png);
	GRRLIB_texImg *updated_close_img=GRRLIB_LoadTexture(updated_close_png);
	GRRLIB_texImg *updated_close_highlight_img=GRRLIB_LoadTexture(updated_close_highlight_png);
	GRRLIB_texImg *apps_repo_img=GRRLIB_LoadTexture(apps_repo_png);
	GRRLIB_texImg *apps_start_cat_img=GRRLIB_LoadTexture(apps_start_cat_png);
	GRRLIB_texImg *apps_start_sort_img=GRRLIB_LoadTexture(apps_start_sort_png);

	GRRLIB_texImg *icon1_img = NULL;
	GRRLIB_texImg *icon2_img = NULL;
	GRRLIB_texImg *icon3_img = NULL;
	GRRLIB_texImg *icon4_img = NULL;
	GRRLIB_texImg *icon5_img = NULL;

	// Load icons
	load_icons();

	// Debug
	int xx = 120;
	int yy = 375;

	int ypos = 142;
	int rumble_count = 0;
	bool doRumble = false;
	bool free_string = false;
	int string_count = 6;
	//int string_count2 = 0;
	int start_updated = -1;
	int finish_updated = 0;
	int ypos_updated = 184;

	while(1){

		WPAD_ScanPads();
		u32 pressed = WPAD_ButtonsDown(0);
		u32 held = WPAD_ButtonsHeld(0);

		PAD_ScanPads();
		u32 pressed_gc = PAD_ButtonsDown(0);
		u32 held_gc = PAD_ButtonsHeld(0);

		int old_irx = ir.x;
		int old_iry = ir.y;

		// IR Movement
		WPAD_IR(0, &ir);
		WPAD_Orientation(0, &orient);

		if (!ir.valid) {
			ir.x = old_irx;
			ir.y = old_iry;
			ir.angle = 0;
		}

		// PAD Movement
		if (PAD_StickY(0) > 18 && ir.y > 20) {
			ir.y -=5 ;
			gc_control_used = true;
		}
		if (PAD_StickY(0) < -18 && ir.y < 455) {
			ir.y+=5;
			gc_control_used = true;
		}
		if (PAD_StickX(0) > 18 && ir.x < 625) {
			ir.x+=5;
			gc_control_used = true;
		}
		if (PAD_StickX(0) < -18 && ir.x > -10) {
			ir.x-=5;
			gc_control_used = true;
		}

		if (setting_wiiside == true && !(held & WPAD_BUTTON_1)) {
			if (held & WPAD_BUTTON_RIGHT) {
				ir.y -=5 ;
				gc_control_used = true;
			}
			if (held & WPAD_BUTTON_LEFT) {
				ir.y+=5;
				gc_control_used = true;
			}
			if (held & WPAD_BUTTON_DOWN) {
				ir.x+=5;
				gc_control_used = true;
			}
			if (held & WPAD_BUTTON_UP) {
				ir.x-=5;
				gc_control_used = true;
			}
		}

		GRRLIB_FillScreen(0xFFE1E1E1);
		GRRLIB_DrawImg(0, 0, background_img, 0, 1, 1, 0xFFFFFFFF);

		if (in_menu == false && in_help == false) {
			GRRLIB_DrawImg(580, 150, name_img, 0, 1, 1, 0xFFFFFFFF);
			GRRLIB_DrawImg(580, 200, date_img, 0, 1, 1, 0xFFFFFFFF);
			GRRLIB_DrawImg(580, 250, star_img, 0, 1, 1, 0xFFFFFFFF);
			GRRLIB_DrawImg(580, 300, thumbs_up_img, 0, 1, 1, 0xFFFFFFFF);
			GRRLIB_DrawImg(577, 370, down_img, 0, 1, 1, 0xFFFFFFFF);
		}

		// Apps
		GRRLIB_DrawImg(34, 130, apps_table_blank_img, 0, 1, 1, 0xFFFFFFFF);

		int x;
		int start = -1;
		int finish = 0;
		for (x = 0; x < array_length (homebrew_list); x++) {
			if (ypos + (76 * x) >= 94 && ypos + (76 * x) + 30 < 400) {
				if (start == -1) {
					start = x;
				}

				//if (strlen(homebrew_list[x].name) > 1) {
					finish = x;
				//}
			}
		}

		// Refresh list
		if (refresh_list != start) {
			if (icon1_img)
			{
				free(icon1_img->data);
				free(icon1_img);
			}
			if (icon2_img)
			{
				free(icon2_img->data);
				free(icon2_img);
			}
			if (icon3_img)
			{
				free(icon3_img->data);
				free(icon3_img);
			}
			if (icon4_img)
			{
				free(icon4_img->data);
				free(icon4_img);
			}
			if (icon5_img)
			{
				free(icon5_img->data);
				free(icon5_img);
			}

			// Load images as needed, store them into memory as well as text
			int c;
			for (c = start; c <= (start+4); c++) {

				icons_loaded++;

				if (homebrew_list[c].file_found == 0 || homebrew_list[c].file_found == 2) {

					if (homebrew_list[c].file_found == 0) {
						// Check image file size
						char img_path[100] = "sd:/apps/homebrew_browser/temp/";
						if (setting_use_sd == false) {
							strcpy(img_path,"usb:/apps/homebrew_browser/temp/");
						}
						strcat(img_path, homebrew_list[c].name);
						strcat(img_path, ".png");

						FILE *f = fopen(img_path, "rb");

						if (f == NULL) {
							homebrew_list[c].file_found = -1;
						}
						// Open file and check file length for changes
						else {
							fseek (f , 0, SEEK_END);
							int local_img_size = ftell (f);
							rewind(f);
							fclose(f);

							// Load image into memory if image size matches
							if (local_img_size == homebrew_list[c].img_size) {
								homebrew_list[c].file_found = load_file_to_memory(img_path, &homebrew_list[c].content);
							}
							else {
								homebrew_list[c].file_found = -1;
							}
						}
					}

					// Load text to memory
					/*if (homebrew_list[c].str_name == NULL && homebrew_list[c].str_short_description == NULL) {
						homebrew_list[c].str_name = GRRLIB_TextToTexture(homebrew_list[c].app_name, FONTSIZE_SMALL, 0x575757);
						homebrew_list[c].str_short_description = GRRLIB_TextToTexture(homebrew_list[c].app_short_description, FONTSIZE_SMALL, 0x575757);
						string_count++;
						if (homebrew_list[c].file_found == 2) {
							homebrew_list[c].file_found = 1;
						}
					}*/

				}
				if (homebrew_list[c].content != NULL) { homebrew_list[c].file_found = 1; }

				// Once we've gone through the first 8 images of the category (even if they weren't loaded), we wake the download icon thread and are done
				if (icons_loaded > 8) {
					changing_cat = false;
					icons_loaded = 0;
				}

				// Text list
				if (text_list[c].text == 0) {
					text_list[c].str_name = GRRLIB_TextToTexture(homebrew_list[c].app_name, FONTSIZE_SMALL, 0x575757);
					text_list[c].str_short_description = GRRLIB_TextToTexture(homebrew_list[c].app_short_description, FONTSIZE_SMALL, 0x575757);
					text_list[c].text = 1;

					if (c > string_count) {
						string_count++;
					}

					int a = c - 8;
					if (a > 0) {
						for (a = c - 7; a > 0; a--) {
							if (text_list[a].text != 0) {
								text_list[a].text = 0;

								free(text_list[a].str_name->data);
								free(text_list[a].str_name);
								free(text_list[a].str_short_description->data);
								free(text_list[a].str_short_description);
							}

						}
					}

					a = c + 8;
					if (a < array_length(homebrew_list)) {
						for (a = c + 7; a < array_length(homebrew_list); a++) {
							if (text_list[a].text != 0) {
								text_list[a].text = 0;

								free(text_list[a].str_name->data);
								free(text_list[a].str_name);
								free(text_list[a].str_short_description->data);
								free(text_list[a].str_short_description);
							}

						}
					}

				}
			}

			// Display images
			if (homebrew_list[start].file_found == 1 && strcmp(homebrew_list[start].name,"000") != 0) {
				icon1_img=GRRLIB_LoadTexture(homebrew_list[start].content);
			} else if (strlen(homebrew_list[start].name) >= 3 && strcmp(homebrew_list[start].name,"000") != 0) { icon1_img=GRRLIB_LoadTexture(no_image_png); }
			else { icon1_img=GRRLIB_LoadTexture(blank_png); }

			if (homebrew_list[(start+1)].file_found == 1 && strcmp(homebrew_list[(start+1)].name,"000") != 0) {
				icon2_img=GRRLIB_LoadTexture(homebrew_list[(start+1)].content);
			} else if (strlen(homebrew_list[(start+1)].name) >= 3 && strcmp(homebrew_list[(start+1)].name,"000") != 0) { icon2_img=GRRLIB_LoadTexture(no_image_png); }
			else { icon2_img=GRRLIB_LoadTexture(blank_png); }

			if (homebrew_list[(start+2)].file_found == 1 && strcmp(homebrew_list[(start+2)].name,"000") != 0) {
				icon3_img=GRRLIB_LoadTexture(homebrew_list[(start+2)].content);
			} else if (strlen(homebrew_list[(start+2)].name) >= 3 && strcmp(homebrew_list[(start+2)].name,"000") != 0) { icon3_img=GRRLIB_LoadTexture(no_image_png); }
			else { icon3_img=GRRLIB_LoadTexture(blank_png); }

			if (homebrew_list[(start+3)].file_found == 1 && strcmp(homebrew_list[(start+3)].name,"000") != 0) {
				icon4_img=GRRLIB_LoadTexture(homebrew_list[(start+3)].content);
			} else if (strlen(homebrew_list[(start+3)].name) >= 3 && strcmp(homebrew_list[(start+3)].name,"000") != 0) { icon4_img=GRRLIB_LoadTexture(no_image_png); }
			else { icon4_img=GRRLIB_LoadTexture(blank_png); }

			if (strlen(homebrew_list[(start+4)].name) >= 3 && homebrew_list[(start+4)].file_found == 1 && strcmp(homebrew_list[(start+4)].name,"000") != 0) {
				icon5_img=GRRLIB_LoadTexture(homebrew_list[(start+4)].content);
			} else if (strlen(homebrew_list[(start+4)].name) >= 3 && strcmp(homebrew_list[(start+4)].name,"000") != 0) { icon5_img=GRRLIB_LoadTexture(no_image_png); }
			else { icon5_img=GRRLIB_LoadTexture(blank_png); }

			// Name and Descriptions
			str_name = text_list[start].str_name;
			str_name1 = text_list[(start+1)].str_name;
			str_name2 = text_list[(start+2)].str_name;
			str_name3 = text_list[(start+3)].str_name;
			str_name4 = text_list[(start+4)].str_name;
			str_short_desc = text_list[start].str_short_description;
			str_short_desc1 = text_list[(start+1)].str_short_description;
			str_short_desc2 = text_list[(start+2)].str_short_description;
			str_short_desc3 = text_list[(start+3)].str_short_description;
			str_short_desc4 = text_list[(start+4)].str_short_description;

			refresh_list = start;
		}

		// Depreciated
		/*if (icons_loaded > 8) {
			icons_loaded++;
		}*/
		/*if (icons_loaded > 2000) {
			changing_cat = false;
			icons_loaded = 0;
		}*/

		// Update ypos from ypos updated
		if (show_updated_apps == true || select_repo == true) {
			ypos = ypos_updated;
		}

		// Dpad up / down
		if (setting_wiiside == false) {
			if ((held & WPAD_BUTTON_DOWN || held_gc & PAD_BUTTON_DOWN) && ((strlen(homebrew_list[finish+1].name) > 1 && hbb_app_about == false && updating == -1 && in_menu == false && in_help == false && show_updated_apps == false) || (show_updated_apps == true && finish_updated+1 < updated_apps_count) || (select_repo == true && finish_updated+1 < repo_count))) { ypos-= 4; }
			if ((held & WPAD_BUTTON_UP || held_gc & PAD_BUTTON_UP) && ((show_updated_apps == false && select_repo == false && ypos <= 140) || ((show_updated_apps == true || select_repo == true) && ypos <= 180)) && hbb_app_about == false && updating == -1 && ((in_menu == false && select_repo == false) || (in_menu == true && select_repo == true)) && in_help == false) { ypos+= 4; }
		}

		// Scrolling by B button
		if (pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) {
			ir_pitch = orient.pitch;
			ir_x = ir.x;
			ir_y = ir.y;
			ir_rotation = ir.angle;
		}

		// Wiimote sideways
		if (held & WPAD_BUTTON_1) {
			if (held & WPAD_BUTTON_LEFT && ((strlen(homebrew_list[finish+1].name) > 1 && hbb_app_about == false && updating == -1 && in_menu == false && in_help == false && show_updated_apps == false) || (show_updated_apps == true && finish_updated+1 < updated_apps_count) || (select_repo == true && finish_updated+1 < repo_count))) {
				ypos-= 5;
			}
			if (held & WPAD_BUTTON_RIGHT && ((show_updated_apps == false && select_repo == false && ypos <= 140) || ((show_updated_apps == true || select_repo == true) && ypos <= 180)) && hbb_app_about == false && updating == -1 && ((in_menu == false && select_repo == false) || (in_menu == true && select_repo == true)) && in_help == false) {
				ypos+= 5;
			}
		}

		// Wiimote scrolling
		if (held & WPAD_BUTTON_B) {
			float pitch_dir = orient.pitch - ir_pitch;

			// If moving up
			if ((pitch_dir > 2 && strlen(homebrew_list[finish+1].name) > 1 && show_updated_apps == false && select_repo == false) || (pitch_dir > 2 && show_updated_apps == true && finish_updated+1 < updated_apps_count) || (pitch_dir > 2 && select_repo == true && finish_updated+1 < repo_count)) {
				ypos-= 1;
				if (pitch_dir > 4) {
					ypos-= 1;
				}
				if (pitch_dir > 6) {
					ypos-= 1;
				}
				if (pitch_dir > 8) {
					ypos-= 1;
				}
				if (pitch_dir > 10) {
					ypos-= 2;
				}
				if (pitch_dir > 12) {
					ypos-= 2;
				}
				if (pitch_dir > 30) {
					ypos-= 5;
				}
			}

			// If moving down
			if (pitch_dir < -2 && ((show_updated_apps == false && select_repo == false && ypos <= 140) || (show_updated_apps == true && ypos <= 180) || (select_repo == true && ypos <= 180))) {
				ypos+= 1;
				if (pitch_dir < -4) {
					ypos+= 1;
				}
				if (pitch_dir < -6) {
					ypos+= 1;
				}
				if (pitch_dir < -8) {
					ypos+= 1;
				}
				if (pitch_dir < -10) {
					ypos+= 2;
				}
				if (pitch_dir < -12) {
					ypos+= 2;
				}
				if (pitch_dir < -30) {
					ypos+= 5;
				}
			}

			// Set IR not to move
			ir.x = ir_x;
			ir.y = ir_y;
			ir.angle = ir_rotation;
		}

		// GC scrolling
		if ((strlen(homebrew_list[finish+1].name) > 1 && show_updated_apps == false && select_repo == false) || (show_updated_apps == true && finish_updated+1 < updated_apps_count) || (select_repo == true && finish_updated+1 < repo_count)) {
			if (PAD_SubStickY(0) < -18) {
				ypos-= 2;
			}
			if (PAD_SubStickY(0) < -28) {
				ypos-= 1;
			}
			if (PAD_SubStickY(0) < -38) {
				ypos-= 1;
			}
			if (PAD_SubStickY(0) < -48) {
				ypos-= 1;
			}
			if (PAD_SubStickY(0) < -60) {
				ypos-= 5;
			}
		}
		if ((show_updated_apps == false && select_repo == false && ypos <= 140) || (show_updated_apps == true && ypos <= 180) || (select_repo == true && ypos <= 180)) {
			if (PAD_SubStickY(0) > 18) {
				ypos+= 2;
			}
			if (PAD_SubStickY(0) > 28) {
				ypos+= 1;
			}
			if (PAD_SubStickY(0) > 38) {
				ypos+= 1;
			}
			if (PAD_SubStickY(0) > 48) {
				ypos+= 1;
			}
			if (PAD_SubStickY(0) > 60) {
				ypos+= 5;
			}
		}

		// Nunchuck scrolling
		struct expansion_t data;
		WPAD_Expansion(WPAD_CHAN_0, &data);
		if (data.type == WPAD_EXP_NUNCHUK) {
			//GRRLIB_Printf(150,400,text_font1,0xFF000000,2,"x = %i, y = %i \n", (int) data.nunchuk.js.pos.x, (int) data.nunchuk.js.pos.y);

			if ((strlen(homebrew_list[finish+1].name) > 1 && show_updated_apps == false && select_repo == false) || (show_updated_apps == true && finish_updated+1 < updated_apps_count) || (select_repo == true && finish_updated+1 < repo_count)) {
				if (data.nunchuk.js.pos.y < 120) {
					ypos-= 2;
				}
				if (data.nunchuk.js.pos.y < 110) {
					ypos-= 1;
				}
				if (data.nunchuk.js.pos.y < 100) {
					ypos-= 1;
				}
				if (data.nunchuk.js.pos.y < 80) {
					ypos-= 1;
				}
				if (data.nunchuk.js.pos.y < 70) {
					ypos-= 1;
				}
				if (data.nunchuk.js.pos.y < 50) {
					ypos-= 5;
				}
			}
			if ((show_updated_apps == false && select_repo == false && ypos <= 140) || (show_updated_apps == true && ypos <= 180) || (select_repo == true && ypos <= 180)) {
				if (data.nunchuk.js.pos.y > 140) {
					ypos+= 2;
				}
				if (data.nunchuk.js.pos.y > 150) {
					ypos+= 1;
				}
				if (data.nunchuk.js.pos.y > 160) {
					ypos+= 1;
				}
				if (data.nunchuk.js.pos.y > 180) {
					ypos+= 1;
				}
				if (data.nunchuk.js.pos.y > 190) {
					ypos+= 1;
				}
				if (data.nunchuk.js.pos.y > 210) {
					ypos+= 5;
				}
			}
		}

		// Update ypos updated from ypos
		if (show_updated_apps == true || select_repo == true) {
			ypos_updated = ypos;
			ypos = 142;
		}

		// Display icons, text
		GRRLIB_DrawImg(60, ypos + (76 * start) + 4, icon1_img, 0, 1, 1, 0xFFFFFFC8);
		GRRLIB_DrawImg(210, ypos + (76 * start) + 4, str_name, 0, 1.0, 1.0, 0xFFFFFFFF);
		GRRLIB_DrawImg(210, ypos + (76 * start) + 30, str_short_desc, 0, 1.0, 1.0, 0xFFFFFFFF);

		GRRLIB_DrawImg(60, ypos + (76 * (start+1) + 4), icon2_img, 0, 1, 1, 0xFFFFFFC8);
		GRRLIB_DrawImg(210, ypos + (76 * (start+1)) + 4, str_name1, 0, 1.0, 1.0, 0xFFFFFFFF);
		GRRLIB_DrawImg(210, ypos + (76 * (start+1)) + 30, str_short_desc1, 0, 1.0, 1.0, 0xFFFFFFFF);

		/*if (pressed & WPAD_BUTTON_2) {
			ir_y_temp = ir.y;
		}*/

		/*if (held & WPAD_BUTTON_2 && hbb_app_about == false) {
			GRRLIB_DrawImg(60, (ir.y - ir_y_temp) + ypos + (76 * (start+2) + 4), icon3_img, 0, 1, 1, 0xFFFFFFC8);
			GRRLIB_DrawImg(210, (ir.y - ir_y_temp) + ypos + (76 * (start+2)) + 4, str_name2, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(210, (ir.y - ir_y_temp) + ypos + (76 * (start+2)) + 30, str_short_desc2, 0, 1.0, 1.0, 0xFFFFFFFF);
		}
		else {*/
			GRRLIB_DrawImg(60, ypos + (76 * (start+2) + 4), icon3_img, 0, 1, 1, 0xFFFFFFC8);
			GRRLIB_DrawImg(210, ypos + (76 * (start+2)) + 4, str_name2, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(210, ypos + (76 * (start+2)) + 30, str_short_desc2, 0, 1.0, 1.0, 0xFFFFFFFF);
		//}

		GRRLIB_DrawImg(60, ypos + (76 * (start+3) + 4), icon4_img, 0, 1, 1, 0xFFFFFFC8);
		GRRLIB_DrawImg(210, ypos + (76 * (start+3)) + 4, str_name3, 0, 1.0, 1.0, 0xFFFFFFFF);
		GRRLIB_DrawImg(210, ypos + (76 * (start+3)) + 30, str_short_desc3, 0, 1.0, 1.0, 0xFFFFFFFF);

		GRRLIB_DrawImg(60, ypos + (76 * (start+4) + 4), icon5_img, 0, 1, 1, 0xFFFFFFC8);
		GRRLIB_DrawImg(210, ypos + (76 * (start+4)) + 4, str_name4, 0, 1.0, 1.0, 0xFFFFFFFF);
		GRRLIB_DrawImg(210, ypos + (76 * (start+4)) + 30, str_short_desc4, 0, 1.0, 1.0, 0xFFFFFFFF);

		// Highlighting and display tick, question mark or plus
		if (hbb_app_about == false && updating == -1 && in_menu == false && in_help == false && show_updated_apps == false) {

			int b;
			for(b = 0; b <= 4; b++) {
				if (strlen(homebrew_list[(start + b)].name) >= 2 && strcmp(homebrew_list[(start + b)].name,"000") != 0) {
					if (ir.y > 138 && ir.y < 437 && ir.x > 40 && ir.x < 536 && ir.y > (ypos + (76 * (start + b))) && ir.y < (ypos + (76 * (start + b)) + 64) && !(held & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || held_gc & PAD_BUTTON_B)) {
						doRumble = true;
						GRRLIB_DrawImg(110, ypos + (76 * (start + b)), blue_dark_img, 0, 1, 1, 0xFFFFFFFF);

						if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && hbb_app_about == false) {
							//GRRLIB_Printf(150,300,text_font1,0xFF000000,2,"Icon = %i\n", start + b);
							hbb_app_about = true;
							update_about = true;
							current_app = start + b;
							wait_a_press = 10;
						}
						if ((pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R) && hbb_app_about == false && homebrew_list[(start + b)].local_app_size > 0 && homebrew_list[(start + b)].in_download_queue == 2) {
							homebrew_list[(start + b)].in_download_queue = false;
						}
						else if ((pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R) && hbb_app_about == false && homebrew_list[(start + b)].local_app_size != homebrew_list[(start + b)].app_size) {
							homebrew_list[(start + b)].in_download_queue = true;
						}
						else if ((pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L) && hbb_app_about == false && homebrew_list[(start + b)].local_app_size != homebrew_list[(start + b)].app_size && homebrew_list[(start + b)].in_download_queue == true) {
							homebrew_list[(start + b)].in_download_queue = false;
						}
						else if ((pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L) && hbb_app_about == false && homebrew_list[(start + b)].local_app_size > 0) {
							homebrew_list[(start + b)].in_download_queue = 2;
						}

						// ***************
						/*if (held & WPAD_BUTTON_2 && hbb_app_about == false) {
							current_app = start + b;
						}*/
					}
					else {
						GRRLIB_DrawImg(110, ypos + (76 * (start + b)), blue_light_img, 0, 1, 1, 0xFFFFFFFF);
					}

					if (homebrew_list[(start + b)].local_app_size > 0 && homebrew_list[(start + b)].in_download_queue == false) {
						if (homebrew_list[(start + b)].local_app_size == homebrew_list[(start + b)].app_size && homebrew_list[(start + b)].in_download_queue != 2) {
							GRRLIB_DrawImg(506, ypos + (76 * (start + b)) + 22, app_tick_img, 0, 1, 1, 0xFFFFFFFF);
						}
						else if (homebrew_list[(start + b)].local_app_size != homebrew_list[(start + b)].app_size && homebrew_list[(start + b)].in_download_queue != 2 && homebrew_list[(start + b)].no_manage == false) {
							GRRLIB_DrawImg(506, ypos + (76 * (start + b)) + 22, app_question_img, 0, 1, 1, 0xFFFFFFFF);
						}

						if (homebrew_list[(start + b)].no_manage == true) {
							GRRLIB_DrawImg(506, ypos + (76 * (start + b)) + 22, app_tick_img, 0, 1, 1, 0xFFFFFFFF);
						}
					}
					else {
						//if ((homebrew_list[(start + b)].app_time + 432000) > current_time && (strcmp (store_homebrew_list[0].name, homebrew_list[(start + b)].name) == 0 && (download_in_progress == false || extract_in_progress == false))) {
						//	GRRLIB_DrawImg(468, ypos + (76 * (start + b)) - 6, app_new_img, 0, 1, 1, 0xFFFFFFFF);
						//}
						if ((homebrew_list[(start + b)].app_time + 432000) > current_time && (strcmp (store_homebrew_list[0].name, homebrew_list[(start + b)].name) != 0)) {
							GRRLIB_DrawImg(468, ypos + (76 * (start + b)) - 6, app_new_img, 0, 1, 1, 0xFFFFFFFF);
						}
					}

					if (homebrew_list[(start + b)].in_download_queue == true) {
						GRRLIB_DrawImg(506, ypos + (76 * (start + b)) + 22, app_plus_img, 0, 1, 1, 0xFFFFFFFF);
					}
					else if (homebrew_list[(start + b)].in_download_queue == 2) {
						GRRLIB_DrawImg(506, ypos + (76 * (start + b)) + 22, app_minus_img, 0, 1, 1, 0xFFFFFFFF);
					}

					if (strcmp (store_homebrew_list[0].name, homebrew_list[(start + b)].name) == 0 && (download_in_progress == true || extract_in_progress == true)) {
						GRRLIB_DrawImg(506, ypos + (76 * (start + b)) + 22, app_download_img, 0, 1, 1, 0xFFFFFFFF);
					}
				}
			}
		}

		// Updated apps message
		if (setting_show_updated == true && show_updated_apps == true) {
			GRRLIB_DrawImg(30, 148, apps_updated_img, 0, 1, 1, 0xFFFFFFFF);

			// grrlib text the apps
			if (load_updated == false) {
				int u;
				for (u = 0; u < updated_apps_count; u++) {
					updated_apps_list[u].str_text = GRRLIB_TextToTexture(updated_apps_list[u].text, FONTSIZE_SMALLER, 0x575757);
				}
				load_updated = true;
			}

			int x;
			start_updated = -1;
			for (x = 0; x < updated_apps_count; x++) {
				if (ypos_updated + (25 * x) >= 175 && ypos_updated + (25 * x) + 30 < 425) {
					if (start_updated == -1) {
						start_updated = x;
					}
					if (start_updated >= 0) {
						GRRLIB_DrawImg(50, (x * 25) + ypos_updated, updated_apps_list[x].str_text, 0, 1.0, 1.0, 0xFFFFFFFF);
					}
					finish_updated = x;
				}
			}

			GRRLIB_DrawImg(454, 156, updated_close_img, 0, 1, 1, 0xFFFFFFFF);

			if (ir.x > 438 && ir.x < 490 && ir.y > 155 && ir.y < 173) {
				doRumble = true;
				GRRLIB_DrawImg(454, 156, updated_close_highlight_img, 0, 1, 1, 0xFFFFFFFF);
				if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
					show_updated_apps = false;
					wait_a_press = 10;
				}
			}

			if (pressed & WPAD_BUTTON_HOME) {
				show_updated_apps = false;
				wait_a_press = 10;
			}

			//if (pressed & WPAD_BUTTON_B || pressed_gc & PAD_BUTTON_B) {
			//	show_updated_apps = false;
			//	wait_a_press = 10;
			//}
		}


		GRRLIB_DrawImg(34, 127, apps_table_img, 0, 1, 1, 0xFFFFFFFF);
		GRRLIB_DrawImg(34, 440, apps_bottom_img, 0, 1, 1, 0xFFFFFFFF);
		GRRLIB_DrawImg(34, 68, apps_top_img, 0, 1, 1, 0xFFFFFFFF);
		GRRLIB_DrawImg(40, 122, apps_top2_img, 0, 1, 1, 0xFFFFFFFF);
		GRRLIB_DrawImg(300, 53, version_img, 0, 1, 1, 0xFFFFFFFF);

		// Category selection
		GRRLIB_DrawImg(36, 103, cat_all_img, 0, 1, 1, 0xFFFFFFFF);
		if (category_selection == 0) {
			GRRLIB_DrawImg(34, 103, cat_demo_img, 0, 1, 1, 0xFFFFFFFF);
		}
		if (category_selection == 1) {
			GRRLIB_DrawImg(147, 103, cat_emulator_img, 0, 1, 1, 0xFFFFFFFF);
		}
		if (category_selection == 2) {
			GRRLIB_DrawImg(252, 103, cat_games_img, 0, 1, 1, 0xFFFFFFFF);
		}
		if (category_selection == 3) {
			GRRLIB_DrawImg(359, 103, cat_media_img, 0, 1, 1, 0xFFFFFFFF);
		}
		if (category_selection == 4) {
			GRRLIB_DrawImg(466, 103, cat_utilities_img, 0, 1, 1, 0xFFFFFFFF);
		}
		if (ir.x > 22 && ir.x < 130 && ir.y > 100 && ir.y < 126) {
			doRumble = true;
			if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && updating == -1 && array_length(demos_list) >= 1) {
				category_old_selection = category_selection;
				category_selection = 0;
				updated_cat = true;
				close_windows();
			}
			GRRLIB_DrawImg(34, 103, cat_demo_highlight_img, 0, 1, 1, 0xFFFFFFFF);
		}
		if (ir.x > 134 && ir.x < 234 && ir.y > 100 && ir.y < 126) {
			doRumble = true;
			if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && updating == -1 && array_length(emulators_list) >= 1) {
				category_old_selection = category_selection;
				category_selection = 1;
				updated_cat = true;
				close_windows();
			}
			GRRLIB_DrawImg(147, 103, cat_emulator_highlight_img, 0, 1, 1, 0xFFFFFFFF);
		}
		if (ir.x > 240 && ir.x < 342 && ir.y > 100 && ir.y < 126) {
			doRumble = true;
			if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && updating == -1 && array_length(games_list) >= 1) {
				category_old_selection = category_selection;
				category_selection = 2;
				updated_cat = true;
				close_windows();
			}
			GRRLIB_DrawImg(252, 103, cat_games_highlight_img, 0, 1, 1, 0xFFFFFFFF);
		}
		if (ir.x > 348 && ir.x < 448 && ir.y > 100 && ir.y < 126) {
			doRumble = true;
			if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && updating == -1 && array_length(media_list) >= 1) {
				category_old_selection = category_selection;
				category_selection = 3;
				updated_cat = true;
				close_windows();
			}
			GRRLIB_DrawImg(359, 103, cat_media_highlight_img, 0, 1, 1, 0xFFFFFFFF);
		}
		if (ir.x > 452 && ir.x < 548 && ir.y > 100 && ir.y < 126) {
			doRumble = true;
			if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && updating == -1 && array_length(utilities_list) >= 1) {
				category_old_selection = category_selection;
				category_selection = 4;
				updated_cat = true;
				close_windows();
			}
			GRRLIB_DrawImg(466, 103, cat_utilities_highlight_img, 0, 1, 1, 0xFFFFFFFF);
		}

		// Category Change DPAD
		if (setting_wiiside == false) {
			if ((pressed & WPAD_BUTTON_LEFT || pressed_gc & PAD_BUTTON_LEFT) && category_selection > 0 && category_selection < 5 && hbb_app_about == false && updating == -1 && in_menu == false && in_help == false) {
				if ((category_selection-1 == 0 && array_length(demos_list) >= 1) || (category_selection-1 == 1 && array_length(emulators_list) >= 1) || (category_selection-1 == 2 && array_length(games_list) >= 1) || (category_selection-1 == 3 && array_length(media_list) >= 1) || (category_selection-1 == 4 && array_length(utilities_list) >= 1)) {
					category_old_selection = category_selection;
					category_selection--;
					updated_cat = true;
				}
			}
			if ((pressed & WPAD_BUTTON_RIGHT || pressed_gc & PAD_BUTTON_RIGHT) && category_selection < 4 && hbb_app_about == false && updating == -1 && in_menu == false && in_help == false) {
				if ((category_selection+1 == 0 && array_length(demos_list) >= 1) || (category_selection+1 == 1 && array_length(emulators_list) >= 1) || (category_selection+1 == 2 && array_length(games_list) >= 1) || (category_selection+1 == 3 && array_length(media_list) >= 1) || (category_selection+1 == 4 && array_length(utilities_list) >= 1)) {
					category_old_selection = category_selection;
					category_selection++;
					updated_cat = true;
				}
			}
		}

		// Changed category
		if (category_selection != category_old_selection && updated_cat == true) {
			ypos = 142;
			icons_loaded = 0;
			WPAD_Rumble(WPAD_CHAN_0, 0);

			if (in_help == true) {
				help_section = 0;
				in_help = false;
			}

			// Tell downloading icon thread to sleep until we've loaded the new category
			if (download_icon > 0 && download_in_progress == false) {
				changing_cat = true;
				while (download_icon_sleeping != true) {
					usleep(10000);
				}
			}

			if (category_selection != 9) {
				update_lists();

				// Clear homebrew list
				clear_list();

				// Free list strings
				free_string = true;
			}

			// Update category
			if (category_selection == 0) {
				int i;
				for (i = 0; i < array_length (demos_list); i++) {
					homebrew_list[i] = demos_list[i];
				}
			}
			if (category_selection == 1) {
				int i;
				for (i = 0; i < array_length (emulators_list); i++) {
					homebrew_list[i] = emulators_list[i];
				}
			}
			if (category_selection == 2) {
				int i;
				for (i = 0; i < array_length (games_list); i++) {
					homebrew_list[i] = games_list[i];
				}
			}
			if (category_selection == 3) {
				int i;
				for (i = 0; i < array_length (media_list); i++) {
					homebrew_list[i] = media_list[i];
				}
			}
			if (category_selection == 4) {
				int i;
				for (i = 0; i < array_length (utilities_list); i++) {
					homebrew_list[i] = utilities_list[i];
				}
			}
			if (category_selection == 5) {
				int i;
				for (i = 0; i < 4; i++) {
					strcpy(homebrew_list[i].name,"000");
				}
				int j = 0;
				for (i = 0; i < array_length (total_list); i++) {
					if (total_list[i].local_app_size > 0) {
						homebrew_list[j] = total_list[i];
						j++;
					}
				}
				wait_a_press = 7;
			}
			if (category_selection == 6) {
				int i;
				for (i = 0; i < 4; i++) {
					strcpy(homebrew_list[i].name,"000");
				}
				int j = 0;
				for (i = 0; i < array_length (total_list); i++) {
					//if (total_list[i].in_download_queue == true) {
					if (total_list[i].in_download_queue >= 1) {
						homebrew_list[j] = total_list[i];
						j++;
					}
				}
				wait_a_press = 7;
			}
			if (category_selection == 7) {
				int i;
				for (i = 0; i < 4; i++) {
					strcpy(homebrew_list[i].name,"000");
				}
				int j = 0;
				for (i = 0; i < array_length (total_list); i++) {
					if (total_list[i].local_app_size > 0 && total_list[i].local_app_size != total_list[i].app_size) {
						homebrew_list[j] = total_list[i];
						homebrew_list[j].in_download_queue = true;
						j++;
					}
					else if (total_list[i].local_app_size > 0 && total_list[i].in_download_queue == 2) {
						homebrew_list[j] = total_list[i];
						homebrew_list[j].in_download_queue = 2;
						j++;
					}
				}
				wait_a_press = 7;
			}

			if (setting_hide_installed == true && category_selection != 5 && category_selection != 6 && category_selection != 7) {
				hide_apps_installed();
			}

			if (sort_up_down == 0) { sort_by_date(0); }
			if (sort_up_down == 1) { sort_by_date(1); }
			if (sort_up_down == 2) { sort_by_rating(0); }
			if (sort_up_down == 3) { sort_by_rating(1); }
			if (sort_up_down == 4) { sort_by_downloads(0); }
			if (sort_up_down == 5) { sort_by_downloads(1); }
			if (sort_up_down == 6) { sort_by_name(0); }
			if (sort_up_down == 7) { sort_by_name(1); }

			refresh_list = -1;
			updated_cat = false;
			changing_cat = false;
		}

		if (held & WPAD_BUTTON_1) {
			if (held & WPAD_BUTTON_DOWN) { yy -= 3; }
			if (held & WPAD_BUTTON_UP) { yy += 3; }
			if (held & WPAD_BUTTON_LEFT) { xx -= 3; }
			if (held & WPAD_BUTTON_RIGHT) { xx += 3; }
		}

		if (pressed & WPAD_BUTTON_DOWN) { yy -= 1; }
		if (pressed & WPAD_BUTTON_UP) { yy += 1; }
		if (pressed & WPAD_BUTTON_LEFT) { xx -= 1; }
		if (pressed & WPAD_BUTTON_RIGHT) { xx += 1; }


		// Main menu icon Highlight
		if (ir.x > 480 && ir.x < 530 && ir.y > 25 && ir.y < 75) {
			doRumble = true;
			GRRLIB_DrawImg(483, 17, sd_card_highlight_img, 0, 1, 1, 0xFFFFFFFF);
			if (setting_tool_tip == true) { GRRLIB_DrawImg(300, 26, tool_tip_installedapps_img, 0, 1, 1, 0xFFFFFFFF); }
			if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && updating == -1) {
				category_old_selection = category_selection;
				category_selection = 5;
				updated_cat = true;
				close_windows();
			}
		}
		if (ir.x > 542 && ir.x < 592 && ir.y > 25 && ir.y < 75) {
			doRumble = true;
			GRRLIB_DrawImg(546, 16, help_highlight_img, 0, 1, 1, 0xFFFFFFFF);
			if (setting_tool_tip == true) { GRRLIB_DrawImg(493, 25, tooltip_help_img, 0, 1, 1, 0xFFFFFFFF); }
			if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && updating == -1) {
				close_windows();
				in_help = true;
				help_section = 0;
			}
		}

		if (hbb_app_about == false && updating == -1 && in_menu == false && in_help == false) {
			if (ir.x > 565 && ir.x < 600 && ir.y > 150 && ir.y < 185) {
				doRumble = true;
				GRRLIB_DrawImg(580, 150, name_highlight_img, 0, 1, 1, 0xFFFFFFFF);
				if (setting_tool_tip == true) { GRRLIB_DrawImg(433, 147, tool_tip_sort_name_img, 0, 1, 1, 0xFFFFFFFF); }
				if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
					// Free list strings
					free_string = true;
					if (sort_up_down == 6) { sort_up_down = 7; sort_by_name(1); refresh_list = -1; ypos = 142; }
					else { sort_up_down = 6; sort_by_name(0); refresh_list = -1; ypos = 142; }
				}
			}
			if (ir.x > 565 && ir.x < 600 && ir.y > 200 && ir.y < 235) {
				doRumble = true;
				GRRLIB_DrawImg(580, 200, date_highlight_img, 0, 1, 1, 0xFFFFFFFF);
				if (setting_tool_tip == true) { GRRLIB_DrawImg(450, 197, tool_tip_sort_date_img, 0, 1, 1, 0xFFFFFFFF); }
				if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
					// Free list strings
					free_string = true;
					if (sort_up_down == 0) { sort_up_down = 1; sort_by_date(1); refresh_list = -1; ypos = 142; }
					else { sort_up_down = 0; sort_by_date(0); refresh_list = -1; ypos = 142; }
				}
			}
			if (ir.x > 565 && ir.x < 600 && ir.y > 250 && ir.y < 285) {
				doRumble = true;
				GRRLIB_DrawImg(580, 250, star_highlight_img, 0, 1, 1, 0xFFFFFFFF);
				if (setting_tool_tip == true) { GRRLIB_DrawImg(433, 246, tool_tip_sort_rating_img, 0, 1, 1, 0xFFFFFFFF); }
				if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
					// Free list strings
					free_string = true;
					if (sort_up_down == 2) { sort_up_down = 3; sort_by_rating(1); refresh_list = -1; ypos = 142; }
					else { sort_up_down = 2; sort_by_rating(0); refresh_list = -1; ypos = 142; }
				}
			}
			if (ir.x > 565 && ir.x < 600 && ir.y > 300 && ir.y < 335) {
				doRumble = true;
				GRRLIB_DrawImg(580, 300, thumbs_up_highlight_img, 0, 1, 1, 0xFFFFFFFF);
				if (setting_tool_tip == true) { GRRLIB_DrawImg(392, 295, tool_tip_sort_downloads_img, 0, 1, 1, 0xFFFFFFFF); }
				if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
					// Free list strings
					free_string = true;
					if (sort_up_down == 4) { sort_up_down = 5; sort_by_downloads(1); refresh_list = -1; ypos = 142; }
					else { sort_up_down = 4; sort_by_downloads(0); refresh_list = -1; ypos = 142; }
				}
			}
			if (ir.x > 562 && ir.x < 596 && ir.y > 373 && ir.y < 404) {
				doRumble = true;
				GRRLIB_DrawImg(577, 370, down_highlight_img, 0, 1, 1, 0xFFFFFFFF);
				if (setting_tool_tip == true) {
					if (category_selection == 5) {
						GRRLIB_DrawImg(435, 368, tool_tip_update_apps_img, 0, 1, 1, 0xFFFFFFFF);
					}
					else if (category_selection == 6 || category_selection == 7) {
						GRRLIB_DrawImg(351, 364, tool_tip_download_apps_img, 0, 1, 1, 0xFFFFFFFF);
					}
					else {
						GRRLIB_DrawImg(330, 364, tool_tip_queue_img, 0, 1, 1, 0xFFFFFFFF);
					}
				}
				if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && download_in_progress == false) {
					if (category_selection == 5 && updating == -1 && array_length (homebrew_list) >= 1) {
						//if (setting_online == true && hide_apps_updated() == true) {
							//updating = 0;
							//free_update = true;
							//download_queue_size();
							category_selection = 7;
							updated_cat = true;
						//}
					}
					else {
						if ((category_selection == 6 || category_selection == 7) && updating == -1 && strcmp(homebrew_list[0].name,"000") != 0 && array_length (homebrew_list) >= 1) {
							if (setting_online == true && hide_apps_updated() == true) {
								clear_temp_list();
								updating = 0;
								free_update = true;
								download_queue_size();
							}
						}
						else {
							category_old_selection = category_selection;
							category_selection = 6;
							updated_cat = true;
						}
					}
				}
			}
		}

		// Up down arrows
		if (hbb_app_about == false && updating == -1 && in_menu == false && in_help == false) {
			if (sort_up_down == 0) { GRRLIB_DrawImg(619, 220, sort_arrow_down_img, 0, 1, 1, 0xFFFFFFFF); }
			if (sort_up_down == 1) { GRRLIB_DrawImg(619, 204, sort_arrow_up_img, 0, 1, 1, 0xFFFFFFFF); }
			if (sort_up_down == 2) { GRRLIB_DrawImg(619, 270, sort_arrow_down_img, 0, 1, 1, 0xFFFFFFFF); }
			if (sort_up_down == 3) { GRRLIB_DrawImg(619, 254, sort_arrow_up_img, 0, 1, 1, 0xFFFFFFFF); }
			if (sort_up_down == 4) { GRRLIB_DrawImg(619, 320, sort_arrow_down_img, 0, 1, 1, 0xFFFFFFFF); }
			if (sort_up_down == 5) { GRRLIB_DrawImg(619, 304, sort_arrow_up_img, 0, 1, 1, 0xFFFFFFFF); }
			if (sort_up_down == 6) { GRRLIB_DrawImg(619, 170, sort_arrow_down_img, 0, 1, 1, 0xFFFFFFFF); }
			if (sort_up_down == 7) { GRRLIB_DrawImg(619, 154, sort_arrow_up_img, 0, 1, 1, 0xFFFFFFFF); }
		}

		// Updating
		if (updating >= 0 && strcmp(homebrew_list[0].name,"000") != 0 && in_menu == false && in_help == false) {
			if (free_update == true) {
				char temp[50];
				//if (category_selection == 6) {
					sprintf (temp, "Processing %i/%i applications", new_updating + 1, array_length (homebrew_list));
				//}
				//else {
				//	sprintf (temp, "Updating %i/%i applications", new_updating + 1, array_length (homebrew_list));
				//}

				str_title_status = GRRLIB_TextToTexture(temp, FONTSIZE_SMALL, 0x575757);

				str_res_title = GRRLIB_TextToTexture(homebrew_list[new_updating].app_name, FONTSIZE_SMALL, 0x575757);

				free_update = false;
			}

			GRRLIB_DrawImg(34, 128, about_blank_img, 0, 1, 1, 0xFFFFFFFF);

			if ((updating < array_length (homebrew_list) && new_updating < array_length (homebrew_list)) || new_updating == 10000) {

				GRRLIB_DrawImg(220, 152, str_title_status, 0, 1.0, 1.0, 0xFFFFFFFF);
				GRRLIB_DrawImg(348 - (strlen(homebrew_list[updating].app_name) * 5.5), 196, str_res_title, 0, 1.0, 1.0, 0xFFFFFFFF);

				// Download
				if (download_in_progress == true) {
					int download_progress_count = (int) download_progress_counter / download_part_size;
					if (download_progress_count > 99) { download_progress_count = 100; }

					int x;
					for (x = 0; x < download_progress_count; x++) {
						GRRLIB_DrawImg(139 + (x * 4), 270, blue_progress_img, 0, 1, 1, 0xFFFFFFFF);
					}

					GRRLIB_DrawImg(124, 261, download_frame_img, 0, 1, 1, 0xFFFFFFFF);
					GRRLIB_DrawImg(208, 276, str_downloading, 0, 1.0, 1.0, 0xFFFFFFFF);

					if (pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) {
						cancel_download = true;
						//new_updating = 10000;
						wait_a_press = 20;
					}

					// Download info
					char temp[50];
					double temp_download_progress_counter = download_progress_counter;
					double temp_total_app_size = homebrew_list[updating].total_app_size;
					sprintf (temp, "%3.2fMB / %3.2fMB", (temp_download_progress_counter/1000/1000), (temp_total_app_size/1000/1000));
					str_download_info = GRRLIB_TextToTexture(temp, FONTSIZE_SMALLER, 0x575757);
					GRRLIB_DrawImg(360, 278, str_download_info, 0, 1.0, 1.0, 0xFFFFFFFF);
					free_download_info = true;
				}

				// Extract
				if (extract_in_progress == true) {

					int extract_progress_count = (int) (zip_progress / extract_part_size);
					if (extract_progress_count > 100) { extract_progress_count = 100; }

					int x;
					for (x = 0; x < extract_progress_count; x++) {
						GRRLIB_DrawImg(139 + (x * 4), 270, blue_progress_img, 0, 1, 1, 0xFFFFFFFF);
					}

					GRRLIB_DrawImg(124, 261, download_frame_img, 0, 1, 1, 0xFFFFFFFF);
					GRRLIB_DrawImg(270, 276, str_extracting, 0, 1.0, 1.0, 0xFFFFFFFF);

					if (pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) {
						cancel_extract = true;
						//new_updating = 10000;
						wait_a_press = 20;
					}

					// Extract info
					char temp[50];
					int temp_extract_file_counter = unzip_file_counter;
					int temp_extract_file_count = unzip_file_count;
					sprintf (temp, "%i / %i", temp_extract_file_counter, temp_extract_file_count);
					str_download_info = GRRLIB_TextToTexture(temp, FONTSIZE_SMALLER, 0x575757);
					GRRLIB_DrawImg(410, 278, str_download_info, 0, 1.0, 1.0, 0xFFFFFFFF);
					free_download_info = true;
				}

				// Delete in progress
				if (delete_in_progress == true) {
					GRRLIB_DrawImg(290, 276, str_deleting, 0, 1.0, 1.0, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) {
						cancel_delete = true;
						new_updating = 10000;
						wait_a_press = 20;
					}
				}

				// Cancel prompt
				if ((download_in_progress == true || extract_in_progress == true) && setting_prompt_cancel == true && (cancel_download == true || cancel_extract == true) && cancel_confirmed == false) {
					if (cancel_wait == 0) {
						cancel_wait = 10;
					}
					else if (cancel_wait == 1) {
						cancel_wait = -1;
					}

					if (cancel_wait != -1) {
						cancel_wait--;
					}

					GRRLIB_DrawImg(196, 250, cancel_download_prompt_img, 0, 1, 1, 0xFFFFFFFF);
					GRRLIB_DrawImg(213, 321, button_yes_img, 0, 1, 1, 0xFFFFFFFF);
					GRRLIB_DrawImg(353, 321, button_no_img, 0, 1, 1, 0xFFFFFFFF);

					if (ir.x > 199 && ir.x < 299 && ir.y > 320 && ir.y < 355) {
						doRumble = true;
						GRRLIB_DrawImg(213, 321, button_yes_highlight_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							cancel_confirmed = true;
							new_updating = 10000;
							cancel_wait = 0;
						}
					}

					if (ir.x > 337 && ir.x < 440 && ir.y > 320 && ir.y < 355) {
						doRumble = true;
						GRRLIB_DrawImg(353, 321, button_no_highlight_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							cancel_download = false;
							cancel_extract = false;
							cancel_wait = 0;
						}
					}

					if ((pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) && cancel_wait == -1) {
						cancel_download = false;
						cancel_extract = false;
						cancel_wait = 0;
					}
				}
				else if ((download_in_progress == true || extract_in_progress == true) && setting_prompt_cancel == false && (cancel_download == true || cancel_extract == true)) {
					new_updating = 10000;
				}


				// Overall progress
				int overall_progress = (int) (updating_current_size / updating_part_size);
				if (overall_progress > 98) { overall_progress = 100; }

				//int progress_size =  100 / array_length(homebrew_list);
				//int overall_progress = updating * progress_size;

				int x;
				for (x = 0; x < overall_progress; x++) {
					GRRLIB_DrawImg(139 + (x * 4), 370, blue_progress_img, 0, 1, 1, 0xFFFFFFFF);
				}

				GRRLIB_DrawImg(124, 361, download_frame_img, 0, 1, 1, 0xFFFFFFFF);
				GRRLIB_DrawImg(270, 376, str_overall_progress, 0, 1.0, 1.0, 0xFFFFFFFF);


				if (new_updating != 10000 && (updating != new_updating || updating == 0) && download_in_progress == false && extract_in_progress == false && delete_in_progress == false) {
					updating = new_updating;
					if (strlen(homebrew_list[updating].name) >= 3) {

						// Delete
						if (homebrew_list[updating].in_download_queue == 2) {
							delete_in_progress = true;
							selected_app = updating;
							initialise_delete();
						}

						// Downloading
						else {
							download_in_progress = true;
							selected_app = updating;
							//if (setting_repo == 0) {
								add_to_stats();
							//}
							save_xml_name();
							initialise_download();
							if (homebrew_list[selected_app].local_app_size > 0 && homebrew_list[selected_app].local_app_size != homebrew_list[selected_app].app_size) {
								update_xml = 1;
							}
						}
					}
					free_update = true;
				}

				// Update XML
				if (update_xml == 2) {
					update_xml = 0;
					copy_xml_name();
				}
			}

			// Finished
			else if (download_in_progress == false && extract_in_progress == false && delete_in_progress == false) {
				//GRRLIB_Printf(150,300,text_font1,0xFF000000,2,"FINISHED\n");
				download_in_progress = false;
				extract_in_progress = false;
				delete_in_progress = false;
				cancel_extract = false;
				cancel_download = false;
				display_message_counter = 0;
				error_number = 0;
				updating = -1;
				new_updating = 0;
				cancel_confirmed = false;

				if (category_selection == 7) {
					category_old_selection = 10;
					category_selection = 5;
					updated_cat = true;
				}
				if (category_selection == 6) {
					clear_temp_list();

					int x;
					for (x = 0; x < array_length (homebrew_list); x++) {
						temp_list2[x] = homebrew_list[x];
					}

					clear_list();

					int i;
					for (i = 0; i < 4; i++) {
						strcpy(homebrew_list[i].name,"000");
					}

					i = 0;
					int j = 0;
					for (i = 0; i < array_length (temp_list2); i++) {
						//if (temp_list2[i].local_app_size == temp_list2[i].app_size) {
							homebrew_list[j] = temp_list2[i];
							j++;
						//}
					}

					category_old_selection = 10;
					category_selection = 9;
					updated_cat = true;
					refresh_list = -1;
					free_string = true;
				}
			}

			//GRRLIB_Printf(150,420,text_font1,0xFF000000,2,"App: %i \n", updating);

			// Download failed or extracting failed?
			if (download_in_progress == -1 || extract_in_progress == -1 || delete_in_progress == -1) {

				// Display what error occured
				if (error_number == 1) {
					GRRLIB_DrawImg(210, 276, str_download_zip_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 2) {
					GRRLIB_DrawImg(210, 276, str_create_folder_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 3) {
					GRRLIB_DrawImg(210, 276, str_download_icon_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 4) {
					GRRLIB_DrawImg(210, 276, str_extract_zip_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 5) {
					GRRLIB_DrawImg(190, 276, str_boot_file_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 6) {
					GRRLIB_DrawImg(210, 396, str_delete_file_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 9) {
					GRRLIB_DrawImg(240, 276, str_free_space_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 10) {
					GRRLIB_DrawImg(240, 276, str_no_wifi, 0, 1.0, 1.0, 0xFFFFFFFF);
				}

				// Reset vars
				if ((pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) && wait_a_press == 0) {
					download_in_progress = false;
					extract_in_progress = false;
					delete_in_progress = false;
					cancel_extract = false;
					cancel_download = false;
					display_message_counter = 0;
					error_number = 0;
					cancel_confirmed = false;
					updating = 9999;
				}
			}

			if (wait_a_press > 0) {
				wait_a_press--;
			}

			if ((pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) && download_in_progress == false && extract_in_progress == false && display_message_counter == 0 && wait_a_press == 0) {
				updating = -1;
				new_updating = 0;
				updated_cat = true;
				refresh_list = -1;
				cancel_confirmed = false;
			}
		}


		// About
		if (hbb_app_about == true && updating == -1 && in_menu == false && in_help == false) {
			if (update_about == true) {
				str_res_title = GRRLIB_TextToTexture(homebrew_list[current_app].app_name, FONTSIZE_SMALL, 0x575757);
				str_res_author = GRRLIB_TextToTexture(homebrew_list[current_app].app_author, FONTSIZE_SMALLER, COLOUR_BLUE);
				str_res_version = GRRLIB_TextToTexture(homebrew_list[current_app].app_version, FONTSIZE_SMALLER, COLOUR_BLUE);
				str_res_downloads = GRRLIB_TextToTexture(homebrew_list[current_app].app_downloads, FONTSIZE_SMALLER, COLOUR_BLUE);

				app_icon_img = GRRLIB_LoadTexture(homebrew_list[current_app].content);

				char temp[50];
				if (homebrew_list[current_app].app_total_size > 0 && homebrew_list[current_app].app_total_size < 1048576) {
					int appsize = homebrew_list[current_app].app_total_size / 1024;
					sprintf (temp, "%i KB", appsize);
				}
				else {
					float appsize = (float) (homebrew_list[current_app].app_total_size / 1024) / 1024;
					sprintf (temp, "%1.1f MB", appsize);
				}
				str_res_size = GRRLIB_TextToTexture(temp, FONTSIZE_SMALLER, COLOUR_BLUE);


				app_time = homebrew_list[current_app].app_time;
				char timebuf[50];
				timeinfo = localtime ( &app_time );
				strftime (timebuf,50,"%d %b %Y",timeinfo);
				str_res_date = GRRLIB_TextToTexture(timebuf, FONTSIZE_SMALLER, COLOUR_BLUE);

				const int text_size = sizeof(homebrew_list[0].app_description);
				char text_description[text_size];
				strcpy(text_description, homebrew_list[current_app].app_description);

				int s;
				int l = 0;
				for(s = strlen(text_description); s < text_size; s++) {
					text_description[s] = text_white[0];
					l = text_size > s ? s : text_size;
				}
				text_description[l] = '\0';

				int x;
				int count = 0;
				int textrow = 0;
				int offset = 0;
				char test[80];
				for (x = 0; x < strlen(text_description); x++) {
					test[count] = text_description[x];
					count++;
					if (x >= (55 * (textrow+1)) && x <= (75 * (textrow+1)) && text_description[x] == ' ' && textrow == 0) {
						test[count] = '\0';
						string1 = GRRLIB_TextToTexture(test, FONTSIZE_SMALLER, COLOUR_BLUE);
						textrow = 1;
						offset+= count - 55;
						count = 0;
					}
					if (x >= (55 * (textrow+1) + offset) && x <= (75 * (textrow+1) + offset) && text_description[x] == ' ' && textrow == 1) {
						test[count] = '\0';
						string2 = GRRLIB_TextToTexture(test, FONTSIZE_SMALLER, COLOUR_BLUE);
						textrow = 2;
						offset+= count - 55;
						count = 0;
					}
					if (x >= (55 * (textrow+1) + offset) && x <= (75 * (textrow+1) + offset) && text_description[x] == ' ' && textrow == 2) {
						test[count] = '\0';
						string3 = GRRLIB_TextToTexture(test, FONTSIZE_SMALLER, COLOUR_BLUE);
						textrow = 3;
						offset+= count - 55;
						count = 0;
					}
					if (x >= (55 * (textrow+1) + offset) && x <= (75 * (textrow+1) + offset) && text_description[x] == ' ' && textrow == 3) {
						test[count] = '\0';
						string4 = GRRLIB_TextToTexture(test, FONTSIZE_SMALLER, COLOUR_BLUE);
						textrow = 4;
						offset+= count - 55;
						count = 0;
					}
					if (x >= (55 * (textrow+1) + offset) && x <= (75 * (textrow+1) + offset) && text_description[x] == ' ' && textrow == 4) {
						test[count] = '\0';
						string5 = GRRLIB_TextToTexture(test, FONTSIZE_SMALLER, COLOUR_BLUE);
						count = 0;
						textrow = 5;
						break;
					}
				}

				// Get user's rating if needed
				int user_rating = atoi(homebrew_list[current_app].user_rating);
				if (user_rating == 0 && get_rating_in_progress == false && setting_get_rating == true && setting_online == true && esid > 0 && download_in_progress == false && extract_in_progress == false && delete_in_progress == false) {
					get_rating_in_progress = true;
					selected_app = current_app;
					initialise_rating();
				}

				update_about = false;

				//clear_store_list();
				//store_homebrew_list[0] = homebrew_list[current_app];
			}


			GRRLIB_DrawImg(34, 128, about_blank_img, 0, 1, 1, 0xFFFFFFFF);

			GRRLIB_DrawImg(330 - (strlen(homebrew_list[current_app].app_name) * 5.5), 140, str_res_title, 0, 1.0, 1.0, 0xFFFFFFFF);

			GRRLIB_DrawImg(70, 170, string1, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(70, 190, string2, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(70, 210, string3, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(70, 230, string4, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(70, 250, string5, 0, 1.0, 1.0, 0xFFFFFFFF);

			GRRLIB_DrawImg(70, 290, str_author, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(70, 310, str_version, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(70, 330, str_downloads, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(70, 350, str_size, 0, 1.0, 1.0, 0xFFFFFFFF);

			GRRLIB_DrawImg(290, 290, str_date, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(290, 310, str_controllers, 0, 1.0, 1.0, 0xFFFFFFFF);
			//if ((download_in_progress == false && extract_in_progress == false && delete_in_progress == false && rating_in_progress == false) || (strcmp (store_homebrew_list[0].name, homebrew_list[current_app].name) != 0 && rating_in_progress == false)) {
			//	GRRLIB_DrawImg(70, 375, str_rating, 0, 1.0, 1.0, 0xFFFFFFFF);
 			//	GRRLIB_DrawImg(180, 375, str_your_rating, 0, 1.0, 1.0, 0xFFFFFFFF);
			//}
			GRRLIB_DrawImg(140, 290, str_res_author, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(140, 310, str_res_version, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(174, 330, str_res_downloads, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(120, 350, str_res_size, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(340, 290, str_res_date, 0, 1.0, 1.0, 0xFFFFFFFF);

			if (strstr(homebrew_list[current_app].app_controllers, "wwww")) {
				GRRLIB_DrawImg(290, 330, control_wiimote_4_img, 0, 1, 1, 0xFFFFFFFF);
			}
			else if (strstr(homebrew_list[current_app].app_controllers, "www")) {
				GRRLIB_DrawImg(290, 330, control_wiimote_3_img, 0, 1, 1, 0xFFFFFFFF);
			}
			else if (strstr(homebrew_list[current_app].app_controllers, "ww")) {
				GRRLIB_DrawImg(290, 330, control_wiimote_2_img, 0, 1, 1, 0xFFFFFFFF);
			}
			else if (strstr(homebrew_list[current_app].app_controllers, "w")) {
				GRRLIB_DrawImg(290, 330, control_wiimote_img, 0, 1, 1, 0xFFFFFFFF);
			}
			else { GRRLIB_DrawImg(290, 330, control_wiimote_img, 0, 1, 1, 0xFFFFFF3C); }
			if (strstr(homebrew_list[current_app].app_controllers, "n")) {
				GRRLIB_DrawImg(310, 330, control_nunchuck_img, 0, 1, 1, 0xFFFFFFFF);
			} else { GRRLIB_DrawImg(310, 330, control_nunchuck_img, 0, 1, 1, 0xFFFFFF3C); }
			if (strstr(homebrew_list[current_app].app_controllers, "c")) {
				GRRLIB_DrawImg(346, 330, control_classic_img, 0, 1, 1, 0xFFFFFFFF);
			} else { GRRLIB_DrawImg(346, 330, control_classic_img, 0, 1, 1, 0xFFFFFF3C); }
			if (strstr(homebrew_list[current_app].app_controllers, "g")) {
				GRRLIB_DrawImg(395, 330, control_gcn_img, 0, 1, 1, 0xFFFFFFFF);
			} else { GRRLIB_DrawImg(395, 330, control_gcn_img, 0, 1, 1, 0xFFFFFF3C); }
			if (strstr(homebrew_list[current_app].app_controllers, "z")) {
				GRRLIB_DrawImg(455, 330, control_zapper_img, 0, 1, 1, 0xFFFFFFFF);
			} else { GRRLIB_DrawImg(455, 330, control_zapper_img, 0, 1, 1, 0xFFFFFF3C); }
			if (strstr(homebrew_list[current_app].app_controllers, "k")) {
				GRRLIB_DrawImg(544, 330, control_keyboard_img, 0, 1, 1, 0xFFFFFFFF);
			} else { GRRLIB_DrawImg(544, 330, control_keyboard_img, 0, 1, 1, 0xFFFFFF3C); }
			GRRLIB_DrawImg(489, 290, str_sdhc, 0, 1.0, 1.0, 0xFFFFFFFF);
			if (strstr(homebrew_list[current_app].app_controllers, "s")) {
				GRRLIB_DrawImg(553, 290, str_sdhc_yes, 0, 1.0, 1.0, 0xFFFFFFFF);
			} else { GRRLIB_DrawImg(553, 290, str_sdhc_no, 0, 1.0, 1.0, 0xFFFFFFFF);}

			if ((download_in_progress == false && extract_in_progress == false && delete_in_progress == false && rating_in_progress == false) || (strcmp (store_homebrew_list[0].name, homebrew_list[current_app].name) != 0 && rating_in_progress == false)) {

				{
                    // draw app icon
                    GRRLIB_DrawImg(70, 370, app_icon_img, 0, 1.0, 1.0, 0xFFFFFFFF);

                    //int rating;
//
					//rating = homebrew_list[current_app].app_rating;
					//uint32_t color;
					//for (int i = 0; i < 5; ++i)
					//{
					//	if (rating > i)
					//		color = 0xFFFFFFFF;
					//	else
					//		color = 0x00000040;
//
					//	GRRLIB_DrawImg(70 + ( rate_star_img->w * i ), 400, rate_star_img, 0, 1, 1, color);
					//}
//
					//rating = atoi(homebrew_list[current_app].user_rating);
					//for (int i = 0; i < 5; ++i)
					//{
					//	if (rating > i)
					//		color = 0xFFFFFFFF;
					//	else
					//		color = 0x00000040;
//
					//	GRRLIB_DrawImg(180 + ( rate_star_img->w * i ), 400, rate_star_img, 0, 1, 1, color);
					//}
				}

				// Download or updated enabled?
				if (homebrew_list[current_app].local_app_size > 0) {
					if (homebrew_list[current_app].local_app_size != homebrew_list[current_app].app_size && download_arrow == 0 && homebrew_list[current_app].no_manage == false) {
						GRRLIB_DrawImg(340, 385, update_img, 0, 1, 1, 0xFFFFFFFF);
						if (ir.x > 333 && ir.x < 470 && ir.y > 385 && ir.y < 420) {
							doRumble = true;
							GRRLIB_DrawImg(340, 385, update_highlight_img, 0, 1, 1, 0xFFFFFFFF);
							if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0 && download_in_progress == false && extract_in_progress == false && delete_in_progress == false) {
								download_in_progress = true;
								selected_app = current_app;
								//if (setting_repo == 0) {
									add_to_stats();
								//}
								save_xml_name();
								initialise_download();
								if (homebrew_list[selected_app].local_app_size > 0 && homebrew_list[selected_app].local_app_size != homebrew_list[selected_app].app_size) {
									update_xml = 1;
								}
							}
						}
					}
					else if (homebrew_list[current_app].local_app_size > 0 && download_arrow == 0) {
						GRRLIB_DrawImg(340, 385, update_img, 0, 1, 1, 0xFFFFFF50);
					}

					if (download_arrow == 0) {
						GRRLIB_DrawImg(485, 385, delete_button_img, 0, 1, 1, 0xFFFFFFFF);

						if (ir.x > 475 && ir.x < 575 && ir.y > 385 && ir.y < 420) {
							doRumble = true;
							GRRLIB_DrawImg(485, 385, delete_button_highlight_img, 0, 1, 1, 0xFFFFFFFF);
							if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0 && download_in_progress == false && extract_in_progress == false && delete_in_progress == false) {
								delete_in_progress = true;
								selected_app = current_app;
								initialise_delete();
							}
						}
					}

					if (download_arrow != 0) {
						if (homebrew_list[current_app].no_manage == false) {
							GRRLIB_DrawImg(490, 385, button_yes_img, 0, 1, 1, 0xFFFFFF96);

							if (ir.x > 480 && ir.x < 580 && ir.y > 385 && ir.y < 420) {
								doRumble = true;
								GRRLIB_DrawImg(490, 385, button_yes_img, 0, 1, 1, 0xFFFFFFFF);
								if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0 && download_in_progress == false && extract_in_progress == false && delete_in_progress == false) {
									homebrew_list[current_app].no_manage = true;
								}
							}
						}
						else {
							GRRLIB_DrawImg(490, 385, button_no_img, 0, 1, 1, 0xFFFFFF96);

							if (ir.x > 480 && ir.x < 580 && ir.y > 385 && ir.y < 420) {
								doRumble = true;
								GRRLIB_DrawImg(490, 385, button_no_img, 0, 1, 1, 0xFFFFFFFF);
								if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0 && download_in_progress == false && extract_in_progress == false && delete_in_progress == false) {
									homebrew_list[current_app].no_manage = false;
								}
							}
						}
					}
				}
				else {

					if (download_in_progress == true && strcmp (store_homebrew_list[0].name, homebrew_list[current_app].name) != 0) {
						GRRLIB_DrawImg(340, 385, download_button_img, 0, 1, 1, 0xFFFFFF50);
					}
					else {
						GRRLIB_DrawImg(340, 385, download_button_img, 0, 1, 1, 0xFFFFFFFF);
						if (ir.x > 330 && ir.x < 470 && ir.y > 385 && ir.y < 420) {
							doRumble = true;
							GRRLIB_DrawImg(340, 385, download_button_highlight_img, 0, 1, 1, 0xFFFFFFFF);
							if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0 && download_in_progress == false && extract_in_progress == false && delete_in_progress == false) {
								download_in_progress = true;
								selected_app = current_app;
								//if (setting_repo == 0) {
									add_to_stats();
								//}
								initialise_download();
							}
						}
					}

					if (download_arrow == 0) {
						GRRLIB_DrawImg(485, 385, delete_button_img, 0, 1, 1, 0xFFFFFF50);
					}
				}

				// Rating
				//if (ir.x > 165 && ir.x < 262 && ir.y > 375 && ir.y < 418 && (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0 && setting_online == true && esid > 0 && download_in_progress == false && extract_in_progress == false && delete_in_progress == false) {
				//	rating_in_progress = true;
				//	wait_a_press = 2;
				//}
			}

			// Submit Rating
			//if (rating_in_progress == true) {
			//	GRRLIB_DrawImg(210, 374, str_rate_app, 0, 1.0, 1.0, 0xFFFFFFFF);
			//	//GRRLIB_DrawImg(208, 400, rate_0_img, 0, 2, 2, 0xFFFFFFFF);
//
			//	int rating = 0;
			//	uint32_t color;
			//	bool submit_rating = false;
//
			//	if (wait_a_press == 0) {
			//		if (ir.x > 196 && ir.x < 235 && ir.y > 398 && ir.y < 435) {
			//			rating = 1;
			//			if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) { rating_number[0] = '1'; submit_rating = true; }
			//		}
			//		if (ir.x > 235 && ir.x < 274 && ir.y > 398 && ir.y < 435) {
			//			rating = 2;
			//			if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) { rating_number[0] = '2'; submit_rating = true; }
			//		}
			//		if (ir.x > 274 && ir.x < 312 && ir.y > 398 && ir.y < 435) {
			//			rating = 3;
			//			if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) { rating_number[0] = '3'; submit_rating = true; }
			//		}
			//		if (ir.x > 312 && ir.x < 351 && ir.y > 398 && ir.y < 435) {
			//			rating = 4;
			//			if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) { rating_number[0] = '4'; submit_rating = true; }
			//		}
			//		if (ir.x > 351 && ir.x < 390 && ir.y > 398 && ir.y < 435) {
			//			rating = 5;
			//			if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) { rating_number[0] = '5'; submit_rating = true; }
			//		}
			//	}
//
			//	for (int i = 0; i < 5; ++i)
			//	{
			//		if (rating > i)
			//			color = 0xFFFFFFFF;
			//		else
			//			color = 0x00000040;
//
			//		GRRLIB_DrawImg(208 + ( rate_star_img->w * i * 2 ), 400, rate_star_img, 0, 2, 2, color);
			//	}
//
			//	if (submit_rating == true) {
			//		selected_app = current_app;
			//		initialise_update_rating();
			//		usleep(300000);
			//		rating_in_progress = false;
			//	}
//
			//	if (pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) {
			//		rating_in_progress = false;
			//		wait_a_press = 2;
			//	}
			//}

			// Downloading in progress, display progress
			if (download_in_progress == true && strcmp (store_homebrew_list[0].name, homebrew_list[current_app].name) == 0) {

				int download_progress_count = (int) download_progress_counter / download_part_size;
				if (download_progress_count > 99) { download_progress_count = 100; }

				int x;
				for (x = 0; x < download_progress_count; x++) {
					GRRLIB_DrawImg(139 + (x * 4), 390, blue_progress_img, 0, 1, 1, 0xFFFFFFFF);
				}

				GRRLIB_DrawImg(124, 381, download_frame_img, 0, 1, 1, 0xFFFFFFFF);
				GRRLIB_DrawImg(208, 396, str_downloading, 0, 1.0, 1.0, 0xFFFFFFFF);

				if (pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) {
					cancel_download = true;
				}

				// Download info
				char temp[50];
				double temp_download_progress_counter = download_progress_counter;
				double temp_total_app_size = store_homebrew_list[0].total_app_size;
				sprintf (temp, "%3.2fMB / %3.2fMB", (temp_download_progress_counter/1000/1000), (temp_total_app_size/1000/1000));
				str_download_info = GRRLIB_TextToTexture(temp, FONTSIZE_SMALLER, 0x575757);
				GRRLIB_DrawImg(360, 398, str_download_info, 0, 1.0, 1.0, 0xFFFFFFFF);
				free_download_info = true;
			}

			// Extracting in progress, display progress
			if (extract_in_progress == true && strcmp (store_homebrew_list[0].name, homebrew_list[current_app].name) == 0) {

				int extract_progress_count = (int) (zip_progress / extract_part_size);
				if (extract_progress_count > 100) { extract_progress_count = 100; }

				int x;
				for (x = 0; x < extract_progress_count; x++) {
					GRRLIB_DrawImg(139 + (x * 4), 390, blue_progress_img, 0, 1, 1, 0xFFFFFFFF);
				}

				GRRLIB_DrawImg(124, 381, download_frame_img, 0, 1, 1, 0xFFFFFFFF);
				GRRLIB_DrawImg(270, 396, str_extracting, 0, 1.0, 1.0, 0xFFFFFFFF);

				if (pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) {
					cancel_extract = true;
				}

				// Extract info
				char temp[50];
				int temp_extract_file_counter = unzip_file_counter;
				int temp_extract_file_count = unzip_file_count;
				sprintf (temp, "%i / %i", temp_extract_file_counter, temp_extract_file_count);
				str_download_info = GRRLIB_TextToTexture(temp, FONTSIZE_SMALLER, 0x575757);
				GRRLIB_DrawImg(410, 398, str_download_info, 0, 1.0, 1.0, 0xFFFFFFFF);
				free_download_info = true;
			}

			// Cancel prompt
			if ((download_in_progress == true || extract_in_progress == true) && setting_prompt_cancel == true && (cancel_download == true || cancel_extract == true) && cancel_confirmed == false) {
				if (cancel_wait == 0) {
					cancel_wait = 10;
				}
				else if (cancel_wait == 1) {
					cancel_wait = -1;
				}

				if (cancel_wait != -1) {
					cancel_wait--;
				}

				GRRLIB_DrawImg(196, 250, cancel_download_prompt_img, 0, 1, 1, 0xFFFFFFFF);
				GRRLIB_DrawImg(213, 321, button_yes_img, 0, 1, 1, 0xFFFFFFFF);
				GRRLIB_DrawImg(353, 321, button_no_img, 0, 1, 1, 0xFFFFFFFF);

				if (ir.x > 199 && ir.x < 299 && ir.y > 320 && ir.y < 355) {
					doRumble = true;
					GRRLIB_DrawImg(213, 321, button_yes_highlight_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						cancel_confirmed = true;
						cancel_wait = 0;
					}
				}

				if (ir.x > 337 && ir.x < 440 && ir.y > 320 && ir.y < 355) {
					doRumble = true;
					GRRLIB_DrawImg(353, 321, button_no_highlight_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						cancel_download = false;
						cancel_extract = false;
						cancel_wait = 0;
					}
				}

				if ((pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) && cancel_wait == -1) {
					cancel_download = false;
					cancel_extract = false;
					cancel_wait = 0;
				}
			}

			if (download_in_progress == false && extract_in_progress == false) {
				cancel_download = false;
				cancel_extract = false;
				cancel_wait = 0;
			}

			// Delete in progress
			if (delete_in_progress == true && strcmp (store_homebrew_list[0].name, homebrew_list[current_app].name) == 0) {
				GRRLIB_DrawImg(270, 396, str_deleting, 0, 1.0, 1.0, 0xFFFFFFFF);
			}


			// Download failed or extracting failed or deleting failed?
			if ((download_in_progress == -1 || extract_in_progress == -1 || delete_in_progress == -1) && display_message_counter == 0) {

				// Display what error occured
				if (error_number == 2 || error_number == 3 || error_number == 4 || error_number == 5) {
					initialise_delete();
				}

				cancel_extract = false;
				display_message_counter = 200;
			}

			// Display error message for some time
			if (display_message_counter > 0 && strcmp (store_homebrew_list[0].name, homebrew_list[current_app].name) == 0) {
				display_message_counter--;

				// Display what error occured
				if (error_number == 1) {
					GRRLIB_DrawImg(190, 396, str_download_zip_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 2) {
					GRRLIB_DrawImg(190, 396, str_create_folder_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 3) {
					GRRLIB_DrawImg(190, 396, str_download_icon_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 4) {
					GRRLIB_DrawImg(190, 396, str_extract_zip_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 5) {
					GRRLIB_DrawImg(190, 396, str_boot_file_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 6) {
					GRRLIB_DrawImg(190, 396, str_delete_file_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 7) {
					GRRLIB_DrawImg(210, 396, str_delete_folder_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 8) {
					GRRLIB_DrawImg(190, 396, str_delete_app_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 9) {
					GRRLIB_DrawImg(190, 396, str_free_space_failed, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				if (error_number == 10) {
					GRRLIB_DrawImg(190, 396, str_no_wifi, 0, 1.0, 1.0, 0xFFFFFFFF);
				}

				if (pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) {
					display_message_counter = 1;
				}

				// Reset vars
				if (display_message_counter == 1) {
					download_in_progress = false;
					extract_in_progress = false;
					delete_in_progress = false;
					cancel_download = false;
					cancel_extract = false;
					display_message_counter = 0;
					error_number = 0;
					cancel_confirmed = false;
					download_arrow = 0;
				}
			}

			if (wait_a_press > 0) {
				wait_a_press--;
			}

			if ((pressed & WPAD_BUTTON_HOME || pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B || pressed_gc & PAD_BUTTON_START) && ((download_in_progress == false && extract_in_progress == false && delete_in_progress == false) || strcmp (store_homebrew_list[0].name, homebrew_list[current_app].name) != 0) && display_message_counter == 0 && wait_a_press == 0) {
				hbb_app_about = false;
				free_about = true;
				download_arrow = 0;
				if (download_in_progress == false && extract_in_progress == false && delete_in_progress == false) {
					strcpy(store_homebrew_list[0].name, " ");
				}
			}
		}

		// Empty messages
		if ((category_selection == 6 || category_selection == 7) && strcmp(homebrew_list[0].name,"000") == 0 && wait_a_press == 0 && in_menu == false && in_help == false) {
			GRRLIB_DrawImg(88, 159, str_empty_queue, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(88, 189, str_empty_queue1, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(88, 219, str_empty_queue2, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(88, 249, str_empty_queue3, 0, 1.0, 1.0, 0xFFFFFFFF);
		}
		if (category_selection == 5 && strcmp(homebrew_list[0].name,"000") == 0 && wait_a_press == 0 && in_menu == false && in_help == false) {
			GRRLIB_DrawImg(88, 159, str_empty_sd, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(88, 189, str_empty_sd1, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(88, 219, str_empty_sd2, 0, 1.0, 1.0, 0xFFFFFFFF);
		}

		// Help
		if (in_help == true) {
			GRRLIB_DrawImg(34, 128, about_blank_img, 0, 1, 1, 0xFFFFFFFF);

			if (help_section == 0) {
				if (ir.x > 250 && ir.x < 590 && ir.y > 146 && ir.y < 188) {
					doRumble = true;
					GRRLIB_DrawImg(164, 146, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 2;
					}
				}
				if (ir.x > 250 && ir.x < 590 && ir.y > 196 && ir.y < 238) {
					doRumble = true;
					GRRLIB_DrawImg(164, 196, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 4;
					}
				}
				if (ir.x > 250 && ir.x < 590 && ir.y > 246 && ir.y < 288) {
					doRumble = true;
					GRRLIB_DrawImg(164, 246, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 6;
					}
				}
				if (ir.x > 250 && ir.x < 590 && ir.y > 296 && ir.y < 338) {
					doRumble = true;
					GRRLIB_DrawImg(164, 296, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 7;
					}
				}
				if (ir.x > 250 && ir.x < 590 && ir.y > 346 && ir.y < 388) {
					doRumble = true;
					GRRLIB_DrawImg(164, 346, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 8;
					}
				}

				if (xfb_height == 480) { GRRLIB_DrawImg(0, 252, guy_help_img, 0, 1, 1, 0xFFFFFFFF); }
				else { GRRLIB_DrawImg(0, 275, guy_help_img, 0, 1, 1, 0xFFFFFFFF); }

				GRRLIB_DrawImg(435, 160, str_help_about, 0, 1.0, 1.0, 0xFFFFFFFF);
				GRRLIB_DrawImg(437, 210, str_help_applisting, 0, 1.0, 1.0, 0xFFFFFFFF);
				GRRLIB_DrawImg(413, 260, str_help_appinfo, 0, 1.0, 1.0, 0xFFFFFFFF);
				GRRLIB_DrawImg(364, 310, str_help_appview, 0, 1.0, 1.0, 0xFFFFFFFF);
				GRRLIB_DrawImg(281, 360, str_help_downapp, 0, 1.0, 1.0, 0xFFFFFFFF);

				if ((pressed & WPAD_BUTTON_PLUS) || (setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || (pressed_gc & PAD_TRIGGER_R) || (pressed_gc & PAD_BUTTON_RIGHT)) {
					help_section = 1;
				}

				GRRLIB_DrawImg(452, 432, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 430 && ir.x < 468 && ir.y > 400 && ir.y < 430) {
					doRumble = true;
					GRRLIB_DrawImg(452, 432, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 1;
					}
				}

				GRRLIB_DrawImg(355, 418, str_page1o2, 0, 1.0, 1.0, 0xFFFFFFFF);

			}
			else if (help_section == 1) {
				if (ir.x > 250 && ir.x < 590 && ir.y > 146 && ir.y < 188) {
					doRumble = true;
					GRRLIB_DrawImg(164, 146, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 11;
					}
				}
				if (ir.x > 250 && ir.x < 590 && ir.y > 196 && ir.y < 238) {
					doRumble = true;
					GRRLIB_DrawImg(164, 196, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 15;
					}
				}
				if (ir.x > 250 && ir.x < 590 && ir.y > 246 && ir.y < 288) {
					doRumble = true;
					GRRLIB_DrawImg(164, 246, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 16;
					}
				}
				if (ir.x > 250 && ir.x < 590 && ir.y > 296 && ir.y < 338) {
					doRumble = true;
					GRRLIB_DrawImg(164, 296, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 17;
					}
				}
				if (ir.x > 250 && ir.x < 590 && ir.y > 346 && ir.y < 388) {
					doRumble = true;
					GRRLIB_DrawImg(164, 346, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 19;
					}
				}

				if (xfb_height == 480) { GRRLIB_DrawImg(0, 252, guy_help_img, 0, 1, 1, 0xFFFFFFFF); }
				else { GRRLIB_DrawImg(0, 275, guy_help_img, 0, 1, 1, 0xFFFFFFFF); }

				GRRLIB_DrawImg(504, 160, str_help_settings, 0, 1.0, 1.0, 0xFFFFFFFF);
				GRRLIB_DrawImg(487, 210, str_help_controller, 0, 1.0, 1.0, 0xFFFFFFFF);
				GRRLIB_DrawImg(511, 260, str_help_ratings, 0, 1.0, 1.0, 0xFFFFFFFF);
				GRRLIB_DrawImg(439, 310, str_help_sort, 0, 1.0, 1.0, 0xFFFFFFFF);
				GRRLIB_DrawImg(340, 360, str_help_queue, 0, 1.0, 1.0, 0xFFFFFFFF);

				if ((pressed & WPAD_BUTTON_MINUS) || (setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || (pressed_gc & PAD_TRIGGER_L) || (pressed_gc & PAD_BUTTON_LEFT)) {
					help_section = 0;
				}

				GRRLIB_DrawImg(332, 395, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 285 && ir.x < 322 && ir.y > 400 && ir.y < 430) {
					doRumble = true;
					GRRLIB_DrawImg(332, 395, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 0;
					}
				}

				GRRLIB_DrawImg(355, 418, str_page2o2, 0, 1.0, 1.0, 0xFFFFFFFF);

			}

			if (help_section == 2) {
				GRRLIB_DrawImg(32, 128, help_about_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 0; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 3; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 0;
					}
				}
				GRRLIB_DrawImg(568, 135, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 3;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page1o2, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 3) {
				GRRLIB_DrawImg(32, 128, help_about_2_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 2; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 0;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page2o2, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 4) {
				GRRLIB_DrawImg(32, 128, help_apps_information_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 0; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 5; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 0;
					}
				}
				GRRLIB_DrawImg(568, 135, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 5;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page1o2, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 5) {
				GRRLIB_DrawImg(32, 128, help_apps_information_2_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 4; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 4;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page2o2, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 6) {
				GRRLIB_DrawImg(32, 128, help_apps_listing_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 0; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 0;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page1o1, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 7) {
				GRRLIB_DrawImg(32, 128, help_apps_installed_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 0; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 0;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page1o1, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 8) {
				GRRLIB_DrawImg(32, 128, help_download_update_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 0; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 9; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 0;
					}
				}
				GRRLIB_DrawImg(568, 135, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 9;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page1o3, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 9) {
				GRRLIB_DrawImg(32, 128, help_download_update_2_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 8; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 10; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 8;
					}
				}
				GRRLIB_DrawImg(568, 135, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 10;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page2o3, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 10) {
				GRRLIB_DrawImg(32, 128, help_download_update_3_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 9; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 9;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page3o3, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 11) {
				GRRLIB_DrawImg(32, 128, help_settings_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 1; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 12; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					doRumble = true;
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 1;
					}
				}
				GRRLIB_DrawImg(568, 135, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 12;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page1o7, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 12) {
				GRRLIB_DrawImg(32, 128, help_settings_2_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 11; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 13; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 11;
					}
				}
				GRRLIB_DrawImg(568, 135, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 13;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page2o7, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 13) {
				GRRLIB_DrawImg(32, 128, help_settings_3_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 12; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 14; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 12;
					}
				}
				GRRLIB_DrawImg(568, 135, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 14;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page3o7, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 14) {
				GRRLIB_DrawImg(32, 128, help_settings_4_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 13; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 22; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 13;
					}
				}
				GRRLIB_DrawImg(568, 135, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 22;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page4o7, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 22) {
				GRRLIB_DrawImg(32, 128, help_settings_5_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 14; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 23; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 14;
					}
				}
				GRRLIB_DrawImg(568, 135, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 23;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page5o7, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 23) {
				GRRLIB_DrawImg(32, 128, help_settings_6_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 22; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 24; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 22;
					}
				}
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 24;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page6o7, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 24) {
				GRRLIB_DrawImg(32, 128, help_settings_7_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 23; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 23;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page7o7, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 15) {
				GRRLIB_DrawImg(32, 128, help_controller_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 1; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 1;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page1o1, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 16) {
				GRRLIB_DrawImg(32, 128, help_ratings_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 1; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 1;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page1o1, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 17) {
				GRRLIB_DrawImg(32, 128, help_sorting_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 1; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 18; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 1;
					}
				}
				GRRLIB_DrawImg(568, 135, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 18;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page1o2, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 18) {
				GRRLIB_DrawImg(32, 128, help_sorting_2_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 17; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 17;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page2o2, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 19) {
				GRRLIB_DrawImg(32, 128, help_queue_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 1; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 20; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 1;
					}
				}
				GRRLIB_DrawImg(568, 135, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 20;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page1o3, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 20) {
				GRRLIB_DrawImg(32, 128, help_queue_2_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 19; }
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || pressed & WPAD_BUTTON_PLUS || pressed_gc & PAD_TRIGGER_R || pressed_gc & PAD_BUTTON_RIGHT) { help_section = 21; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 19;
					}
				}
				GRRLIB_DrawImg(568, 135, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
				if (ir.x > 550 && ir.x < 582 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(568, 135, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 21;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page2o3, 0, 1.0, 1.0, 0xFFFFFFFF);
			}
			else if (help_section == 21) {
				GRRLIB_DrawImg(32, 128, help_queue_3_img, 0, 1, 1, 0xFFFFFFFF);
				if ((setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || pressed & WPAD_BUTTON_MINUS || pressed_gc & PAD_TRIGGER_L || pressed_gc & PAD_BUTTON_LEFT) { help_section = 20; }
				GRRLIB_DrawImg(56, 135, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
				if (ir.x > 46 && ir.x < 80 && ir.y > 140 && ir.y < 172) {
					doRumble = true;
					GRRLIB_DrawImg(56, 135, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						help_section = 20;
					}
				}
				GRRLIB_DrawImg(522, 418, str_page3o3, 0, 1.0, 1.0, 0xFFFFFFFF);
			}

			if (help_section == 0 || help_section == 1) {
				GRRLIB_DrawImg(82, 146, help_bg_img, 0, 1, 1, 0xFFFFFFFF);
			}
			if (wait_a_press > 0) {
				wait_a_press--;
			}
			if ((pressed & WPAD_BUTTON_HOME || pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_START || pressed_gc & PAD_BUTTON_B) && wait_a_press == 0 && help_section == 0) {
				in_help = false;
				wait_a_press = 10;
			}
			else if ((pressed & WPAD_BUTTON_HOME || pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_START || pressed_gc & PAD_BUTTON_B) && help_section != 0 && wait_a_press == 0) {
				if (help_section <= 10) {
					help_section = 0;
				}
				else {
					help_section = 1;
				}
				wait_a_press = 10;
			}
		}

		// Menu
		if (in_menu == true && in_help == false) {
			GRRLIB_DrawImg(34, 130, apps_table_blank_img, 0, 1, 1, 0xFFFFFFFF);

			if (menu_section == 0) {

				if (setting_dischar == false) {
					if (xfb_height == 480) { GRRLIB_DrawImg(0, 252, guy_menu_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(0, 275, guy_menu_img, 0, 1, 1, 0xFFFFFFFF); }
				}

				GRRLIB_DrawImg(283, 155, menu_settings_img, 0, 1, 1, 0xFFFFFFFF);
				if (ir.x > 267 && ir.x < 510 && ir.y > 174 && ir.y < 203) {
					doRumble = true;
					GRRLIB_DrawImg(283, 155, menu_settings_highlight_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						menu_section = 1;
					}
				}

				GRRLIB_DrawImg(283, 230, menu_reboot_img, 0, 1, 1, 0xFFFFFFFF);
				if (ir.x > 267 && ir.x < 510 && ir.y > 250 && ir.y < 278) {
					doRumble = true;
					GRRLIB_DrawImg(283, 230, menu_reboot_highlight_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						WPAD_Rumble(WPAD_CHAN_0, 0);
						WPAD_Rumble(WPAD_CHAN_0, 0);
						stop_mod_music();
						exiting = true;
						if (download_icon > 0) {
							changing_cat = true;
							while (download_icon_sleeping != true) {
								usleep(10000);
							}
						}
						usleep(300000);
						fatUnmount("sd:");
						fatUnmount("usb:");
						//reboot();
						WII_Initialize();
						WII_ReturnToMenu();
					}
				}

				GRRLIB_DrawImg(269, 305, menu_loader_img, 0, 1, 1, 0xFFFFFFFF);
				if (ir.x > 267 && ir.x < 510 && ir.y > 320 && ir.y < 352) {
					doRumble = true;
					GRRLIB_DrawImg(269, 305, menu_loader_highlight_img, 0, 1, 1, 0xFFFFFFFF);
					if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
						WPAD_Rumble(WPAD_CHAN_0, 0);
						WPAD_Rumble(WPAD_CHAN_0, 0);
						exiting = true;
						stop_mod_music();
						usleep(300000);
						if (download_icon > 0) {
							changing_cat = true;
							while (download_icon_sleeping != true) {
								usleep(10000);
							}
						}
						fatUnmount("sd:");
						fatUnmount("usb:");
						exit(0);
					}
				}

				// Exit
				if ((pressed & WPAD_BUTTON_HOME || pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_START || pressed_gc & PAD_BUTTON_B) && wait_a_press == 0) {
					in_menu = false;
				}
			}
			else {
				if (menu_section == 1) {

					if (ir.x > 250 && ir.x < 530 && ir.y > 146 && ir.y < 188) {
						doRumble = true;
						GRRLIB_DrawImg(104, 146, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_sd_card == true) { setting_sd_card = false; }
							else {setting_sd_card = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 196 && ir.y < 238) {
						doRumble = true;
						GRRLIB_DrawImg(104, 196, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_check_size == true) { setting_check_size = false; }
							else {setting_check_size = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 246 && ir.y < 288) {
						doRumble = true;
						GRRLIB_DrawImg(104, 246, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_hide_installed == true) { setting_hide_installed = false; }
							else {setting_hide_installed = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 296 && ir.y < 338) {
						doRumble = true;
						GRRLIB_DrawImg(104, 296, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_get_rating == true) { setting_get_rating = false; }
							else {setting_get_rating = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 346 && ir.y < 388) {
						doRumble = true;
						GRRLIB_DrawImg(104, 346, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_music == true) { setting_music = false; stop_mod_music(); }
							else { setting_music = true; play_mod_music(); }
						}
					}

					if (setting_dischar == false) {
						if (xfb_height == 480) { GRRLIB_DrawImg(0, 252, guy_settings_img, 0, 1, 1, 0xFFFFFFFF); }
						else { GRRLIB_DrawImg(0, 275, guy_settings_img, 0, 1, 1, 0xFFFFFFFF); }
					}

					GRRLIB_DrawImg(313, 160, str_setting_card_space, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(304, 210, str_setting_free_space, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(285, 260, str_setting_hide_app, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(277, 310, str_setting_rating, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(290, 360, str_setting_bgmusic, 0, 1.0, 1.0, 0xFFFFFFFF);

					if (setting_sd_card == true) { GRRLIB_DrawImg(498, 148, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 148, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_check_size == true) { GRRLIB_DrawImg(498, 198, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 198, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_hide_installed == true) { GRRLIB_DrawImg(498, 248, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 248, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_get_rating == true) { GRRLIB_DrawImg(498, 298, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 298, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_music == true) { GRRLIB_DrawImg(498, 348, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 348, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }

					if ((pressed & WPAD_BUTTON_PLUS) || (setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || (pressed_gc & PAD_TRIGGER_R) || (pressed_gc & PAD_BUTTON_RIGHT)) {
						menu_section = 2;
					}

					GRRLIB_DrawImg(452, 432, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
					if (ir.x > 430 && ir.x < 468 && ir.y > 400 && ir.y < 430) {
						doRumble = true;
						GRRLIB_DrawImg(452, 432, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							menu_section = 2;
						}
					}

					GRRLIB_DrawImg(355, 418, str_page1o5, 0, 1.0, 1.0, 0xFFFFFFFF);

				}
				else if (menu_section == 2) {

					if (ir.x > 250 && ir.x < 530 && ir.y > 146 && ir.y < 188) {
						doRumble = true;
						GRRLIB_DrawImg(104, 146, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_rumble == true) { setting_rumble = false; }
							else {setting_rumble = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 196 && ir.y < 238) {
						doRumble = true;
						GRRLIB_DrawImg(104, 196, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_update_icon == true) { setting_update_icon = false; }
							else {setting_update_icon = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 246 && ir.y < 288) {
						doRumble = true;
						GRRLIB_DrawImg(104, 246, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_online == true) { setting_online = false; }
							else {setting_online = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 296 && ir.y < 338) {
						doRumble = true;
						GRRLIB_DrawImg(104, 296, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_tool_tip == true) { setting_tool_tip = false; }
							else {setting_tool_tip = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 346 && ir.y < 388) {
						doRumble = true;
						GRRLIB_DrawImg(104, 346, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_prompt_cancel == true) { setting_prompt_cancel = false; }
							else {setting_prompt_cancel = true; }
						}
					}

					if (setting_dischar == false) {
						if (xfb_height == 480) { GRRLIB_DrawImg(0, 252, guy_settings_img, 0, 1, 1, 0xFFFFFFFF); }
						else { GRRLIB_DrawImg(0, 275, guy_settings_img, 0, 1, 1, 0xFFFFFFFF); }
					}

					GRRLIB_DrawImg(399, 160, str_setting_rumble, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(343, 210, str_setting_icon, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(345, 260, str_setting_offline, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(392, 310, str_setting_tool_tip, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(285, 360, str_setting_prompt, 0, 1.0, 1.0, 0xFFFFFFFF);

					if (setting_rumble == true) { GRRLIB_DrawImg(498, 148, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 148, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_update_icon == true) { GRRLIB_DrawImg(498, 198, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 198, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_online == false) { GRRLIB_DrawImg(498, 248, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 248, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_tool_tip == true) { GRRLIB_DrawImg(498, 298, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 298, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_prompt_cancel == true) { GRRLIB_DrawImg(498, 348, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 348, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }


					if ((pressed & WPAD_BUTTON_MINUS) || (setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || (pressed_gc & PAD_TRIGGER_L) || (pressed_gc & PAD_BUTTON_LEFT)) {
						menu_section = 1;
					}
					if ((pressed & WPAD_BUTTON_PLUS) || (setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || (pressed_gc & PAD_TRIGGER_R) || (pressed_gc & PAD_BUTTON_RIGHT)) {
						menu_section = 3;
					}

					GRRLIB_DrawImg(332, 395, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
					if (ir.x > 285 && ir.x < 322 && ir.y > 400 && ir.y < 430) {
						doRumble = true;
						GRRLIB_DrawImg(332, 395, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							menu_section = 1;
						}
					}
					GRRLIB_DrawImg(452, 432, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
					if (ir.x > 430 && ir.x < 468 && ir.y > 400 && ir.y < 430) {
						doRumble = true;
						GRRLIB_DrawImg(452, 432, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							menu_section = 3;
						}
					}

					GRRLIB_DrawImg(355, 418, str_page2o5, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				else if (menu_section == 3) {

					if (ir.x > 250 && ir.x < 530 && ir.y > 146 && ir.y < 188 && select_repo == false && select_category == false) {
						doRumble = true;
						GRRLIB_DrawImg(104, 146, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_power_off == true) { setting_power_off = false; }
							else {setting_power_off = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 196 && ir.y < 238 && select_repo == false && select_category == false) {
						doRumble = true;
						GRRLIB_DrawImg(104, 196, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_show_updated == true) { setting_show_updated = false; }
							else {setting_show_updated = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 246 && ir.y < 288 && select_repo == false && select_category == false) {
						doRumble = true;
						GRRLIB_DrawImg(104, 246, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_use_sd == true) { setting_use_sd = false; }
							else {setting_use_sd = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 296 && ir.y < 338 && select_repo == false && select_category == false) {
						doRumble = true;
						GRRLIB_DrawImg(104, 296, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							select_repo = true;
							wait_a_press = 10;
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 346 && ir.y < 388 && select_repo == false && select_category == false) {
						doRumble = true;
						GRRLIB_DrawImg(104, 346, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							select_category = true;
							wait_a_press = 10;
						}
					}

					if (setting_dischar == false) {
						if (xfb_height == 480) { GRRLIB_DrawImg(0, 252, guy_settings_img, 0, 1, 1, 0xFFFFFFFF); }
						else { GRRLIB_DrawImg(0, 275, guy_settings_img, 0, 1, 1, 0xFFFFFFFF); }
					}

					GRRLIB_DrawImg(292, 160, str_setting_power, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(259, 210, str_setting_updated, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(355, 260, str_setting_sd, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(244, 310, str_setting_repo, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(300, 360, str_setting_cat, 0, 1.0, 1.0, 0xFFFFFFFF);

					if (setting_power_off == true) { GRRLIB_DrawImg(498, 148, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 148, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_show_updated == true) { GRRLIB_DrawImg(498, 198, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 198, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_use_sd == true) { GRRLIB_DrawImg(498, 248, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 248, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_repo > 0) { GRRLIB_DrawImg(498, 298, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 298, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_category == 2) { GRRLIB_DrawImg(498, 348, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 348, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }

					if (((pressed & WPAD_BUTTON_MINUS) || (setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || (pressed_gc & PAD_TRIGGER_L) || (pressed_gc & PAD_BUTTON_LEFT)) && select_repo == false && select_category == false) {
						menu_section = 2;
					}
					if (((pressed & WPAD_BUTTON_PLUS) || (setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || (pressed_gc & PAD_TRIGGER_R) || (pressed_gc & PAD_BUTTON_RIGHT)) && select_repo == false && select_category == false) {
						menu_section = 4;
					}

					GRRLIB_DrawImg(332, 395, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
					if (ir.x > 285 && ir.x < 322 && ir.y > 400 && ir.y < 430 && select_repo == false && select_category == false) {
						doRumble = true;
						GRRLIB_DrawImg(332, 395, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							menu_section = 2;
						}
					}
					GRRLIB_DrawImg(452, 432, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
					if (ir.x > 430 && ir.x < 468 && ir.y > 400 && ir.y < 430 && select_repo == false && select_category == false) {
						doRumble = true;
						GRRLIB_DrawImg(452, 432, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							menu_section = 4;
						}
					}

					GRRLIB_DrawImg(355, 418, str_page3o5, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				else if (menu_section == 4) {

					if (ir.x > 250 && ir.x < 530 && ir.y > 146 && ir.y < 188 && select_sort == false) {
						doRumble = true;
						GRRLIB_DrawImg(104, 146, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							select_sort = true;
							wait_a_press = 10;
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 196 && ir.y < 238 && select_sort == false) {
						doRumble = true;
						GRRLIB_DrawImg(104, 196, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_disusb == true) { setting_disusb = false; }
							else {setting_disusb = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 246 && ir.y < 288 && select_sort == false) {
						doRumble = true;
						GRRLIB_DrawImg(104, 246, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_dischar == true) { setting_dischar = false; }
							else {setting_dischar = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 296 && ir.y < 338 && select_sort == false) {
						doRumble = true;
						GRRLIB_DrawImg(104, 296, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_wiiside == true) { setting_wiiside = false; }
							else {setting_wiiside = true; }
						}
					}
					if (ir.x > 250 && ir.x < 530 && ir.y > 346 && ir.y < 388 && select_sort == false) {
						doRumble = true;
						GRRLIB_DrawImg(104, 346, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_update == true) { setting_update = false; }
							else {setting_update = true; }

						}
					}

					if (setting_dischar == false) {
						if (xfb_height == 480) { GRRLIB_DrawImg(0, 252, guy_settings_img, 0, 1, 1, 0xFFFFFFFF); }
						else { GRRLIB_DrawImg(0, 275, guy_settings_img, 0, 1, 1, 0xFFFFFFFF); }
					}

					GRRLIB_DrawImg(351, 160, str_setting_sort, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(282, 210, str_setting_disusb, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(296, 260, str_setting_dischar, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(248, 310, str_setting_wiiside, 0, 1.0, 1.0, 0xFFFFFFFF);
					GRRLIB_DrawImg(286, 360, str_setting_update, 0, 1.0, 1.0, 0xFFFFFFFF);

					if (setting_sort == 1) { GRRLIB_DrawImg(498, 148, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 148, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_disusb == true) { GRRLIB_DrawImg(498, 198, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 198, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_dischar == true) { GRRLIB_DrawImg(498, 248, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 248, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_wiiside == true) { GRRLIB_DrawImg(498, 298, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 298, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					if (setting_update == true) { GRRLIB_DrawImg(498, 348, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 348, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }

					if (((pressed & WPAD_BUTTON_MINUS) || (setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || (pressed_gc & PAD_TRIGGER_L) || (pressed_gc & PAD_BUTTON_LEFT)) && select_sort == false) {
						menu_section = 3;
					}
					if (((pressed & WPAD_BUTTON_PLUS) || (setting_wiiside == false && pressed & WPAD_BUTTON_RIGHT) || (pressed_gc & PAD_TRIGGER_R) || (pressed_gc & PAD_BUTTON_RIGHT)) && select_repo == false && select_category == false) {
						menu_section = 5;
					}

					GRRLIB_DrawImg(332, 395, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
					if (ir.x > 285 && ir.x < 322 && ir.y > 400 && ir.y < 430 && select_sort == false) {
						doRumble = true;
						GRRLIB_DrawImg(332, 395, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							menu_section = 3;
						}
					}
					GRRLIB_DrawImg(452, 432, down_clear_img, 270, 1, 1, 0xFFFFFFFF);
					if (ir.x > 430 && ir.x < 468 && ir.y > 400 && ir.y < 430 && select_sort == false) {
						doRumble = true;
						GRRLIB_DrawImg(452, 432, down_clear_highlight_img, 270, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							menu_section = 5;
						}
					}

					GRRLIB_DrawImg(355, 418,  str_page4o5, 0, 1.0, 1.0, 0xFFFFFFFF);
				}
				else if (menu_section == 5) {

					if (ir.x > 250 && ir.x < 530 && ir.y > 146 && ir.y < 188) {
						doRumble = true;
						GRRLIB_DrawImg(104, 146, blue_light_small_img, 0, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							if (setting_server == true) { setting_server = false; }
							else {setting_server = true; }
						}
					}

					if (setting_dischar == false) {
						if (xfb_height == 480) { GRRLIB_DrawImg(0, 252, guy_settings_img, 0, 1, 1, 0xFFFFFFFF); }
						else { GRRLIB_DrawImg(0, 275, guy_settings_img, 0, 1, 1, 0xFFFFFFFF); }
					}

					GRRLIB_DrawImg(258, 160, str_setting_server, 0, 1.0, 1.0, 0xFFFFFFFF);

					if (setting_server == false) { GRRLIB_DrawImg(498, 148, setting_cross_img, 0, 1, 1, 0xFFFFFFFF); }
					else { GRRLIB_DrawImg(498, 148, app_tick_img, 0, 1, 1, 0xFFFFFFFF); }

					if (((pressed & WPAD_BUTTON_MINUS) || (setting_wiiside == false && pressed & WPAD_BUTTON_LEFT) || (pressed_gc & PAD_TRIGGER_L) || (pressed_gc & PAD_BUTTON_LEFT)) && select_sort == false) {
						menu_section = 4;
					}

					GRRLIB_DrawImg(332, 395, down_clear_img, 90, 1, 1, 0xFFFFFFFF);
					if (ir.x > 285 && ir.x < 322 && ir.y > 400 && ir.y < 430) {
						doRumble = true;
						GRRLIB_DrawImg(332, 395, down_clear_highlight_img, 90, 1, 1, 0xFFFFFFFF);
						if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
							menu_section = 4;
						}
					}

					GRRLIB_DrawImg(355, 418, str_page5o5, 0, 1.0, 1.0, 0xFFFFFFFF);
				}

				if ((pressed & WPAD_BUTTON_HOME || ((pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1) && select_repo == false && select_category == false) || pressed_gc & PAD_BUTTON_START || pressed_gc & PAD_BUTTON_B) && wait_a_press == 0) {
					menu_section = 0;
					select_repo = false;
					select_category = false;
					update_settings();
				}
			}

			if (wait_a_press > 0) {
				wait_a_press--;
			}
			GRRLIB_DrawImg(82, 146, gear_bg_img, 0, 1, 1, 0xFFFFFFFF);
		}
		else {
			if ((pressed & WPAD_BUTTON_HOME || pressed_gc & PAD_BUTTON_START) && hbb_app_about == false && updating == -1) {
				in_menu = true;
				wait_a_press = 10;
			}
		}

		// Update XML
		if (update_xml == 2) {
			update_xml = 0;
			copy_xml_name();
		}

		// Select Repo
		if (select_repo == true) {
			GRRLIB_DrawImg(123, 148, apps_repo_img, 0, 1, 1, 0xFFFFFFFF);

			if (repo_texted == false) {
				int u;
				for (u = 0; u < repo_count; u++) {
					repo_list[u].str_text = GRRLIB_TextToTexture(repo_list[u].name, FONTSIZE_SMALLER, 0x575757);
				}
				repo_texted = true;
			}

			int x;
			start_updated = -1;
			for (x = 0; x < repo_count; x++) {
				if (ypos_updated + (25 * x) >= 175 && ypos_updated + (25 * x) + 30 < 425) {
					if (start_updated == -1) {
						start_updated = x;
					}
					if (start_updated >= 0) {

						if (ir.y > 138 && ir.y < 437 && ir.x > 40 && ir.x < 536 && ir.y > (ypos_updated + (25 * x)) && ir.y < (ypos_updated + (25 * x) + 20) && !(held & WPAD_BUTTON_B || held_gc & PAD_BUTTON_B)) {
							doRumble = true;
							GRRLIB_DrawImg(150, ypos_updated + (25 * x) - 1, blue_dark_small_img, 0, 1, 1, 0xFFFFFFC8);
							if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0) {
								setting_repo = x;
							}
						}

						GRRLIB_DrawImg(150, (x * 25) + ypos_updated, repo_list[x].str_text, 0, 1.0, 1.0, 0xFFFFFFFF);

						if (setting_repo == x) {
							GRRLIB_DrawImg(150 + strlen(repo_list[x].name) * 10, ypos_updated + (25 * x), app_tick_small_img, 0, 1, 1, 0xFFFFFFFF);
						}

					}
					finish_updated = x;
				}
			}

			GRRLIB_DrawImg(426, 156, updated_close_img, 0, 1, 1, 0xFFFFFFFF);

			if (ir.x > 412 && ir.x < 460 && ir.y > 155 && ir.y < 173) {
				doRumble = true;
				GRRLIB_DrawImg(426, 156, updated_close_highlight_img, 0, 1, 1, 0xFFFFFFFF);
				if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
					select_repo = false;
					wait_a_press = 10;
				}
			}

			if (pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) {
				select_repo = false;
				wait_a_press = 10;
			}
		}

		// Select category
		if (select_category == true) {
			GRRLIB_DrawImg(123, 148, apps_start_cat_img, 0, 1, 1, 0xFFFFFFFF);

			GRRLIB_DrawImg(180, 195, str_cat_1, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(180, 225, str_cat_2, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(180, 255, str_cat_3, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(180, 285, str_cat_4, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(180, 315, str_cat_5, 0, 1.0, 1.0, 0xFFFFFFFF);

			if (ir.y > 195 && ir.y < 220 && ir.x > 150 && ir.x < 400) {
				doRumble = true;
				GRRLIB_DrawImg(165, 197, blue_dark_small2_img, 0, 1, 1, 0xFFFFFFC8);
				if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0) {
					setting_category = 0;
				}
			}
			if (setting_category == 0) {	GRRLIB_DrawImg(380, 198, app_tick_small_img, 0, 1, 1, 0xFFFFFFFF); }

			if (ir.y > 225 && ir.y < 250 && ir.x > 150 && ir.x < 400) {
				doRumble = true;
				GRRLIB_DrawImg(165, 227, blue_dark_small2_img, 0, 1, 1, 0xFFFFFFC8);
				if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0) {
					setting_category = 1;
				}
			}
			if (setting_category == 1) {	GRRLIB_DrawImg(380, 228, app_tick_small_img, 0, 1, 1, 0xFFFFFFFF); }

			if (ir.y > 255 && ir.y < 280 && ir.x > 150 && ir.x < 400) {
				doRumble = true;
				GRRLIB_DrawImg(165, 257, blue_dark_small2_img, 0, 1, 1, 0xFFFFFFC8);
				if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0) {
					setting_category = 2;
				}
			}
			if (setting_category == 2) {	GRRLIB_DrawImg(380, 258, app_tick_small_img, 0, 1, 1, 0xFFFFFFFF); }

			if (ir.y > 285 && ir.y < 310 && ir.x > 150 && ir.x < 400) {
				doRumble = true;
				GRRLIB_DrawImg(165, 287, blue_dark_small2_img, 0, 1, 1, 0xFFFFFFC8);
				if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0) {
					setting_category = 3;
				}
			}
			if (setting_category == 3) {	GRRLIB_DrawImg(380, 288, app_tick_small_img, 0, 1, 1, 0xFFFFFFFF); }

			if (ir.y > 315 && ir.y < 340 && ir.x > 150 && ir.x < 400) {
				doRumble = true;
				GRRLIB_DrawImg(165, 317, blue_dark_small2_img, 0, 1, 1, 0xFFFFFFC8);
				if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0) {
					setting_category = 4;
				}
			}
			if (setting_category == 4) {	GRRLIB_DrawImg(380, 318, app_tick_small_img, 0, 1, 1, 0xFFFFFFFF); }


			GRRLIB_DrawImg(386, 156, updated_close_img, 0, 1, 1, 0xFFFFFFFF);

			if (ir.x > 372 && ir.x < 420 && ir.y > 155 && ir.y < 173) {
				doRumble = true;
				GRRLIB_DrawImg(386, 156, updated_close_highlight_img, 0, 1, 1, 0xFFFFFFFF);
				if (pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) {
					select_category = false;
					wait_a_press = 10;
				}
			}

			if (pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) {
				select_category = false;
				wait_a_press = 10;
			}
		}

		// Select sort
		if (select_sort == true) {
			GRRLIB_DrawImg(123, 148, apps_start_sort_img, 0, 1, 1, 0xFFFFFFFF);

			GRRLIB_DrawImg(180, 195, str_sort_1, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(180, 225, str_sort_2, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(180, 255, str_sort_3, 0, 1.0, 1.0, 0xFFFFFFFF);
			GRRLIB_DrawImg(180, 285, str_sort_4, 0, 1.0, 1.0, 0xFFFFFFFF);

			if (ir.y > 195 && ir.y < 220 && ir.x > 150 && ir.x < 400) {
				doRumble = true;
				GRRLIB_DrawImg(165, 197, blue_dark_small2_img, 0, 1, 1, 0xFFFFFFC8);
				if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0) {
					setting_sort = 0;
				}
			}
			if (setting_sort == 0) {	GRRLIB_DrawImg(380, 198, app_tick_small_img, 0, 1, 1, 0xFFFFFFFF); }

			if (ir.y > 225 && ir.y < 250 && ir.x > 150 && ir.x < 400) {
				doRumble = true;
				GRRLIB_DrawImg(165, 227, blue_dark_small2_img, 0, 1, 1, 0xFFFFFFC8);
				if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0) {
					setting_sort = 1;
				}
			}
			if (setting_sort == 1) {	GRRLIB_DrawImg(380, 228, app_tick_small_img, 0, 1, 1, 0xFFFFFFFF); }

			if (ir.y > 255 && ir.y < 280 && ir.x > 150 && ir.x < 400) {
				doRumble = true;
				GRRLIB_DrawImg(165, 257, blue_dark_small2_img, 0, 1, 1, 0xFFFFFFC8);
				if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0) {
					setting_sort = 2;
				}
			}
			if (setting_sort == 2) {	GRRLIB_DrawImg(380, 258, app_tick_small_img, 0, 1, 1, 0xFFFFFFFF); }

			if (ir.y > 285 && ir.y < 310 && ir.x > 150 && ir.x < 400) {
				doRumble = true;
				GRRLIB_DrawImg(165, 287, blue_dark_small2_img, 0, 1, 1, 0xFFFFFFC8);
				if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0) {
					setting_sort = 3;
				}
			}
			if (setting_sort == 3) {	GRRLIB_DrawImg(380, 288, app_tick_small_img, 0, 1, 1, 0xFFFFFFFF); }


			GRRLIB_DrawImg(386, 156, updated_close_img, 0, 1, 1, 0xFFFFFFFF);

			if (ir.x > 372 && ir.x < 420 && ir.y > 155 && ir.y < 173) {
				doRumble = true;
				GRRLIB_DrawImg(386, 156, updated_close_highlight_img, 0, 1, 1, 0xFFFFFFFF);
				if ((pressed & WPAD_BUTTON_A || pressed & WPAD_BUTTON_2 || pressed_gc & PAD_BUTTON_A) && wait_a_press == 0) {
					select_sort = false;
					wait_a_press = 10;
				}
			}

			if (pressed & WPAD_BUTTON_B || pressed & WPAD_BUTTON_1 || pressed_gc & PAD_BUTTON_B) {
				select_sort = false;
				wait_a_press = 10;
			}
		}

		// SD card usage
		if (setting_sd_card == true && sd_card_update == true) {
			WPAD_Rumble(WPAD_CHAN_0, 0);
			struct statvfs fiData;
			if (setting_use_sd == true) {
				if (statvfs("sd:/",&fiData) < 0) {
					//snprintf(diskinfo, sizeof(diskinfo)-1, "Free space: Unknown");
				}
				else{
					sd_card_free = fiData.f_bfree;
					sd_card_free = round(sd_card_free * fiData.f_bsize);
					sd_card_free = sd_card_free / 1000 / 1000;

					//sd_used = fiData.f_blocks*fiData.f_bsize/1000000;
					/*if (fiData.f_bfree >= 1000) {
						sd_card_free = fiData.f_bfree;
						sd_card_free = sd_card_free / 1000;
						sd_card_free = round(sd_card_free * fiData.f_bsize);
						sd_card_free = sd_card_free / 1000;
					}
					else {
						sd_card_free = fiData.f_bsize;
						sd_card_free = sd_card_free / 1000;
						sd_card_free = round(sd_card_free * fiData.f_bfree);
						sd_card_free = sd_card_free / 1000;
					}*/
				}
			}
			else if (setting_use_sd == false) {
				if (statvfs("usb:/",&fiData) < 0) {
				//snprintf(diskinfo, sizeof(diskinfo)-1, "Free space: Unknown");
				}
				else{
					sd_card_free = fiData.f_bfree;
					sd_card_free = round(sd_card_free * fiData.f_bsize);
					sd_card_free = sd_card_free / 1000 / 1000;
					//sd_used = fiData.f_blocks*fiData.f_bsize/1000000;
					/*if (fiData.f_bfree >= 1000) {
						sd_card_free = fiData.f_bfree;
						sd_card_free = sd_card_free / 1000;
						sd_card_free = round(sd_card_free * fiData.f_bsize);
						sd_card_free = sd_card_free / 1000;
					}
					else {
						sd_card_free = fiData.f_bsize;
						sd_card_free = sd_card_free / 1000;
						sd_card_free = round(sd_card_free * fiData.f_bfree);
						sd_card_free = sd_card_free / 1000;
					}*/
				}
			}

			if (free_sd_size == true) {
				free(str_sd_card->data);
				free(str_sd_card);
				free_sd_size = false;
			}

			char temp[50];
			sprintf (temp, "%lli MB Free", sd_card_free);

			str_sd_card = GRRLIB_TextToTexture(temp, FONTSIZE_SMALLER, 0x9d9d9d);
			sd_card_update = false;
			free_sd_size = true;
		}

		if (setting_sd_card == true) {
			GRRLIB_DrawImg(468, 441, str_sd_card, 0, 1.0, 1.0, 0xFFFFFFFF);
		}

		// Rumble
		if (setting_rumble == true) {
			if (doRumble == false) {
				rumble_count = 0;
			}
			if (rumble_count > 2) {
				WPAD_Rumble(WPAD_CHAN_0, 0);
				doRumble = false;
			}
			else if (doRumble == true) {
				if (ir.valid) {
					WPAD_Rumble(WPAD_CHAN_0, 1);
				}
				rumble_count++;
			}
		}

		// Icon load info
		if (download_icon > 0 && download_in_progress == false && extract_in_progress == false) {
			char temp[50];
			sprintf (temp, "Checking icon %i / %i", download_icon, array_length(total_list));
			str_icon_info = GRRLIB_TextToTexture(temp, FONTSIZE_SMALLER, 0x9d9d9d);
			GRRLIB_DrawImg(248, 441, str_icon_info, 0, 1.0, 1.0, 0xFFFFFFFF);
			free_icon_info = true;
		}


		if (download_in_progress == true && updating == -1 && strcmp (store_homebrew_list[0].name, homebrew_list[current_app].name) != 0) {
			GRRLIB_DrawImg(248, 441, str_downloading_small, 0, 1.0, 1.0, 0xFFFFFFFF);
		}
		if (extract_in_progress == true && updating == -1 && strcmp (store_homebrew_list[0].name, homebrew_list[current_app].name) != 0) {
			GRRLIB_DrawImg(248, 441, str_extracting_small, 0, 1.0, 1.0, 0xFFFFFFFF);
		}

		// Draw the IR pointer
		if (ir.valid || (ir.valid && held & WPAD_BUTTON_B) || held_gc & PAD_BUTTON_B) {
			GRRLIB_DrawImg(ir.x, ir.y - 36, mouse_img, ir.angle, 1, 1, 0xFFFFFFFF);
			gc_control_used = false;
		}
		else if (gc_control_used == true) {
			GRRLIB_DrawImg(ir.x, ir.y - 36, mouse_img, ir.angle, 1, 1, 0xFFFFFFFF);
		}

		GRRLIB_Render();

		// Free about text when out of about window
		if (free_about == true) {
			free(string1->data);
			free(string1);
			free(string2->data);
			free(string2);
			free(string3->data);
			free(string3);
			free(string4->data);
			free(string4);
			free(string5->data);
			free(string5);
			free(str_res_title->data);
			free(str_res_title);
			free(str_res_author->data);
			free(str_res_author);
			free(str_res_version->data);
			free(str_res_version);
			free(str_res_downloads->data);
			free(str_res_downloads);
			free(str_res_size->data);
			free(str_res_size);
			free(str_res_date->data);
			free(str_res_date);
			GRRLIB_ClearTex(app_icon_img);
			free(app_icon_img->data);
			free(app_icon_img);
			free_about = false;
		}

		// Free the strings from the list
		if (free_string == true) {
			int a = 0;
			for (a = 0; a < string_count; a++) {
				if (text_list[a].text != 0) {
					text_list[a].text = 0;

					free(text_list[a].str_name);
					free(text_list[a].str_short_description);
				}

			}
			free_string = false;
			string_count = 6;
		}

		/*if (string_count / 100 > string_count2) {
			string_count2 = string_count / 100;
			int a = string_count - 100;
			for (a = 0; a < string_count-8; a++) {
				text_list[a].text = 0;

				free(text_list[a].str_name);
				free(text_list[a].str_short_description);

			}
		}*/

		// Icon loading info
		if (free_icon_info == true && download_icon > 0) {
			free(str_icon_info->data);
			free(str_icon_info);
			free_icon_info = false;
		}

		// Download size progress
		if (free_download_info == true) {
			free(str_download_info->data);
			free(str_download_info);
			free_download_info = false;
		}

		if (wait_a_press > 0) {
			wait_a_press--;
		}

		// Exit
		if ((held & WPAD_BUTTON_HOME || held_gc & PAD_BUTTON_START) && held_counter >= 1) {
			held_counter++;
		}
		if (pressed & WPAD_BUTTON_HOME || pressed_gc & PAD_BUTTON_START) {
			held_counter = 1;
		}
		if (held_counter > 20) {
			WPAD_Rumble(WPAD_CHAN_0, 0);
			WPAD_Rumble(WPAD_CHAN_0, 0);
			update_lists();
			if (setting_repo == 0) {
				//save_no_manage_list();
			}
			stop_mod_music();
			exiting = true;
			usleep(300000);
			if (download_icon > 0) {
				changing_cat = true;
				while (download_icon_sleeping != true) {
					usleep(10000);
				}
			}
			fatUnmount("sd:");
			fatUnmount("usb:");
			exit(0);
		}

		if (HWButton != -1 && setting_power_off == true) {
			WPAD_Rumble(WPAD_CHAN_0, 0);
			WPAD_Rumble(WPAD_CHAN_0, 0);
			update_lists();
			if (setting_repo == 0) {
				//save_no_manage_list();
			}
			stop_mod_music();
			exiting = true;
			usleep(300000);
			if (download_icon > 0) {
				changing_cat = true;
				while (download_icon_sleeping != true) {
					usleep(10000);
				}
			}
			fatUnmount("sd:");
			fatUnmount("usb:");
			SYS_ResetSystem(HWButton, 0, 0);
		}

	}

	// Ensure libcurl has been cleaned up.
	curl_global_cleanup();

	return 0;
}
