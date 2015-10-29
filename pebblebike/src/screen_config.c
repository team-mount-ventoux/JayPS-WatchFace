#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screen_speed.h"
#include "screens.h"
#include "buttons.h"
#include "screen_config.h"


FieldLayer *cur_field;
bool config_hidden = false;
static AppTimer *config_timer;
uint8_t config_screen = CONFIG_SCREEN_DISABLED;
uint8_t config_field = CONFIG_FIELD_SCREEN_A__MIN;

ConfigData config;

const char *field_get_title(uint8_t field) {
  switch(field) {
    case FIELD_AVGSPEED: return "Avg speed"; break;
    case FIELD_DISTANCE: return "Distance"; break;
    case FIELD_ALTITUDE: return "Altitude"; break;
    case FIELD_ASCENT: return "Ascent"; break;
    case FIELD_SPEED: return "Speed"; break;
    case FIELD_BEARING: return "Bearing"; break;
    case FIELD_DURATION: return "Duration"; break;
    case FIELD_MAXSPEED: return "Max speed"; break;
    //case FIELD_LAT: return "Lat"; break;
    //case FIELD_LON: return "Lon"; break;
    case FIELD_ASCENTRATE: return "Ascent rate"; break;
    //case FIELD_NBASCENT: return "Nb ascent"; break;
    case FIELD_SLOPE: return "Slope"; break;
    case FIELD_ACCURACY: return "Accuracy"; break;
    case FIELD_HEARTRATE: return "Heartrate"; break;
    case FIELD_CADENCE: return "Cadence"; break;
    case FIELD_TEMPERATURE: return "Temperature"; break;
    default: return "Unknown";
  }
}
const char *field_get_text(uint8_t field) {
  switch(field) {
    case FIELD_AVGSPEED: return s_data.avgspeed; break;
    case FIELD_DISTANCE: return s_data.distance; break;
    case FIELD_ALTITUDE: return s_data.altitude; break;
    case FIELD_ASCENT: return s_data.ascent; break;
    case FIELD_SPEED: return s_data.speed; break;
    case FIELD_BEARING: return s_data.bearing; break;
    case FIELD_DURATION: return s_data.elapsedtime; break;
    case FIELD_MAXSPEED: return s_data.maxspeed; break;
    //case FIELD_LAT: return s_data.lat; break;
    //case FIELD_LON: return s_data.lon; break;
    case FIELD_ASCENTRATE: return s_data.ascentrate; break;
    //case FIELD_NBASCENT: return s_data.nbascent; break;
    case FIELD_SLOPE: return s_data.slope; break;
    case FIELD_ACCURACY: return s_data.accuracy; break;
    case FIELD_HEARTRATE: return s_data.heartrate; break;
    case FIELD_CADENCE: return s_data.cadence; break;
    case FIELD_TEMPERATURE: return s_data.temperature; break;
    default: return "-";
  }
}
const char *field_get_units(uint8_t field) {
  switch(field) {
    case FIELD_AVGSPEED: return s_data.unitsSpeed; break;
    case FIELD_DISTANCE: return s_data.unitsDistance; break;
    case FIELD_ALTITUDE: return s_data.unitsAltitude; break;
    case FIELD_ASCENT: return s_data.unitsAltitude; break;
    case FIELD_SPEED: return s_data.unitsSpeed; break;
    case FIELD_BEARING: return "°"; break;
    case FIELD_DURATION: return "s"; break;
    case FIELD_MAXSPEED: return s_data.unitsSpeed; break;
    //case FIELD_LAT: return ""; break;
    //case FIELD_LON: return ""; break;
    case FIELD_ASCENTRATE: return s_data.unitsAscentRate; break;
    //case FIELD_NBASCENT: return ""; break;
    case FIELD_SLOPE: return "%"; break;
    case FIELD_ACCURACY: return "m"; break;
    case FIELD_HEARTRATE: return HEART_RATE_UNIT; break;
    case FIELD_CADENCE: return "rpm"; break;
    case FIELD_TEMPERATURE: return s_data.unitsTemperature; break;
    default: return "Unk";
  }
}
void field_set_text(FieldLayer field_layer, uint8_t force_field) {
  uint8_t type = force_field != FIELD__UNUSED ? force_field : field_layer.type;
  if (field_layer.title_layer != NULL) {
    text_layer_set_text(field_layer.title_layer, field_get_title(type));
  }
  if (field_layer.data_layer != NULL) {
    text_layer_set_text(field_layer.data_layer, field_get_text(type));
  }
  if (field_layer.unit_layer != NULL) {
    text_layer_set_text(field_layer.unit_layer, title_instead_of_units ? field_get_title(type) : field_get_units(type));
  }
}
void screen_speed_update_config() {
  field_set_text(s_data.screenA_layer.field_top, s_data.page_number == PAGE_HEARTRATE ? FIELD_HEARTRATE : FIELD__UNUSED);
  field_set_text(s_data.screenA_layer.field_top2, FIELD__UNUSED);
  field_set_text(s_data.screenA_layer.field_bottom_left, FIELD__UNUSED);
  field_set_text(s_data.screenA_layer.field_bottom_right, FIELD__UNUSED);
}
void screen_altitude_update_config() {
  field_set_text(s_data.screenB_layer.field_top_left, FIELD__UNUSED);
  field_set_text(s_data.screenB_layer.field_top_right, FIELD__UNUSED);
  field_set_text(s_data.screenB_layer.field_bottom_left, FIELD__UNUSED);
  field_set_text(s_data.screenB_layer.field_bottom_right, FIELD__UNUSED);
}

