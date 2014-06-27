#include "pebble.h"
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "pebblebike.h"
#include "communication.h"
#include "buttons.h"
#include "menu.h"
#include "screens.h"
#include "screen_speed.h"
#include "screen_altitude.h"
#include "screen_live.h"
#include "screen_map.h"
#if DEBUG
  #include "screen_debug.h"
#endif

GFont font_12, font_18, font_24;

AppData s_data;
GPSData s_gpsdata;
LiveData s_live;
int nbchange_state=0;

void change_units(uint8_t units, bool first_time) {
  if ((units == s_gpsdata.units) && !first_time) {
    return;
  }

  if (first_time) {
    if (persist_exists(PERSIST_UNITS_KEY)) {
      units = persist_read_int(PERSIST_UNITS_KEY);
    } else {
      // default value
      units = UNITS_METRIC;
    }
  } else {
    // save new value
    persist_write_int(PERSIST_UNITS_KEY, units);
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
  if (!first_time) {
    layer_mark_dirty(text_layer_get_layer(s_data.miles_layer));
    layer_mark_dirty(text_layer_get_layer(s_data.mph_layer));
    layer_mark_dirty(text_layer_get_layer(s_data.avgmph_layer));
  }
  if (s_data.page_number == PAGE_SPEED) {
    strncpy(s_data.unitsSpeedOrHeartRate, s_data.unitsSpeed, 8);
  }
}

void change_state(uint8_t state) {
  if (state == s_data.state) {
    return;
  }
  s_data.state = state;

  buttons_update();
  
  nbchange_state++;
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {

  char *time_format;
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(s_data.time, sizeof(s_data.time), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (s_data.time[0] == '0')) {
    memmove(s_data.time, &s_data.time[1], sizeof(s_data.time) - 1);
  }
  layer_mark_dirty(s_data.topbar_layer.layer);
}

void bt_callback(bool connected) {
  topbar_toggle_bluetooth_icon(connected);
}

static void init(void) {

  s_data.phone_battery_level = -1;

  font_12 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_12));
  font_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_18));
  font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_24));
  
  s_gpsdata.heartrate = 255; // no data at startup

  // set default unit of measure
  change_units(UNITS_IMPERIAL, true);
  
  buttons_init();

  s_data.window = window_create();
  window_set_background_color(s_data.window, GColorWhite);
  window_set_fullscreen(s_data.window, true);

  topbar_layer_init(s_data.window);

  screen_speed_layer_init(s_data.window);
  screen_altitude_layer_init(s_data.window);
  screen_live_layer_init(s_data.window);
  screen_map_layer_init(s_data.window);

  #if DEBUG
    screen_debug1_layer_init(s_data.window);
    screen_debug2_layer_init(s_data.window);
  #endif

  action_bar_init(s_data.window);
  menu_init();

  // Reduce the sniff interval for more responsive messaging at the expense of
  // increased energy consumption by the Bluetooth module
  // The sniff interval will be restored by the system after the app has been
  // unloaded
  //app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  
  communication_init();
  
  window_stack_push(s_data.window, true /* Animated */);
  
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  bluetooth_connection_service_subscribe(bt_callback);
  
  send_version();
}
static void deinit(void) {
  communication_deinit();
  
  window_destroy(s_data.window);

  topbar_layer_deinit();

  screen_speed_deinit();
  screen_altitude_layer_deinit();
  screen_live_layer_deinit();
  screen_map_layer_deinit();

  #if DEBUG
    screen_debug1_layer_deinit();
    screen_debug2_layer_deinit();
  #endif
  
  action_bar_deinit();
  menu_deinit();

  buttons_deinit();

  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}