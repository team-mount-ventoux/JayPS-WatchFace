#include "pebble.h"
#include <stdint.h>
#include <string.h>
#include "config.h"
#ifdef ENABLE_LOCALIZE
  #include "localize.h"
#endif
#include "pebblebike.h"
#include "communication.h"
#ifdef PBL_HEALTH
  #include "health.h"
#endif
#include "heartrate.h"
#include "buttons.h"
#include "menu.h"
#include "screens.h"
#include "screen_data.h"
#ifdef ENABLE_FUNCTION_LIVE
  #include "screen_live.h"
#endif
#include "screen_map.h"
#include "screen_config.h"
#include "graph.h"

#ifdef PBL_PLATFORM_CHALK
GFont font_roboto_bold_16;
#endif
GFont font_roboto_bold_62;

AppData s_data;
GPSData s_gpsdata;
#ifdef ENABLE_FUNCTION_LIVE
LiveData s_live;
#endif
bool title_instead_of_units = true;
GColor bg_color_data_main;
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
  switch(s_gpsdata.units) {
    case UNITS_IMPERIAL:
      strncpy(s_data.unitsSpeed, SPEED_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsAltitude, ALTITUDE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsAscentRate, ASCENT_RATE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsTemperature, TEMPERATURE_UNIT_IMPERIAL, 8);
      break;
    case UNITS_METRIC:
      strncpy(s_data.unitsSpeed, SPEED_UNIT_METRIC, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_METRIC, 8);
      strncpy(s_data.unitsAltitude, ALTITUDE_UNIT_METRIC, 8);
      strncpy(s_data.unitsAscentRate, ASCENT_RATE_UNIT_METRIC, 8);
      strncpy(s_data.unitsTemperature, TEMPERATURE_UNIT_METRIC, 8);
      break;
    case UNITS_NAUTICAL_IMPERIAL:
      strncpy(s_data.unitsSpeed, SPEED_UNIT_NAUTICAL, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_NAUTICAL, 8);
      strncpy(s_data.unitsAltitude, ALTITUDE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsAscentRate, ASCENT_RATE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsTemperature, TEMPERATURE_UNIT_IMPERIAL, 8);
      break;
    case UNITS_NAUTICAL_METRIC:
      strncpy(s_data.unitsSpeed, SPEED_UNIT_NAUTICAL, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_NAUTICAL, 8);
      strncpy(s_data.unitsAltitude, ALTITUDE_UNIT_METRIC, 8);
      strncpy(s_data.unitsAscentRate, ASCENT_RATE_UNIT_METRIC, 8);
      strncpy(s_data.unitsTemperature, TEMPERATURE_UNIT_METRIC, 8);
      break;
    case UNITS_RUNNING_IMPERIAL:
      strncpy(s_data.unitsSpeed, SPEED_UNIT_RUNNING_IMPERIAL, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsAltitude, ALTITUDE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsAscentRate, ASCENT_RATE_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsTemperature, TEMPERATURE_UNIT_IMPERIAL, 8);
      break;
    case UNITS_RUNNING_METRIC:
      strncpy(s_data.unitsSpeed, SPEED_UNIT_RUNNING_METRIC, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_METRIC, 8);
      strncpy(s_data.unitsAltitude, ALTITUDE_UNIT_METRIC, 8);
      strncpy(s_data.unitsAscentRate, ASCENT_RATE_UNIT_METRIC, 8);
      strncpy(s_data.unitsTemperature, TEMPERATURE_UNIT_METRIC, 8);
      break;
  }

  if (!first_time) {
    //todo(custom) screen_data_dirty
    layer_mark_dirty(text_layer_get_layer(s_data.screenData_layer.field_top.unit_layer));
    layer_mark_dirty(text_layer_get_layer(s_data.screenData_layer.field_top2.unit_layer));
    layer_mark_dirty(text_layer_get_layer(s_data.screenData_layer.field_bottom_left.unit_layer));
    layer_mark_dirty(text_layer_get_layer(s_data.screenData_layer.field_bottom_right.unit_layer));
  }
  if (s_data.data_subpage != SUBPAGE_UNDEF) {
    strncpy(s_data.unitsSpeedOrHeartRate, s_data.unitsSpeed, 8);
  }
}

void change_state(uint8_t state) {
  if (state == s_data.state) {
    return;
  }
  s_data.state = state;
  if (s_data.state == STATE_STOP) {
    screen_reset_instant_data();
  }
  layer_set_hidden(text_layer_get_layer(s_data.topbar_layer.accuracy_layer), s_data.state == STATE_STOP);
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
  //layer_mark_dirty(s_data.topbar_layer.layer);
  update_screens();
}

