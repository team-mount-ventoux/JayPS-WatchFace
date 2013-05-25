#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include <stdint.h>
#include <string.h>

// 5DD35873-3BB6-44D6-8255-0E61BC3B97F5
#define MY_UUID { 0x5D, 0xD3, 0x58, 0x73, 0x3B, 0xB6, 0x44, 0xD6, 0x82, 0x55, 0x0E, 0x61, 0xBC, 0x3B, 0x97, 0xF5 }
PBL_APP_INFO(MY_UUID,
             "Pebble Cycling", "N Jackson",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_STANDARD_APP);

enum {
  SPEED_TEXT = 0x0,     // TUPLE_CSTR
  DISTANCE_TEXT = 0x1,  // TUPLE_CSTR
  AVGSPEED_TEXT = 0x2,  // TUPLE_CSTR
};

#define NUMBER_OF_IMAGES 11
#define TOTAL_IMAGE_SLOTS 4
#define EMPTY_SLOT -1
#define CHAR_WIDTH 35
#define DOT_WIDTH 15
#define CHAR_HEIGHT 51

#define CANVAS_WIDTH 144

const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_IMAGE_NUM_0, RESOURCE_ID_IMAGE_NUM_1, RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3, RESOURCE_ID_IMAGE_NUM_4, RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6, RESOURCE_ID_IMAGE_NUM_7, RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9,RESOURCE_ID_IMAGE_NUM_DOT
};

BmpContainer image_containers[TOTAL_IMAGE_SLOTS];
int image_slot_state[TOTAL_IMAGE_SLOTS] = {EMPTY_SLOT, EMPTY_SLOT, EMPTY_SLOT, EMPTY_SLOT};

TextLayer mph_layer;
TextLayer distance_layer;
TextLayer miles_layer;
TextLayer avg_layer;
TextLayer avgmph_layer;

Layer line_layer;
Layer bottom_layer;

static struct AppData {
  Window window;
  TextLayer speed_layer;
  TextLayer distance_layer;
  TextLayer avgspeed_layer;
  char speed[16];
  char distance[16];
  char avgspeed[16];
  AppSync sync;
  uint8_t sync_buffer[128];
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
    break;
  case DISTANCE_TEXT:
    strncpy(s_data.distance, new_tuple->value->cstring, 16);
    break;
  case AVGSPEED_TEXT:
    strncpy(s_data.avgspeed, new_tuple->value->cstring, 16);
    break;
  default:
    return;
  }

}

void show_speed(char* speed) {

  // clean up memory
  for(int n=0; n < TOTAL_IMAGE_SLOTS; n++) {
    if(image_slot_state[n] != EMPTY_SLOT) {
      layer_remove_from_parent(&image_containers[n].layer.layer);
      bmp_deinit_container(&image_containers[n]);
      image_slot_state[n] = EMPTY_SLOT;
    }
  }

  int len = strlen(speed);
  // get the size
  int size = 0;
  if(len > 1)
    size = (len * CHAR_WIDTH) - 20; // dot is 20 pixels smaller
  else if(len ==1)
    size = CHAR_WIDTH;

  int leftpos = (CANVAS_WIDTH - size) / 2;


  for(int c=0; c < len; c++) {

    int digit_value = -1;
    if(speed[c] == '.') {
      digit_value = 10;
    } else {
      digit_value = speed[c] - '0';
    }

    bmp_init_container(IMAGE_RESOURCE_IDS[digit_value], &image_containers[c]);
    image_containers[c].layer.layer.frame.origin.x = leftpos;
    image_containers[c].layer.layer.frame.origin.y = 10;
    layer_add_child(&s_data.window.layer, &image_containers[c].layer.layer);
    leftpos += (digit_value == 10) ? DOT_WIDTH : CHAR_WIDTH;
    image_slot_state[c] = 1;

  }

}

void line_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, GPoint(72, 90), GPoint(72, 160));
}

