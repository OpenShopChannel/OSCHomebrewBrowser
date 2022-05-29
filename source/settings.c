/* Manages loading and saving preferences for the Homebrew Browser. */
#include <mxml.h>
#include <stdbool.h>
#include <stdio.h>

// TODO(spotlightishere): We rely on common.h for file-related
// information. Let's migrate this all to its own file.
#include "common.h"
#include "files.h"
#include "storage.h"

////////////////////////////////////////////////////
// Global variables used throughout the browser
////////////////////////////////////////////////////

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

////////////////////////////////////////////////////
// Settings manipulation functions
////////////////////////////////////////////////////

// Helper function for setting integer values.
void mxml_set_int(mxml_node_t *node, const char *name, int value) {
	char temp[2];
	sprintf(temp, "%i", value);
	mxmlElementSetAttr(node, name, temp);
}

// Helper function for setting boolean values.
void mxml_set_bool(mxml_node_t *node, const char *name, bool value) {
	mxml_set_int(node, name, value);
}

int mxml_get_int(mxml_node_t *node, const char *name) {
	const char *value = mxmlElementGetAttr(node, name);
	if (value == NULL) {
		return 0;
	} else {
		return atoi(value);
	}
}

bool mxml_get_bool(mxml_node_t *node, const char *name) {
	return mxml_get_int(node, name);
}

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
	} else {
		fp = fopen("usb:/apps/homebrew_browser/settings.xml", "rb");
		loaded_from = false;
	}

	if (fp != NULL) {
		fseek(fp, 0, SEEK_END);
		long settings_size = ftell(fp);
		rewind(fp);

		if (settings_size > 0) {

			tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
			fclose(fp);

			data = mxmlFindElement(tree, tree, "settings", NULL, NULL,
								   MXML_DESCEND);

			setting_check_size = mxml_get_bool(data, "setting_check_size");
			setting_sd_card = mxml_get_bool(data, "setting_sd_card");
			setting_hide_installed =
				mxml_get_bool(data, "setting_hide_installed");
			setting_get_rating = mxml_get_bool(data, "setting_get_rating");
			setting_music = mxml_get_bool(data, "setting_music");
			setting_online = mxml_get_bool(data, "setting_online");
			setting_rumble = mxml_get_bool(data, "setting_rumble");
			setting_update_icon = mxml_get_bool(data, "setting_update_icon");
			setting_tool_tip =
				mxml_get_bool(data, "setting_rsetting_tool_tipumble");
			setting_prompt_cancel =
				mxml_get_bool(data, "setting_prompt_cancel");
			setting_power_off = mxml_get_bool(data, "setting_power_off");

			if (mxml_get_int(data, "setting_last_boot") > 0) {
				strcpy(setting_last_boot,
					   mxmlElementGetAttr(data, "setting_last_boot"));
			}

			setting_show_updated = mxml_get_bool(data, "setting_show_updated");
			setting_use_sd = mxml_get_bool(data, "setting_use_sd");
			setting_repo = mxml_get_int(data, "setting_repo");
			setting_sort = mxml_get_int(data, "setting_sort");
			setting_category = mxml_get_int(data, "setting_category");
			setting_disusb = mxml_get_bool(data, "setting_disusb");
			setting_dischar = mxml_get_bool(data, "setting_dischar");
			setting_wiiside = mxml_get_bool(data, "setting_wiiside");
			setting_update = mxml_get_bool(data, "setting_update");
			setting_server = mxml_get_bool(data, "setting_server");

			mxmlDelete(data);
			mxmlDelete(tree);

			if (loaded_from == true) {
				printf("Settings loaded from SD card.\n");
			} else {
				printf("Settings loaded from USB device.\n");
			}

			// Double check that setting SD is correct
			if (setting_use_sd == true && sd_mounted == false) {
				setting_use_sd = false;
				printf("Settings say to load from SD, no SD found. "
					   "Using USB instead.\n");
			} else if (setting_use_sd == false && usb_mounted == false) {
				setting_use_sd = true;
				printf("Settings say to load from USB, no USB found. "
					   "Using SD instead.\n");
			}
			printf("\n");
		} else {
			if (loaded_from == true) {
				remove_file("sd:/apps/homebrew_browser/settings.xml");
			} else {
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
	} else {
		strcpy(rootdir, "usb:/");
	}
}

void update_settings() {
	mxml_node_t *xml;
	mxml_node_t *data;

	xml = mxmlNewXML("1.0");

	data = mxmlNewElement(xml, "settings");

	mxml_set_bool(data, "setting_check_size", setting_check_size);
	mxml_set_bool(data, "setting_sd_card", setting_sd_card);
	mxml_set_bool(data, "setting_hide_installed", setting_hide_installed);
	mxml_set_bool(data, "setting_get_rating", setting_get_rating);
	mxml_set_bool(data, "setting_music", setting_music);
	mxml_set_bool(data, "setting_online", setting_online);
	mxml_set_bool(data, "setting_rumble", setting_rumble);
	mxml_set_bool(data, "setting_update_icon", setting_update_icon);
	mxml_set_bool(data, "setting_tool_tip", setting_tool_tip);
	mxml_set_bool(data, "setting_prompt_cancel", setting_prompt_cancel);
	mxml_set_bool(data, "setting_power_off", setting_power_off);

	// We use setting_last_boot as a canary for time-related issues.
	mxmlElementSetAttr(data, "setting_last_boot", setting_last_boot);

	mxml_set_bool(data, "setting_show_updated", setting_show_updated);
	mxml_set_bool(data, "setting_use_sd", setting_use_sd);
	mxml_set_int(data, "setting_repo", setting_repo);
	mxml_set_int(data, "setting_sort", setting_sort);
	mxml_set_int(data, "setting_category", setting_category);
	mxml_set_bool(data, "setting_disusb", setting_disusb);
	mxml_set_bool(data, "setting_dischar", setting_dischar);
	mxml_set_bool(data, "setting_wiiside", setting_wiiside);
	mxml_set_bool(data, "setting_update", setting_update);
	mxml_set_bool(data, "setting_server", setting_server);

	FILE *fp = fopen("sd:/apps/homebrew_browser/settings.xml", "wb");
	FILE *fp1 = fopen("usb:/apps/homebrew_browser/settings.xml", "wb");

	if (fp == NULL) {
		// printf("Settings file not found\n");
	} else {
		mxmlSaveFile(xml, fp, MXML_NO_CALLBACK);
		fclose(fp);
		// printf("saved\n");
	}

	if (fp1 == NULL) {
		// printf("Settings file not found\n");
	} else {
		mxmlSaveFile(xml, fp1, MXML_NO_CALLBACK);
		fclose(fp1);
	}

	mxmlDelete(data);
	mxmlDelete(xml);
}

void load_mount_settings() {
	mxml_node_t *tree;
	mxml_node_t *data;

	FILE *fp = fopen("sd:/apps/homebrew_browser/settings.xml", "rb");

	if (fp == NULL) {
		return;
	}

	fseek(fp, 0, SEEK_END);
	long settings_size = ftell(fp);
	rewind(fp);

	if (settings_size <= 0) {
		fclose(fp);
		return;
	}

	tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
	fclose(fp);

	data = mxmlFindElement(tree, tree, "settings", NULL, NULL, MXML_DESCEND);

	if (mxmlElementGetAttr(data, "setting_disusb")) {
		setting_disusb = atoi(mxmlElementGetAttr(data, "setting_disusb"));
	}

	mxmlDelete(data);
	mxmlDelete(tree);
}