void bt_callback(bool connected) {
  topbar_toggle_bluetooth_icon(connected);
}

static void init(void) {
  config_load();

#ifdef ENABLE_LOCALIZE
  locale_init();
#endif
  heartrate_init();
  s_gpsdata.heartrate = 255; // no data at startup
  bg_color_data_main = BG_COLOR_DATA_MAIN;
#ifdef ENABLE_DEBUG_FIELDS_SIZE
  strcpy(s_data.speed, "188.8");
  strcpy(s_data.distance, "88.8");
  strcpy(s_data.avgspeed, "888.8");
  strcpy(s_data.altitude, "888.8");
  strcpy(s_data.ascent, "1342");
  strcpy(s_data.ascentrate, "548");
  strcpy(s_data.slope, "5");
  strcpy(s_data.accuracy, "9");
  strcpy(s_data.bearing, "270");
  strcpy(s_data.elapsedtime, "1:05:28");
  strcpy(s_data.maxspeed, "25.3");
  strcpy(s_data.heartrate, "128");
  strcpy(s_data.cadence, "90");
#endif
#ifdef ENABLE_DEMO
  strcpy(s_data.maxspeed, "26.1");
  strcpy(s_data.distance, "2.0");
  strcpy(s_data.avgspeed, "14.0");
  strcpy(s_data.altitude, "1139");
  strcpy(s_data.accuracy, "4");
  strcpy(s_data.steps, "7548");
  strcpy(s_data.elapsedtime, "1:15:28");
  strcpy(s_data.heartrate, "154");
  s_gpsdata.heartrate = 154;
  heartrate_new_data(s_gpsdata.heartrate);
  s_data.live = 1;
  s_data.state = STATE_START;
#else
  strcpy(s_data.speed, "0.0");
  strcpy(s_data.distance, "-");
  strcpy(s_data.avgspeed, "-");
  strcpy(s_data.altitude, "-");
  strcpy(s_data.ascent, "-");
  strcpy(s_data.ascentrate, "-");
  strcpy(s_data.slope, "-");
  strcpy(s_data.accuracy, "-");
  strcpy(s_data.bearing, "-");
  strcpy(s_data.elapsedtime, "00:00:00");
  strcpy(s_data.maxspeed, "-");
  strcpy(s_data.heartrate, "-");
  strcpy(s_data.cadence, "-");
#endif
  //strcpy(s_data.lat, "-");
  //strcpy(s_data.lon, "-");
  //strcpy(s_data.nbascent, "-");

  s_data.phone_battery_level = -1;

#ifdef PBL_PLATFORM_CHALK
  font_roboto_bold_16 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_16));
#endif
  font_roboto_bold_62 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_62));

  // set default unit of measure
  change_units(UNITS_IMPERIAL, true);

  buttons_init();

  s_data.window = window_create();
  window_set_background_color(s_data.window, BG_COLOR_WINDOW);
#ifdef PBL_SDK_2
  window_set_fullscreen(s_data.window, true);
#endif
  topbar_layer_init(s_data.window);

  screen_data_layer_init(s_data.window);
  //screen_altitude_layer_init(s_data.window);
#ifdef ENABLE_FUNCTION_LIVE
  screen_live_layer_init(s_data.window);
#endif
  screen_map_layer_init(s_data.window);

  #ifdef PRODUCTION
    #ifndef ENABLE_DEMO
      screen_reset_instant_data();
    #endif
  #endif

  action_bar_init(s_data.window);
  menu_init();

  // Reduce the sniff interval for more responsive messaging at the expense of
  // increased energy consumption by the Bluetooth module
  // The sniff interval will be restored by the system after the app has been
  // unloaded
  //app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  
  communication_init();

  screen_data_update_config(true);
  //screen_altitude_update_config();
  graph_init();
  
  window_stack_push(s_data.window, true /* Animated */);
  
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  bluetooth_connection_service_subscribe(bt_callback);
  
  send_version(true);
}
static void deinit(void) {
#ifdef PBL_HEALTH
  health_deinit();
#endif
  communication_deinit();
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

#ifdef ENABLE_APP_DEINIT
  topbar_layer_deinit();
  screen_data_deinit();
  graph_deinit();
  //screen_altitude_layer_deinit();
#ifdef ENABLE_FUNCTION_LIVE
  screen_live_layer_deinit();
#endif
  screen_map_layer_deinit();

  action_bar_deinit();
  menu_deinit();

  buttons_deinit();

#ifdef PBL_PLATFORM_CHALK
  fonts_unload_custom_font(font_roboto_bold_16);
#endif
  fonts_unload_custom_font(font_roboto_bold_62);
  window_destroy(s_data.window);
#endif
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
