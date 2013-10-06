#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "pebblebike.h"
#include "communication.h"
#include "buttons.h"
#include "screens.h"
#include "screen_speed.h"
#include "screen_altitude.h"
#include "screen_live.h"
#include "screen_map.h"
#if DEBUG
#include "screen_debug.h"
#endif

PBL_APP_INFO(MY_UUID,
     APP_NAME, APP_COMPANY,
     VERSION_MAJOR, VERSION_MINOR, /* App version */
     RESOURCE_ID_IMAGE_MENU_ICON,
     APP_INFO_STANDARD_APP);

#if ROCKSHOT
#include "rockshot.h"
#endif

GFont font_12, font_18, font_24;

AppData s_data;
GPSData s_gpsdata;
LiveData s_live;
int nbchange_state=0;

void change_units(uint8_t units, bool force) {
  if ((units == s_gpsdata.units) && !force) {
    return;
  }
  s_gpsdata.units = units;
  if (s_gpsdata.units == UNITS_METRIC) {
    strncpy(s_data.unitsSpeed, SPEED_UNIT_METRIC, 8);
    strncpy(s_data.unitsDistance, DISTANCE_UNIT_METRIC, 8);
    strncpy(s_data.altitude_layer.units, ALTITUDE_UNIT_METRIC, 8);
    strncpy(s_data.altitude_ascent.units, ALTITUDE_UNIT_METRIC, 8);
    strncpy(s_data.altitude_ascent_rate.units, ASCENT_RATE_UNIT_METRIC, 8);
  } else {
    strncpy(s_data.unitsSpeed, SPEED_UNIT_IMPERIAL, 8);
    strncpy(s_data.unitsDistance, DISTANCE_UNIT_IMPERIAL, 8);
    strncpy(s_data.altitude_layer.units, ALTITUDE_UNIT_IMPERIAL, 8);
    strncpy(s_data.altitude_ascent.units, ALTITUDE_UNIT_IMPERIAL, 8);
    strncpy(s_data.altitude_ascent_rate.units, ASCENT_RATE_UNIT_IMPERIAL, 8);
  }
  layer_mark_dirty(&s_data.miles_layer.layer);
  layer_mark_dirty(&s_data.mph_layer.layer);
  layer_mark_dirty(&s_data.avgmph_layer.layer);
}

void change_state(uint8_t state) {
  if (state == s_data.state) {
    return;
  }
  s_data.state = state;

  buttons_update();
  
  nbchange_state++;
}


void handle_init(AppContextRef ctx) {
  (void)ctx;

  resource_init_current_app(&APP_RESOURCES);

  font_12 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_12));
  font_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_18));
  font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_24));

  // set default unit of measure
  change_units(UNITS_IMPERIAL, true);
  
  buttons_init();

  Window* window = &s_data.window;
  window_init(window, "Pebble Bike");
  window_set_background_color(&s_data.window, GColorWhite);
  window_set_fullscreen(&s_data.window, true);

  topbar_layer_init(window);

  screen_speed_layer_init(window);
  screen_altitude_layer_init(window);
  screen_live_layer_init(window);
  screen_map_layer_init(window);

  #if DEBUG
  screen_debug1_layer_init(window);
  screen_debug2_layer_init(window);
  #endif

  action_bar_init(window);

  // Reduce the sniff interval for more responsive messaging at the expense of
  // increased energy consumption by the Bluetooth module
  // The sniff interval will be restored by the system after the app has been
  // unloaded
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);

  window_stack_push(window, true /* Animated */);
  
  send_version();

  #if ROCKSHOT
    rockshot_init(ctx);
  #endif
}
static void handle_deinit(AppContextRef c) {
  app_sync_deinit(&s_data.sync);
   
  screen_speed_deinit();

  buttons_deinit();
}

void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)t;
  (void)ctx;

  char *time_format;
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  string_format_time(s_data.time, sizeof(s_data.time), time_format, t->tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (s_data.time[0] == '0')) {
    memmove(s_data.time, &s_data.time[1], sizeof(s_data.time) - 1);
  }
  layer_mark_dirty(&s_data.topbar_layer.layer);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = MINUTE_UNIT
    },
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 200,
        .outbound = 256,
      },
      .default_callbacks.callbacks = {
        .in_received = communication_in_received_callback,
        .in_dropped = communication_in_dropped_callback,
      },
    }
  };
  #if ROCKSHOT
    rockshot_main(&handlers);
  #endif
  app_event_loop(params, &handlers);
}
