#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include <stdint.h>
#include <string.h>

// 5DD35873-3BB6-44D6-8255-0E61BC3B97F5
#define MY_UUID { 0x5D, 0xD3, 0x58, 0x73, 0x3B, 0xB6, 0x44, 0xD6, 0x82, 0x55, 0x0E, 0x61, 0xBC, 0x3B, 0x97, 0xF5 }
PBL_APP_INFO(MY_UUID,
             "Pebble Bike 1.2", "N Jackson",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_STANDARD_APP);

enum {
  STATE_CHANGED =0x0,
  SPEED_TEXT = 0x1,     // TUPLE_CSTR
  DISTANCE_TEXT = 0x2,  // TUPLE_CSTR
  AVGSPEED_TEXT = 0x3,  // TUPLE_CSTR
  MEASUREMENT_UNITS = 0x4 // TUPLE_INT
};

enum {
  STATE_START = 1,
  STATE_STOP = 2,
};

enum {
  PLAY_PRESS =0x0,
  STOP_PRESS = 0x1,
  REFRESH_PRESS = 0x2,
};

enum {
  UNITS_IMPERIAL = 0x0,
  UNITS_METRIC = 0x1,
};

void handle_init(AppContextRef ctx) {
  (void)ctx;
}

void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)t;
  (void)ctx;
  //update_buttons(s_data.state);
  //layer_mark_dirty(&s_data.speed_layer.layer);
  //layer_mark_dirty(&s_data.distance_layer.layer);
  //layer_mark_dirty(&s_data.avgspeed_layer.layer);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = SECOND_UNIT
    },
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 124,
        .outbound = 16,
      }
    }
  };
  app_event_loop(params, &handlers);
}
