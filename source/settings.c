/* Manages loading and saving preferences for the Homebrew Browser. */
#include <mxml.h>
#include <stdbool.h>
#include <stdio.h>

// TODO(spotlightishere): We rely on common.h for file-related
// information. Let's migrate this all to its own file.
#include "common.h"

#pragma mark - Global variables

bool setting_check_size = true;
bool setting_sd_card = true;
bool setting_hide_installed = false;
bool setting_get_rating = true;
bool setting_music = false;
bool setting_online = true;
bool setting_rumble = true;
bool setting_update_icon = false;
bool setting_tool_tip = true;
char setting_last_boot[14] = "1";
bool setting_show_updated = true;
bool setting_prompt_cancel = true;
bool setting_power_off = false;
bool setting_use_sd = true;
int setting_repo = 0;
bool setting_repo_revert = false;
int setting_category = 2;
int setting_sort = 1;
bool setting_disusb = false;
bool setting_dischar = false;
bool setting_wiiside = false;
bool setting_update = true;
bool setting_server = false;

bool sd_mounted = false;
bool usb_mounted = false;

#pragma mark - Settings manipulation functions

void load_settings() {
	mxml_node_t *tree;
	mxml_node_t *data;

	FILE *fp = NULL;
	int loaded_from = true;

	if (sd_mounted == true) {
		fp = fopen("sd:/apps/homebrew_browser/settings.xml", "rb");
		if (fp == NULL) {
			fclose(fp);
			fp = fopen("usb:/apps/homebrew_browser/settings.xml", "rb");
			loaded_from = false;
		}
	}
	else {
		fp = fopen("usb:/apps/homebrew_browser/settings.xml", "rb");
		loaded_from = false;
	}

	if (fp != NULL) {
		fseek (fp , 0, SEEK_END);
		long settings_size = ftell (fp);
		rewind (fp);

		if (settings_size > 0) {

			tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
			fclose(fp);

			data = mxmlFindElement(tree, tree, "settings", NULL, NULL, MXML_DESCEND);

			if (mxmlElementGetAttr(data,"setting_check_size")) {
				setting_check_size = atoi(mxmlElementGetAttr(data,"setting_check_size"));
			}
			if (mxmlElementGetAttr(data,"setting_sd_card")) {
				setting_sd_card = atoi(mxmlElementGetAttr(data,"setting_sd_card"));
			}
			if (mxmlElementGetAttr(data,"setting_hide_installed")) {
				setting_hide_installed = atoi(mxmlElementGetAttr(data,"setting_hide_installed"));
			}
			if (mxmlElementGetAttr(data,"setting_get_rating")) {
				setting_get_rating = atoi(mxmlElementGetAttr(data,"setting_get_rating"));
			}
			if (mxmlElementGetAttr(data,"setting_music")) {
				setting_music = atoi(mxmlElementGetAttr(data,"setting_music"));
			}
			if (mxmlElementGetAttr(data,"setting_online") && setting_online == true) {
				setting_online = atoi(mxmlElementGetAttr(data,"setting_online"));
			}
			if (mxmlElementGetAttr(data,"setting_rumble")) {
				setting_rumble = atoi(mxmlElementGetAttr(data,"setting_rumble"));
			}
			if (mxmlElementGetAttr(data,"setting_update_icon")) {
				setting_update_icon = atoi(mxmlElementGetAttr(data,"setting_update_icon"));
			}
			if (mxmlElementGetAttr(data,"setting_tool_tip")) {
				setting_tool_tip = atoi(mxmlElementGetAttr(data,"setting_tool_tip"));
			}
			if (mxmlElementGetAttr(data,"setting_prompt_cancel")) {
				setting_prompt_cancel = atoi(mxmlElementGetAttr(data,"setting_prompt_cancel"));
			}
			if (mxmlElementGetAttr(data,"setting_power_off")) {
				setting_power_off = atoi(mxmlElementGetAttr(data,"setting_power_off"));
			}
			if (mxmlElementGetAttr(data,"setting_last_boot")) {
				if (atoi(mxmlElementGetAttr(data,"setting_last_boot")) > 0) {
					strcpy(setting_last_boot, mxmlElementGetAttr(data,"setting_last_boot"));
				}
			}
			if (mxmlElementGetAttr(data,"setting_show_updated")) {
				setting_show_updated = atoi(mxmlElementGetAttr(data,"setting_show_updated"));
			}
			if (mxmlElementGetAttr(data,"setting_use_sd")) {
				setting_use_sd = atoi(mxmlElementGetAttr(data,"setting_use_sd"));
			}
			if (mxmlElementGetAttr(data,"setting_repo")) {
				if (atoi(mxmlElementGetAttr(data,"setting_repo")) >= 0) {
					setting_repo = atoi(mxmlElementGetAttr(data,"setting_repo"));
				}
			}
			if (mxmlElementGetAttr(data,"setting_sort")) {
				if (atoi(mxmlElementGetAttr(data,"setting_sort")) >= 0) {
					setting_sort = atoi(mxmlElementGetAttr(data,"setting_sort"));
				}
			}
			if (mxmlElementGetAttr(data,"setting_category")) {
				if (atoi(mxmlElementGetAttr(data,"setting_category")) >= 0) {
					setting_category = atoi(mxmlElementGetAttr(data,"setting_category"));
				}
			}
			if (mxmlElementGetAttr(data,"setting_disusb")) {
				setting_disusb = atoi(mxmlElementGetAttr(data,"setting_disusb"));
			}
			if (mxmlElementGetAttr(data,"setting_dischar")) {
				setting_dischar = atoi(mxmlElementGetAttr(data,"setting_dischar"));
			}
			if (mxmlElementGetAttr(data,"setting_wiiside")) {
				setting_wiiside = atoi(mxmlElementGetAttr(data,"setting_wiiside"));
			}
			if (mxmlElementGetAttr(data,"setting_update")) {
				setting_update = atoi(mxmlElementGetAttr(data,"setting_update"));
			}
			if (mxmlElementGetAttr(data,"setting_server")) {
				setting_server = atoi(mxmlElementGetAttr(data,"setting_server"));
			}

			mxmlDelete(data);
			mxmlDelete(tree);

			if (loaded_from == true) {
				printf("Settings loaded from SD card.\n");
			}
			else {
				printf("Settings loaded from USB device.\n");
			}

			// Double check that setting SD is correct
			if (setting_use_sd == true && sd_mounted == false) {
				setting_use_sd = false;
				printf("Settings say to load from SD, no SD found. Using USB instead.\n");
			}
			else if (setting_use_sd == false && usb_mounted == false) {
				setting_use_sd = true;
				printf("Settings say to load from USB, no USB found. Using SD instead.\n");
			}
			printf("\n");
		}
		else {
			if (loaded_from == true) {
				remove_file("sd:/apps/homebrew_browser/settings.xml");
			}
			else {
				remove_file("sd:/apps/homebrew_browser/settings.xml");
			}
		}
	}
	fclose(fp);

	// Setting repo revert to codemii
	if (setting_repo_revert == true) {
		setting_repo = 0;
	}

	// What device to use?
	if (setting_use_sd == true) {
		strcpy(rootdir, "sd:/");
	}
	else {
		strcpy(rootdir, "usb:/");
	}
}

