#pragma once
#include "netdownload.h"
#include "mainface.h"
#include <string.h>
#ifdef PBL_PLATFORM_APLITE
#include "png.h"
#endif

#define DATA_KEY 850
#define DATA_SIZE 849

static Window *window;
static GBitmap *current_bmp;
static char *images[] = {"","","","",""};
void show_next_image();
void show_error_image();

void set_image_url(char *data, uint number) {
	images[number] = malloc(sizeof(data));
	strcpy(images[number], data);
}

static void window_load(Window *window) {
	current_bmp = NULL;
	show_mainface();
}

void read_config() {
	char *url;
	uint i = 0;
	uint size = persist_read_int(DATA_SIZE);
	int size_string;

	for (i = 0; i < size; i++) {
		if (persist_exists(DATA_KEY + i)) {
			size_string = persist_get_size(DATA_KEY + i);
			url = malloc(size_string);
			persist_read_string(DATA_KEY + i, url, size_string);
			set_image_url(url, i);
			printf("%d: %s read", i, images[i]);
		}		
	}
}

void write_config() {
	uint i = 0;
	for (i = 0; i < sizeof(images)/sizeof(char*); i++) {
		persist_write_string(DATA_KEY + i, images[i]);
		printf("%d: %s write", i, images[i]);
	}
	persist_write_int(DATA_SIZE, sizeof(images)/sizeof(char*));

}

static void window_unload(Window *window) {
	hide_mainface();
	gbitmap_destroy(current_bmp);
}

void download_complete_handler(NetDownload *download) {

	#ifdef PBL_PLATFORM_APLITE
	GBitmap *bmp = gbitmap_create_with_png_data(download->data, download->length);
	#else
	GBitmap *bmp = gbitmap_create_from_png_data(download->data, download->length);
	#endif
	set_bitmap_data(bmp);

	// Save pointer to currently shown bitmap (to free it)
	if (current_bmp) {
		gbitmap_destroy(current_bmp);
	}
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

	APP_LOG(APP_LOG_LEVEL_DEBUG, "show image");
}

void download_ready_handler() {
	show_next_image();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "will show image");
}

void download_error_handler() {
	show_error_image();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "download image error");
}

void set_image_url_handler(char *data, uint number);

static void init(void) {
	// Need to initialize this first to make sure it is there when
	// the window_load function is called by window_stack_push.
	netdownload_initialize(download_complete_handler, download_ready_handler, download_error_handler, set_image_url_handler);

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