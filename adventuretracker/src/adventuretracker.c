#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include <stdint.h>
#include <string.h>

// 5DD35873-3BB6-44D6-8255-0E61BC3B97F5
#define MY_UUID { 0x5D, 0xD3, 0x58, 0x73, 0x3B, 0xB6, 0x44, 0xD6, 0x82, 0x55, 0x0E, 0x61, 0xBC, 0x3B, 0x97, 0xF5 }
PBL_APP_INFO(MY_UUID,
             "Adventure Tracker App", "AdventureTracker",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

enum {
  SPEED_TEXT = 0x0,     // TUPLE_CSTR
  DISTANCE_TEXT = 0x1,  // TUPLE_CSTR
  AVGSPEED_TEXT = 0x2,  // TUPLE_CSTR
};

static struct AppData {
  Window window;
  TextLayer speed_layer;
  TextLayer distance_layer;
  TextLayer avgspeed_layer;
  char speed[16];
  char distance[16];
  char avgspeed[16];
  AppSync sync;
  uint8_t sync_buffer[32];
} s_data;

// TODO: Error handling
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  (void) dict_error;
  (void) app_message_error;
  (void) context;
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  (void) old_tuple;

  switch (key) {
  case SPEED_TEXT:
    strncpy(s_data.speed, new_tuple->value->cstring, 16);
    layer_mark_dirty(&s_data.speed_layer.layer);
    break;
  case DISTANCE_TEXT:
    strncpy(s_data.distance, new_tuple->value->cstring, 16);
    layer_mark_dirty(&s_data.distance_layer.layer);
    break;
  case AVGSPEED_TEXT:
    strncpy(s_data.avgspeed, new_tuple->value->cstring, 16);
    layer_mark_dirty(&s_data.avgspeed_layer.layer);
    break;
  default:
    return;
  }

}

void update_layer_callback(Layer *me, GContext* ctx) {
  (void)me;

  graphics_context_set_text_color(ctx, GColorWhite);

  // draw the speed
  graphics_text_draw(ctx,
         "23",
         fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
         GRect(5, 5, 144-10, 100),
         GTextOverflowModeWordWrap,
         GTextAlignmentCenter,
         NULL);
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  Window* window = &s_data.window;

  window_init(window, "Adventure Tracker");

  window_set_background_color(&s_data.window, GColorBlack);
  window_set_fullscreen(&s_data.window, true);

  //layer_init(&layer, window.layer.frame);
  //layer.update_proc = update_layer_callback;
  //layer_add_child(&window.layer, &layer);

  Tuplet initial_values[] = {
    TupletCString(SPEED_TEXT, "0"),
    TupletCString(DISTANCE_TEXT, "0"),
    TupletCString(AVGSPEED_TEXT, "0"),
  };

  text_layer_init(&s_data.speed_layer, GRect(28, 10, 84, 76));
  text_layer_set_text_color(&s_data.speed_layer, GColorWhite);
  text_layer_set_background_color(&s_data.speed_layer, GColorClear);
  text_layer_set_font(&s_data.speed_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(&s_data.speed_layer, GTextAlignmentCenter);
  text_layer_set_text(&s_data.speed_layer, s_data.speed);
  layer_add_child(&window->layer, &s_data.speed_layer.layer);

  text_layer_init(&s_data.distance_layer, GRect(10, 116, 48, 28));
  text_layer_set_text_color(&s_data.distance_layer, GColorWhite);
  text_layer_set_background_color(&s_data.distance_layer, GColorClear);
  text_layer_set_font(&s_data.distance_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(&s_data.distance_layer, GTextAlignmentCenter);
  text_layer_set_text(&s_data.distance_layer, s_data.distance);
  layer_add_child(&window->layer, &s_data.distance_layer.layer);

  text_layer_init(&s_data.avgspeed_layer, GRect(80, 116, 48, 28));
  text_layer_set_text_color(&s_data.avgspeed_layer, GColorWhite);
  text_layer_set_background_color(&s_data.avgspeed_layer, GColorClear);
  text_layer_set_font(&s_data.avgspeed_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(&s_data.avgspeed_layer, GTextAlignmentCenter);
  text_layer_set_text(&s_data.avgspeed_layer, s_data.avgspeed);
  layer_add_child(&window->layer, &s_data.avgspeed_layer.layer);

  app_sync_init(&s_data.sync, s_data.sync_buffer, sizeof(s_data.sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
                sync_tuple_changed_callback, sync_error_callback, NULL);

  window_stack_push(window, true /* Animated */);

  //layer_mark_dirty(&layer);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init
  };
  app_event_loop(params, &handlers);
}