void config_change_visibility(FieldLayer* field_layer, bool hidden) {
  if (field_layer->title_layer != NULL) {
    layer_set_hidden(text_layer_get_layer(field_layer->title_layer), hidden);
  }
  if (field_layer->data_layer != NULL) {
    layer_set_hidden(text_layer_get_layer(field_layer->data_layer), hidden);
  }
  if (field_layer->unit_layer != NULL) {
    layer_set_hidden(text_layer_get_layer(field_layer->unit_layer), hidden);  
  }
}
static void config_timer_callback(void *data) {
  config_hidden = !config_hidden;
  config_timer = app_timer_register(config_hidden ? 200 : 1000, config_timer_callback, NULL);
  config_change_visibility(cur_field, config_hidden);
}
void config_start() {
  if (s_data.page_number == PAGE_SPEED) {
    config_screen = CONFIG_SCREEN_A;
    config_field = CONFIG_FIELD_SCREEN_A_TOP;
    cur_field = &s_data.screenA_layer.field_top;
    screen_speed_update_config();
    text_layer_set_text(s_data.topbar_layer.time_layer, field_get_title(cur_field->type));
    layer_mark_dirty(s_data.topbar_layer.layer);
  } else if (s_data.page_number == PAGE_ALTITUDE) {
    config_screen = CONFIG_SCREEN_B;
    config_field = CONFIG_FIELD_SCREEN_B_TOP_LEFT;
    cur_field = &s_data.screenB_layer.field_top_left;
    screen_altitude_update_config();
    text_layer_set_text(s_data.topbar_layer.time_layer, field_get_title(cur_field->type));
    layer_mark_dirty(s_data.topbar_layer.layer);
  } else {
    return;
  }
  vibes_short_pulse();
  if (config_timer != NULL) {
    app_timer_cancel(config_timer);
  }
  config_timer = app_timer_register(1000, config_timer_callback, NULL);
  config_hidden = false;
  action_bar_set_menu_up_down_buttons();
}
void config_stop() {
  config_screen = CONFIG_SCREEN_DISABLED;
  config_change_visibility(cur_field, false);
  app_timer_cancel(config_timer);
  config_timer = NULL;
  text_layer_set_text(s_data.topbar_layer.time_layer, s_data.time);
  layer_mark_dirty(s_data.topbar_layer.layer);
  config_save();
}
void config_change_field() {
  config_change_visibility(cur_field, false);
  config_field++;
  if (config_screen == CONFIG_SCREEN_A && config_field == CONFIG_FIELD_SCREEN_A__MAX) {
    config_field = CONFIG_FIELD_SCREEN_A__MIN;
  } else if (config_screen == CONFIG_SCREEN_B && config_field == CONFIG_FIELD_SCREEN_B__MAX) {
    config_field = CONFIG_FIELD_SCREEN_B__MIN;
  }
  switch (config_field) {
    case CONFIG_FIELD_SCREEN_A_TOP:           cur_field = &s_data.screenA_layer.field_top; break;
    case CONFIG_FIELD_SCREEN_A_TOP2:          cur_field = &s_data.screenA_layer.field_top2; break;
    case CONFIG_FIELD_SCREEN_A_BOTTOM_LEFT:   cur_field = &s_data.screenA_layer.field_bottom_left; break;
    case CONFIG_FIELD_SCREEN_A_BOTTOM_RIGHT:  cur_field = &s_data.screenA_layer.field_bottom_right; break;
    case CONFIG_FIELD_SCREEN_B_TOP_LEFT:      cur_field = &s_data.screenB_layer.field_top_left; break;
    case CONFIG_FIELD_SCREEN_B_TOP_RIGHT:     cur_field = &s_data.screenB_layer.field_top_right; break;
    case CONFIG_FIELD_SCREEN_B_BOTTOM_LEFT:   cur_field = &s_data.screenB_layer.field_bottom_left; break;
    case CONFIG_FIELD_SCREEN_B_BOTTOM_RIGHT:  cur_field = &s_data.screenB_layer.field_bottom_right; break;
  }

  config_change_visibility(cur_field, true);

  text_layer_set_text(s_data.topbar_layer.time_layer, field_get_title(cur_field->type));
  layer_mark_dirty(s_data.topbar_layer.layer);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "config_field %d", config_field);
}
void config_change_type(uint8_t direction) {
  if (direction == CONFIG_CHANGE_TYPE_NEXT) {
    cur_field->type++;
    if (cur_field->type == FIELD__MAX) {
      cur_field->type = FIELD__MIN;
    }
  } else {
    // CONFIG_CHANGE_TYPE_PREVIOUS
    if (cur_field->type == FIELD__MIN) {
      cur_field->type = FIELD__MAX;
    }
    cur_field->type--;
  }
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "type %d", cur_field->type);
  
  if (config_screen == CONFIG_SCREEN_A) {
    screen_speed_update_config();
  } else {
    screen_altitude_update_config();
  }
  text_layer_set_text(s_data.topbar_layer.time_layer, field_get_title(cur_field->type));
  layer_mark_dirty(s_data.topbar_layer.layer);
}

