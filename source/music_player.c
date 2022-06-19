/* Manages music for the Homebrew Browser. */
#include "common.h"
#include "files.h"
#include "mp3player.h"
#include "oggplayer.h"
#include <ogcsys.h>
#include <unistd.h>

static lwp_t music_thread;

// Whether to stop playing MP3 music.
// To stop playing OGG files, use the "StopOgg" function.
bool stop_mp3_music = false;

static void *run_music_thread(void *arg) {
	FILE *f = hbb_fopen("loop.mp3", "rb");
	if (f == NULL) {
		return 0;
	}

	fseek(f, 0, SEEK_END);
	long mp3_size = ftell(f);
	rewind(f);

	// allocate memory to contain the whole file:
	char *buffer = (char *)malloc(sizeof(char) * mp3_size);
	if (buffer == NULL) {
		perror("   Memory error");
	}

	// copy the file into the buffer:
	size_t result = fread(buffer, 1, mp3_size, f);
	if (result != mp3_size) {
		perror("   Reading error");
	}

	fclose(f);

	while (1) {
		if (!MP3Player_IsPlaying()) {
			MP3Player_PlayBuffer(buffer, mp3_size + 29200, NULL);
		}
		if (exiting == true || stop_mp3_music == true) {
			MP3Player_Stop();
			stop_mp3_music = false;
			break;
		}
		usleep(1000);
	}

	return 0;
}

s32 initialise_music() {
	s32 result =
		LWP_CreateThread(&music_thread, run_music_thread, NULL, NULL, 0, 80);
	return result;
}

void initialise_mod_music() {
	FILE *f = hbb_fopen("loop.mod", "rb");

	if (f == NULL) {
		return;
	}

	fseek(f, 0, SEEK_END);
	long mod_size = ftell(f);
	rewind(f);

	// allocate memory to contain the whole file:
	char *buffer = (char *)malloc(sizeof(char) * mod_size);
	if (buffer == NULL) {
		fputs("   Memory error", stderr);
	}

	// copy the file into the buffer:
	size_t result = fread(buffer, 1, mod_size, f);
	if (result != mod_size) {
		fputs("   Reading error", stderr);
	}

	fclose(f);
}

void play_mod_music() {
	FILE *f;

	// MP3
	f = hbb_fopen("loop.mp3", "rb");
	if (f != NULL) {
		fclose(f);
		initialise_music();
		return;
	}

	// If there's no MP3, attempt OGG.
	f = hbb_fopen("loop.ogg", "rb");
	if (f != NULL) {
		fseek(f, 0, SEEK_END);
		long ogg_size = ftell(f);
		rewind(f);

		// allocate memory to contain the whole file:
		char *buffer = (char *)malloc(sizeof(char) * ogg_size);
		if (buffer == NULL) {
			perror("   Memory error");
		}

		// copy the file into the buffer:
		size_t result = fread(buffer, 1, ogg_size, f);
		if (result != ogg_size) {
			perror("   Reading error");
		}

		PlayOgg(buffer, ogg_size, 0, OGG_INFINITE_TIME);
		fclose(f);
		return;
	}

	// Lastly, attempt MOD.
	f = hbb_fopen("loop.mod", "rb");
	if (f != NULL) {
		fclose(f);
		initialise_mod_music();
	}
}

void stop_mod_music() {
	stop_mp3_music = true;
	StopOgg();
}