void botom_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  //GFont font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_24));
  graphics_context_set_fill_color(app_get_current_graphics_context(), GColorWhite);
  graphics_fill_rect(app_get_current_graphics_context(), GRect(0,85,CANVAS_WIDTH,84), 0, GCornerNone);
  
  //GFont font_12 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_12));
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  resource_init_current_app(&APP_RESOURCES);

  GFont font_12 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_12));
  GFont font_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_18));
  GFont font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_24));
  
  Window* window = &s_data.window;
  window_init(window, "Pebble Cycling");
  window_set_background_color(&s_data.window, GColorBlack);
  window_set_fullscreen(&s_data.window, true); 


  layer_init(&bottom_layer, window->layer.frame);
  bottom_layer.update_proc = &botom_layer_update_callback;
  layer_add_child(&window->layer, &bottom_layer);


  text_layer_init(&mph_layer, GRect(0, 60, CANVAS_WIDTH, 21));
  text_layer_set_text(&mph_layer, "mph");
  text_layer_set_text_color(&mph_layer, GColorWhite);
  text_layer_set_background_color(&mph_layer, GColorClear);
  text_layer_set_font(&mph_layer, font_18);
  text_layer_set_text_alignment(&mph_layer, GTextAlignmentCenter);
  layer_add_child(&window->layer, &mph_layer.layer);


  text_layer_init(&distance_layer, GRect(2, 97, 66, 14));
  text_layer_set_text(&distance_layer, "distance");
  text_layer_set_text_color(&distance_layer, GColorBlack);
  text_layer_set_background_color(&distance_layer, GColorClear);
  text_layer_set_font(&distance_layer, font_12);
  text_layer_set_text_alignment(&distance_layer, GTextAlignmentCenter);
  layer_add_child(&window->layer, &distance_layer.layer);

  
  text_layer_init(&miles_layer, GRect(2, 148, 66, 14));
  text_layer_set_text(&miles_layer, "miles");
  text_layer_set_text_color(&miles_layer, GColorBlack);
  text_layer_set_background_color(&miles_layer, GColorClear);
  text_layer_set_font(&miles_layer, font_12);
  text_layer_set_text_alignment(&miles_layer, GTextAlignmentCenter);
  layer_add_child(&window->layer, &miles_layer.layer);


  text_layer_init(&avg_layer, GRect(75, 90, 66, 28));
  text_layer_set_text(&avg_layer, "average speed");
  text_layer_set_text_color(&avg_layer, GColorBlack);
  text_layer_set_background_color(&avg_layer, GColorClear);
  text_layer_set_font(&avg_layer, font_12);
  text_layer_set_text_alignment(&avg_layer, GTextAlignmentCenter);
  layer_add_child(&window->layer, &avg_layer.layer);


  text_layer_init(&avgmph_layer, GRect(75, 148, 66, 14));
  text_layer_set_text(&avgmph_layer, "mph");
  text_layer_set_text_color(&avgmph_layer, GColorBlack);
  text_layer_set_background_color(&avgmph_layer, GColorClear);
  text_layer_set_font(&avgmph_layer, font_12);
  text_layer_set_text_alignment(&avgmph_layer, GTextAlignmentCenter);
  layer_add_child(&window->layer, &avgmph_layer.layer);


  layer_init(&line_layer, window->layer.frame);
  line_layer.update_proc = &line_layer_update_callback;
  layer_add_child(&window->layer, &line_layer);
  

  text_layer_init(&s_data.distance_layer, GRect(2, 116, 66, 32));
  text_layer_set_text_color(&s_data.distance_layer, GColorBlack);
  text_layer_set_background_color(&s_data.distance_layer, GColorClear);
  text_layer_set_font(&s_data.distance_layer, font_24);
  text_layer_set_text_alignment(&s_data.distance_layer, GTextAlignmentCenter);
  text_layer_set_text(&s_data.distance_layer, s_data.distance);
  layer_add_child(&window->layer, &s_data.distance_layer.layer);


  text_layer_init(&s_data.avgspeed_layer, GRect(74, 116, 66, 32));
  text_layer_set_text_color(&s_data.avgspeed_layer, GColorBlack);
  text_layer_set_background_color(&s_data.avgspeed_layer, GColorClear);
  text_layer_set_font(&s_data.avgspeed_layer, font_24);
  text_layer_set_text_alignment(&s_data.avgspeed_layer, GTextAlignmentCenter);
  text_layer_set_text(&s_data.avgspeed_layer, s_data.avgspeed);
  layer_add_child(&window->layer, &s_data.avgspeed_layer.layer);
  

  Tuplet initial_values[] = {
    TupletCString(SPEED_TEXT, "0.0"),
    TupletCString(DISTANCE_TEXT, "0.0"),
    TupletCString(AVGSPEED_TEXT, "0.0"),
  };

  app_sync_init(&s_data.sync, s_data.sync_buffer, sizeof(s_data.sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
                sync_tuple_changed_callback, sync_error_callback, NULL);
  
  window_stack_push(window, true /* Animated */);

}

static void handle_deinit(AppContextRef c) {
   app_sync_deinit(&s_data.sync);
}

void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)t;
  (void)ctx;

  show_speed(s_data.speed);
  layer_mark_dirty(&s_data.distance_layer.layer);
  layer_mark_dirty(&s_data.avgspeed_layer.layer);
}

/*
    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = SECOND_UNIT
    },*/

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = SECOND_UNIT
    },
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 128,
        .outbound = 16,
      }
    }
  };
  app_event_loop(params, &handlers);
}
