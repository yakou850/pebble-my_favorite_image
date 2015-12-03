#include <pebble.h>
#include "main.h"

static char *images[] = {
	"http://850.jp/haruna.png"
};

void show_next_image() {
	// Unload the current image if we had one and save a pointer to this one
	if (current_bmp) {
		gbitmap_destroy(current_bmp);
		current_bmp = NULL;
	}

	netdownload_request(images[image]);

	image++;
	if (image >= sizeof(images)/sizeof(char*)) {
		image = 0;
	}
}

void show_error_image() {
	
}