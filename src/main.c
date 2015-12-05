#include <pebble.h>
#include "main.h"

static uint image = 0;

void show_next_image() {
	uint tryCount = 0;
	// Unload the current image if we had one and save a pointer to this one
	image++;
	while (strlen(images[image])==0) {
		printf("#%d is empty", image);
		image++;
		if (image >= sizeof(images)/sizeof(char*)) {
			image = 0;
		}
		tryCount++;
		if (tryCount == sizeof(images)/sizeof(char*)) {
			break;
		}
	}
	if (current_bmp) {
		gbitmap_destroy(current_bmp);
		current_bmp = NULL;
	}

	printf("#%d: %s", image, images[image]);
	netdownload_request(images[image]);
}

void show_error_image() {
	printf("download error: %d", image);
}

void set_image_url_handler(char *data, uint number) {
	printf("Get string(%d): %s", number, data);
	set_image_url(data, number);
	if (number == 0) {
		image = 0;
		show_next_image();
	}
}