void update_settings() {
	mxml_node_t *xml;
	mxml_node_t *data;

	xml = mxmlNewXML("1.0");

	data = mxmlNewElement(xml, "settings");

	char set1[2];
	sprintf(set1, "%i", setting_check_size);
	mxmlElementSetAttr(data, "setting_check_size", set1);
	char set2[2];
	sprintf(set2, "%i", setting_sd_card);
	mxmlElementSetAttr(data, "setting_sd_card", set2);
	char set3[2];
	sprintf(set3, "%i", setting_hide_installed);
	mxmlElementSetAttr(data, "setting_hide_installed", set3);
	char set4[2];
	sprintf(set4, "%i", setting_get_rating);
	mxmlElementSetAttr(data, "setting_get_rating", set4);
	char set5[2];
	sprintf(set5, "%i", setting_music);
	mxmlElementSetAttr(data, "setting_music", set5);
	char set6[2];
	sprintf(set6, "%i", setting_online);
	mxmlElementSetAttr(data, "setting_online", set6);
	char set7[2];
	sprintf(set7, "%i", setting_rumble);
	mxmlElementSetAttr(data, "setting_rumble", set7);
	char set8[2];
	sprintf(set8, "%i", setting_update_icon);
	mxmlElementSetAttr(data, "setting_update_icon", set8);
	char set9[2];
	sprintf(set9, "%i", setting_tool_tip);
	mxmlElementSetAttr(data, "setting_tool_tip", set9);
	char set10[2];
	sprintf(set10, "%i", setting_prompt_cancel);
	mxmlElementSetAttr(data, "setting_prompt_cancel", set10);
	char set11[2];
	sprintf(set11, "%i", setting_power_off);
	mxmlElementSetAttr(data, "setting_power_off", set11);
	mxmlElementSetAttr(data, "setting_last_boot", setting_last_boot);
	char set12[2];
	sprintf(set12, "%i", setting_show_updated);
	mxmlElementSetAttr(data, "setting_show_updated", set12);
	char set13[2];
	sprintf(set13, "%i", setting_use_sd);
	mxmlElementSetAttr(data, "setting_use_sd", set13);
	char set14[3];
	sprintf(set14, "%i", setting_repo);
	mxmlElementSetAttr(data, "setting_repo", set14);
	char set15[2];
	sprintf(set15, "%i", setting_sort);
	mxmlElementSetAttr(data, "setting_sort", set15);
	char set16[2];
	sprintf(set16, "%i", setting_category);
	mxmlElementSetAttr(data, "setting_category", set16);
	char set17[2];
	sprintf(set17, "%i", setting_disusb);
	mxmlElementSetAttr(data, "setting_disusb", set17);
	char set18[2];
	sprintf(set18, "%i", setting_dischar);
	mxmlElementSetAttr(data, "setting_dischar", set18);
	char set19[2];
	sprintf(set19, "%i", setting_wiiside);
	mxmlElementSetAttr(data, "setting_wiiside", set19);
	char set20[2];
	sprintf(set20, "%i", setting_update);
	mxmlElementSetAttr(data, "setting_update", set20);
	char set21[2];
	sprintf(set21, "%i", setting_server);
	mxmlElementSetAttr(data, "setting_server", set21);

	FILE *fp = fopen("sd:/apps/homebrew_browser/settings.xml", "wb");
	FILE *fp1 = fopen("usb:/apps/homebrew_browser/settings.xml", "wb");

	if (fp == NULL) {
		//printf("Settings file not found\n");
	}
	else {
		mxmlSaveFile(xml, fp, MXML_NO_CALLBACK);
		fclose(fp);
		//printf("saved\n");
	}

	if (fp1 == NULL) {
		//printf("Settings file not found\n");
	}
	else {
		mxmlSaveFile(xml, fp1, MXML_NO_CALLBACK);
		fclose(fp1);
	}

	mxmlDelete(data);
	mxmlDelete(xml);
}
