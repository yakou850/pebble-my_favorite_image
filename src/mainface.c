#include <pebble.h>
#include "mainface.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_gothic_14;
static GFont s_res_gothic_28_bold;
static BitmapLayer *image_layer;
static TextLayer *day_layer;
static TextLayer *weekday_layer;
static TextLayer *clock_layer;

static void initialise_ui(void) {
	s_window = window_create();
	#ifndef PBL_SDK_3
	window_set_fullscreen(s_window, 1);
	#endif
	#ifdef PBL_COLOR
	window_set_background_color(s_window, GColorCyan);
	#endif

	s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
	s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
	// image_layer
	image_layer = bitmap_layer_create(GRect(2, 1, 140, 140));
	bitmap_layer_set_background_color(image_layer, GColorBlack);
	layer_add_child(window_get_root_layer(s_window), (Layer *)image_layer);

	// day_layer
	day_layer = text_layer_create(GRect(5, 144, 32, 14));
	text_layer_set_background_color(day_layer, GColorClear);
	text_layer_set_text(day_layer, "12/03");
	text_layer_set_font(day_layer, s_res_gothic_14);
	layer_add_child(window_get_root_layer(s_window), (Layer *)day_layer);

	// weekday_layer
	weekday_layer = text_layer_create(GRect(40, 144, 23, 14));
	text_layer_set_background_color(weekday_layer, GColorClear);
	text_layer_set_text(weekday_layer, "Tue");
	text_layer_set_font(weekday_layer, s_res_gothic_14);
	layer_add_child(window_get_root_layer(s_window), (Layer *)weekday_layer);

	// clock_layer
	clock_layer = text_layer_create(GRect(85, 137, 51, 28));
	text_layer_set_background_color(clock_layer, GColorClear);
	text_layer_set_text(clock_layer, "00:00");
	text_layer_set_text_alignment(clock_layer, GTextAlignmentRight);
	text_layer_set_font(clock_layer, s_res_gothic_28_bold);
	layer_add_child(window_get_root_layer(s_window), (Layer *)clock_layer);
}

static void destroy_ui(void) {
	window_destroy(s_window);
	bitmap_layer_destroy(image_layer);
	text_layer_destroy(day_layer);
	text_layer_destroy(weekday_layer);
	text_layer_destroy(clock_layer);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
	destroy_ui();
}


void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

void show_mainface(void) {
	initialise_ui();
	window_set_window_handlers(s_window, (WindowHandlers) {
		.unload = handle_window_unload,
	});
	window_stack_push(s_window, true);

	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	update_time();
}

void hide_mainface(void) {
	window_stack_remove(s_window, true);
}

void set_bitmap_data(GBitmap *bitmap) {
	bitmap_layer_set_bitmap(image_layer, bitmap);
}

void update_time(void) {
	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);

	static char buffer_day[] = "00/00";
	strftime(buffer_day, sizeof("00/00"), "%m/%d", tick_time);
	text_layer_set_text(day_layer, buffer_day);

	static char buffer_weekday[] = "Mon";
	strftime(buffer_weekday, sizeof("Mon"), "%a", tick_time);
	text_layer_set_text(weekday_layer, buffer_weekday);

	// Create a long-lived buffer
	static char buffer[] = "00:00";
	if(clock_is_24h_style() == true) {
		// Use 24 hour format
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	} else {
		// Use 12 hour format
		strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
	}
	text_layer_set_text(clock_layer, buffer);
}

