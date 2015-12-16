#include <pebble.h>
#include "main.h"

int image_counter = 0;

void show_next_image() {
	char *image_current;
	for (int i=0; i< IMAGE_MAX; i++) {

		if (image_counter == 0) {
			image_current = image;
		} else if (image_counter == 1) {
			image_current = image2;
		} else if (image_counter == 2) {
			image_current = image3;
		} else if (image_counter == 3) {
			image_current = image4;		
		} else if (image_counter == 4) {
			image_current = image5;
		}
		image_counter++;
		if (image_counter >= IMAGE_MAX) {
			image_counter = 0;
		}
		if (image_current && strlen(image_current) > 0) {
			break;
		}
	}
	if (image_current && strlen(image_current) > 0) {
		netdownload_request(image_current);
	}
}

void show_error_image() {
	if (!error_url_remove) {
		return;
	}
	switch (image_counter) {
		case 1: image = NULL; break;
		case 2: image2 = NULL; break;
		case 3: image3 = NULL; break;
		case 4: image4 = NULL; break;
		case 5: image5 = NULL; break;
	}

}

void set_image_url_handler(char *data, uint number) {
	set_image_url(data, number);
	write_config();
}