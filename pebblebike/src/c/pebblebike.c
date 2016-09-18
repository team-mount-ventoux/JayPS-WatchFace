#include "pebble.h"
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "pebblebike.h"
#include "communication.h"
#ifdef PBL_HEALTH
  #include "health.h"
#endif
#include "buttons.h"
#include "screens.h"
#include "screen_data.h"
#ifdef ENABLE_FUNCTION_LIVE
  #include "screen_live.h"
#endif
#include "screen_map.h"
#include "screen_config.h"
#include "graph.h"

#include "overlord.h"
#include "overlays.h"
#include "ovl/init.h"


#ifdef PBL_PLATFORM_CHALK
GFont font_roboto_bold_16;
#endif
GFont font_roboto_bold_62;

AppData s_data;
GPSData s_gpsdata;

//map
GPoint pts[MAP_NUM_POINTS];
int cur_point = 0;
int map_scale = MAP_SCALE_INI;
int nb_points = 0;
int32_t xposprev = 0, yposprev = 0;


#ifdef ENABLE_FUNCTION_LIVE
LiveData s_live;
#endif
bool title_instead_of_units = true;
GColor bg_color_data_main;
int nbchange_state=0;
bool need_launch_config=false;

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
  overlay_load(OVL_INIT_OVL);
  _ovl_init();

  // callbacks cannot be in the overlay, register them here
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  bluetooth_connection_service_subscribe(bt_callback);
}

#ifdef ENABLE_GLANCE
static void prv_update_app_glance(AppGlanceReloadSession *session, size_t limit, void *context) {
  if (limit < 1) {
    return;
  }

  char message[50];
  snprintf(message, sizeof(message), "%s%s - %s%s", s_data.distance, s_data.unitsDistance, s_data.ascent, s_data.unitsAltitude);
  const AppGlanceSlice entry = (AppGlanceSlice ) {
    .layout = {
        //.icon = RESOURCE_ID_MENU_ICON,
        .subtitle_template_string = message
    },
    .expiration_time = APP_GLANCE_SLICE_NO_EXPIRATION
  };

  const AppGlanceResult result = app_glance_add_slice(session, entry);
  if (result != APP_GLANCE_RESULT_SUCCESS) {
    LOG_INFO("AppGlance Error: %d", result);
  }
}
#endif
static void deinit(void) {

#ifdef ENABLE_GLANCE
  if (s_gpsdata.distance100 > 0) {
    app_glance_reload(prv_update_app_glance, NULL);
  }
#endif
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
  //screen_map_layer_deinit();

  action_bar_deinit();
  //menu_deinit();

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
