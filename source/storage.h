/* Manager for storage devices in the Homebrew Browser. */
#define METHOD_SD 1
#define METHOD_USB 2

extern bool sd_mounted;
extern bool usb_mounted;

void initialise_fat();