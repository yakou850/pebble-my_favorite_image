#include <pebble.h>
#include "main.h"

int image_counter = 0;

void show_next_image() {
	if (image_counter == 0) {
		netdownload_request(image);
		image_counter++;
	} else if (image_counter == 1) {
		netdownload_request(image2);
		image_counter++;
	} else if (image_counter == 2) {
		netdownload_request(image3);
		image_counter++;
	} else if (image_counter == 3) {
		netdownload_request(image4);
		image_counter++;
	} else if (image_counter == 4) {
		netdownload_request(image5);
		image_counter++;
	}

	if (image_counter >= IMAGE_MAX) {
		image_counter = 0;
	}
}

void show_error_image() {
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
	if (number == 0) {
		//		show_next_image();
	}
}