void config_load() {
  if (persist_exists(PERSIST_CONFIG_KEY)) {
    persist_read_data(PERSIST_CONFIG_KEY, &config, sizeof(config));
  } else {
    config.screenA_top_type           = FIELD_SPEED;
    config.screenA_top2_type          = FIELD_ALTITUDE;
    config.screenA_bottom_left_type   = FIELD_DISTANCE;
    config.screenA_bottom_right_type  = FIELD_AVGSPEED;
    config.screenB_top_left_type      = FIELD_ALTITUDE;
    config.screenB_top_right_type     = FIELD_ASCENT;
    config.screenB_bottom_left_type   = FIELD_ASCENTRATE;
    config.screenB_bottom_right_type  = FIELD_SLOPE;
  }
}
void config_save() {
  config.screenA_top_type           = s_data.screenA_layer.field_top.type;
  config.screenA_top2_type          = s_data.screenA_layer.field_top2.type;
  config.screenA_bottom_left_type   = s_data.screenA_layer.field_bottom_left.type;
  config.screenA_bottom_right_type  = s_data.screenA_layer.field_bottom_right.type;
  config.screenB_top_left_type      = s_data.screenB_layer.field_top_left.type;
  config.screenB_top_right_type     = s_data.screenB_layer.field_top_right.type;
  config.screenB_bottom_left_type   = s_data.screenB_layer.field_bottom_left.type;
  config.screenB_bottom_right_type  = s_data.screenB_layer.field_bottom_right.type;
  persist_write_data(PERSIST_CONFIG_KEY, &config, sizeof(config));
}
