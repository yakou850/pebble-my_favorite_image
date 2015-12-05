#pragma once
#include "netdownload.h"
#include "mainface.h"
#include <string.h>
#ifdef PBL_PLATFORM_APLITE
#include "png.h"
#endif


static Window *window;
static GBitmap *current_bmp;
static char *images[] = {"","","","",""};
void show_next_image();
void show_error_image();

void set_image_url(char *data, uint number) {
	strcpy(images[number],data);
}

static void window_load(Window *window) {
	current_bmp = NULL;
	show_mainface();
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
}

static void deinit(void) {
	netdownload_deinitialize(); // call this to avoid 20B memory leak
	window_destroy(window);
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();
	deinit();
}