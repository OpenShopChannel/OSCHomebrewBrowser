/* Manager for storage devices in the Homebrew Browser. */
#include <fat.h>
#include <stdio.h>
#include <stdbool.h>

#include <sys/dir.h>
#include <sys/unistd.h>

#include <sdcard/wiisd_io.h>
#include <ogc/usbstorage.h>

#include "common.h"
#include "settings.h"
#include "storage.h"

#pragma mark - FAT filesystem functions

// Used to store things such as "sd:/" or "usb:/".
char rootdir[10];

bool sd_mounted = false;
bool usb_mounted = false;

// Determine whether we can open a folder on
// the given FAT media.
static bool can_open_root_fs() {
	DIR *root;
	root = opendir(rootdir);
	if (root) {
		closedir(root);
		return true;
	}
	return false;
}

// Ensure that we can read and write on the given FAT media.
bool test_fat() {
	// Try to open root filesystem - if we don't check here, mkdir crashes later
	if (!can_open_root_fs()) {
		printf("Unable to open root filesystem of %s\n", rootdir);
		return false;
	}

	// Change dir
	if (chdir(rootdir)) {
		printf("Could not change to root directory to %s\n", rootdir);
		return false;
	}

	// Create directories
	char dir_apps[50];
	char dir_hbb[150];
	char dir_hbbtemp[150];

	strcpy(dir_apps, rootdir);
	strcat(dir_apps, "apps");
	strcpy(dir_hbb, rootdir);
	strcat(dir_hbb, "apps/homebrew_browser");
	strcpy(dir_hbbtemp, rootdir);
	strcat(dir_hbbtemp, "apps/homebrew_browser/temp");

	if (!opendir(dir_apps)) {
		mkdir(dir_apps, 0777);
		if (!opendir(dir_apps)) {
			printf("Could not create %s directory.\n", dir_apps);
		}
	}


	if (!opendir(dir_hbb)) {
		mkdir(dir_hbb, 0777);
		if (!opendir(dir_hbb)) {
			printf("Could not create %s directory.\n", dir_hbb);
		}
	}

	if (!opendir(dir_hbbtemp)) {
		mkdir(dir_hbbtemp, 0777);
		if (!opendir(dir_hbbtemp)) {
			printf("Could not create %s directory.\n", dir_hbbtemp);
		}
	}

	return true;
}

#pragma mark - Device initialisation

const DISC_INTERFACE* sd = &__io_wiisd;
const DISC_INTERFACE* usb = &__io_usbstorage;

bool initialise_device(int method) {
	bool mounted = false;
	char name[10];
	const DISC_INTERFACE* disc = NULL;

	switch(method)
	{
			case METHOD_SD:
					sprintf(name, "sd");
					disc = sd;
					break;
			case METHOD_USB:
					sprintf(name, "usb");
					disc = usb;
					break;
	}

	if (disc->startup() && fatMountSimple(name, disc)) {
		mounted = true;
	} else {
		printf("Unable to mount %s...\n", name);
	}

	return mounted;
}

void initialise_fat() {
	bool fat_init = false;

	// At least one FAT initialisation must be completed.
	printf("Attempting to mount SD card... ");
	if (initialise_device(METHOD_SD)) {
		strcpy(rootdir, "sd:/");
		if (test_fat() == true) {
			fat_init = true;
			sd_mounted = true;
			printf("SD card mounted.\n");
			load_mount_settings();
		} else {
			fatUnmount("sd:");
			sleep(1);
		}
	}
	
  if (setting_disusb == false) {
		printf("Attempting to mount USB device... ");
		if (initialise_device(METHOD_USB)) {
			strcpy(rootdir, "usb:/");
			if (test_fat() == true) {
				fat_init = true;
				usb_mounted = true;
				printf("USB device mounted.\n");
			}
			else {
				fatUnmount("usb:");
				sleep(1);
			}
		}
	}

	if (!fat_init) {
		die("Could not mount SD card or USB device...");
	}
}
