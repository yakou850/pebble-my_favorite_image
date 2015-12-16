#pragma once
#include "netdownload.h"
#include "mainface.h"
#include <string.h>
#ifdef PBL_PLATFORM_APLITE
#include "png.h"
#endif

#define IMAGE_MAX 5
#define UPDATE_INTERVAL 20

#define DATA_UPDATE_INTERVAL 700
#define DATA_CONFIG_ERROR_URL_REMOVE 710
#define DATA_KEY 850
#define DATA_KEY2 860
#define DATA_KEY3 870
#define DATA_KEY4 880
#define DATA_KEY5 890

static Window *window;
static GBitmap *current_bmp;
static bool isReady = false;
static int update_interval = UPDATE_INTERVAL;
static bool error_url_remove = true;

int update_counter = 0;
char *image;
char *image2;
char *image3;
char *image4;
char *image5;

void show_next_image();
void show_error_image();

void set_image_url(char *data, uint number) {
	switch (number) {
		case 0:
		image = malloc(sizeof(char) * (strlen(data)+1));
		strcpy(image, data);
		break;
		case 1:
		image2 = malloc(sizeof(char) * (strlen(data)+1));
		strcpy(image2, data);
		break;
		case 2:
		image3 = malloc(sizeof(char) * (strlen(data)+1));
		strcpy(image3, data);
		break;
		case 3:
		image4 = malloc(sizeof(char) * (strlen(data)+1));
		strcpy(image4, data);
		break;
		case 4:
		image5 = malloc(sizeof(char) * (strlen(data)+1));
		strcpy(image5, data);
		break;
	}
}

static void window_load(Window *window) {
	current_bmp = NULL;
	show_mainface();
}

void read_config() {
	char *url, *url2, *url3, *url4, *url5;
	int size_string, size_string2, size_string3, size_string4, size_string5;

	if (persist_exists(DATA_KEY)) {
		size_string = persist_get_size(DATA_KEY);
		url = malloc(size_string);
		persist_read_string(DATA_KEY, url, size_string);
		set_image_url(url, 0);
	}	
	if (persist_exists(DATA_KEY2)) {
		size_string2 = persist_get_size(DATA_KEY2);
		url2 = malloc(size_string2);
		persist_read_string(DATA_KEY2, url2, size_string2);
		set_image_url(url2, 1);
	}
	if (persist_exists(DATA_KEY3)) {
		size_string3 = persist_get_size(DATA_KEY3);
		url3 = malloc(size_string3);
		persist_read_string(DATA_KEY3, url3, size_string3);
		set_image_url(url3, 2);
	}
	if (persist_exists(DATA_KEY4)) {
		size_string4 = persist_get_size(DATA_KEY4);
		url4 = malloc(size_string4);
		persist_read_string(DATA_KEY4, url4, size_string4);
		set_image_url(url4, 3);
	}
	if (persist_exists(DATA_KEY5)) {
		size_string5 = persist_get_size(DATA_KEY5);
		url5 = malloc(size_string5);
		persist_read_string(DATA_KEY5, url5, size_string5);
		set_image_url(url5, 4);
	}
	if (persist_exists(DATA_UPDATE_INTERVAL)) {
		update_interval = persist_read_int(DATA_UPDATE_INTERVAL);
	}
	if (persist_exists(DATA_CONFIG_ERROR_URL_REMOVE)) {
		error_url_remove = persist_read_bool(DATA_CONFIG_ERROR_URL_REMOVE);
	}
}

void write_config() {
	if (image) {
		persist_write_string(DATA_KEY, image);
	} else {
		persist_write_string(DATA_KEY, "");
	}
	if (image2) {
		persist_write_string(DATA_KEY2, image2);
	} else {
		persist_write_string(DATA_KEY2, "");
	}
	if (image3) {
		persist_write_string(DATA_KEY3, image3);
	} else {
		persist_write_string(DATA_KEY3, "");
	}
	if (image4) {
		persist_write_string(DATA_KEY4, image4);
	} else {
		persist_write_string(DATA_KEY4, "");
	}
	if (image5) {
		persist_write_string(DATA_KEY5, image5);
	} else {
		persist_write_string(DATA_KEY5, "");
	}
	persist_write_int(DATA_UPDATE_INTERVAL, update_interval);
	persist_write_bool(DATA_CONFIG_ERROR_URL_REMOVE, error_url_remove);
}

static void window_unload(Window *window) {
	hide_mainface();
	gbitmap_destroy(current_bmp);
}

void download_complete_handler(NetDownload *download) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Heap Used: %d, Free: %d ", heap_bytes_used(), heap_bytes_free());
	if (current_bmp) {
		gbitmap_destroy(current_bmp);
		current_bmp = NULL;
	}
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Heap Used: %d, Free: %d ", heap_bytes_used(), heap_bytes_free());


	#ifdef PBL_PLATFORM_APLITE
	GBitmap *bmp = gbitmap_create_with_png_data(download->data, download->length);
	#else
	GBitmap *bmp = gbitmap_create_from_png_data(download->data, download->length);
	#endif

	if (!bmp) {
		show_error_image();
		return;
	}
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Heap Used: %d, Free: %d ", heap_bytes_used(), heap_bytes_free());
	set_bitmap_data(bmp);

	// Save pointer to currently shown bitmap (to free it)
	current_bmp = bmp;

	// Free the memory now
	#ifdef PBL_PLATFORM_APLITE
	// gbitmap_create_with_png_data will free download->data
	#else
	free(download->data);
	#endif
	// We null it out now to avoid a double free
	download->data = NULL;
	netdownload_destroy(download);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Heap Used: %d, Free: %d ", heap_bytes_used(), heap_bytes_free());
}

void download_ready_handler() {
	isReady = true;
	show_next_image();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "will show image");
}

void download_error_handler() {
	show_error_image();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "download image error");
}

void set_image_url_handler(char *data, uint number);

void set_update_interval_handler(uint8_t interval, bool flag) {
	update_interval = (int)interval;
	error_url_remove = flag;
}

void tick_image_handler(struct tm *tick_time, TimeUnits units_changed) {	
	update_time();

	update_counter++;
	if (update_counter >= update_interval) {
		show_next_image();
		update_counter = 0;
	}
}

static void init(void) {
	// Need to initialize this first to make sure it is there when
	// the window_load function is called by window_stack_push.
	netdownload_initialize(download_complete_handler, download_ready_handler, download_error_handler, set_image_url_handler, set_update_interval_handler);

	window = window_create();
	#ifdef PBL_SDK_2
	window_set_fullscreen(window, true);
	#endif
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	const bool animated = true;
	window_stack_push(window, animated);
	read_config();

	tick_timer_service_subscribe(MINUTE_UNIT, tick_image_handler);
}

static void deinit(void) {
	netdownload_deinitialize(); // call this to avoid 20B memory leak
	window_destroy(window);
	write_config();
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();
	deinit();
}