/* Manages loading and saving preferences for the Homebrew Browser. */
#include <mxml.h>
#include <stdbool.h>
#include <stdio.h>

extern bool setting_check_size;
extern bool setting_sd_card;
extern bool setting_hide_installed;
extern bool setting_get_rating;
extern bool setting_music;
extern bool setting_online;
extern bool setting_rumble;
extern bool setting_update_icon;
extern bool setting_tool_tip;
extern char setting_last_boot[14];
extern bool setting_show_updated;
extern bool setting_prompt_cancel;
extern bool setting_power_off;
extern bool setting_use_sd;
extern int setting_repo;
extern bool setting_repo_revert;
extern int setting_sort;
extern int setting_category;
extern bool setting_disusb;
extern bool setting_dischar;
extern bool setting_wiiside;
extern bool setting_update;
extern bool setting_server;

void update_settings();
void load_settings();
void load_mount_settings();
