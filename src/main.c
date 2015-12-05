#include <pebble.h>
#include "main.h"
void show_next_image() {
	if (current_bmp) {
		gbitmap_destroy(current_bmp);
		current_bmp = NULL;
	}

	printf("%s", image);
	netdownload_request(image);
}

void show_error_image() {
}

void set_image_url_handler(char *data, uint number) {
	printf("Get string: %s", data);
	set_image_url(data, number);
	write_config();
	if (number == 0) {
		show_next_image();
	}
}