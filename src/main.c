#include <pebble.h>
#include "main.h"

void show_next_image() {
	netdownload_request(image);
}

void show_error_image() {
	printf("download error");
}

void set_image_url_handler(char *data, uint number) {
	printf("Get string: %s", data);
	set_image_url(data, number);
	write_config();
	if (number == 0) {
		show_next_image();
	}
}