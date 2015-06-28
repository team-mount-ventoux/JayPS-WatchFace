#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screen_config.h"


FieldLayer *cur_field;
bool config_hidden = false;
static AppTimer *config_timer;
uint8_t config_field = CONFIG_FIELD_DISABLED;

ConfigData config;

const char *field_get_title(uint8_t field) {
  switch(field) {
    case FIELD_AVGSPEED: return "Avg speed"; break;
    case FIELD_DISTANCE: return "Distance"; break;
    case FIELD_ALTITUDE: return "Altitude"; break;
    case FIELD_ASCENT: return "Ascent"; break;
    case FIELD_SPEED: return "Speed"; break;
    case FIELD_BEARING: return "Bearing"; break;
    case FIELD_TIME: return "Time"; break;
    case FIELD_MAXSPEED: return "Max speed"; break;
    case FIELD_LAT: return "Lat"; break;
    case FIELD_LON: return "Lon"; break;
    case FIELD_ASCENTRATE: return "Ascent rate"; break;
    case FIELD_NBASCENT: return "Nb ascent"; break;
    case FIELD_SLOPE: return "Slope"; break;
    case FIELD_ACCURACY: return "Accuracy"; break;
    case FIELD_HEARTRATE: return "Heartrate"; break;
    case FIELD_CADENCE: return "Cadence"; break;
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
    case FIELD_BEARING: return "Bearing"; break;
    case FIELD_TIME: return "Time"; break;
    case FIELD_MAXSPEED: return "Max speed"; break;
    case FIELD_LAT: return "Lat"; break;
    case FIELD_LON: return "Lon"; break;
    case FIELD_ASCENTRATE: return s_data.ascentrate; break;
    case FIELD_NBASCENT: return "Nb ascent"; break;
    case FIELD_SLOPE: return s_data.slope; break;
    case FIELD_ACCURACY: return s_data.accuracy; break;
    case FIELD_HEARTRATE: return "hr"; break;
    case FIELD_CADENCE: return "Cadence"; break;
    default: return "-";
  }
}
const char *field_get_units(uint8_t field) {
  switch(field) {
    case FIELD_AVGSPEED: return s_data.unitsSpeed; break;
    case FIELD_DISTANCE: return s_data.unitsDistance; break;
    case FIELD_ALTITUDE: return s_data.altitude_layer.units; break;
    case FIELD_ASCENT: return s_data.altitude_layer.units; break;
    case FIELD_SPEED: return s_data.unitsSpeed; break;
    case FIELD_BEARING: return "°"; break;
    case FIELD_TIME: return "s"; break;
    case FIELD_MAXSPEED: return s_data.unitsSpeed; break;
    case FIELD_LAT: return ""; break;
    case FIELD_LON: return ""; break;
    case FIELD_ASCENTRATE: return s_data.altitude_ascent_rate.units; break;
    case FIELD_NBASCENT: return ""; break;
    case FIELD_SLOPE: return "%"; break;
    case FIELD_ACCURACY: return "m"; break;
    case FIELD_HEARTRATE: return "bpm"; break;
    case FIELD_CADENCE: return "rpm"; break;
    default: return "Unk";
  }
}
void field_set_text(FieldLayer field_layer) {
  if (field_layer.title_layer != NULL) {
    text_layer_set_text(field_layer.title_layer, field_get_title(field_layer.type));
  }
  if (field_layer.data_layer != NULL) {
    text_layer_set_text(field_layer.data_layer, field_get_text(field_layer.type));
  }
  if (field_layer.unit_layer != NULL) {
    text_layer_set_text(field_layer.unit_layer, field_get_units(field_layer.type));
  }
}
void screen_speed_update_config() {
  field_set_text(s_data.screenA_layer.field_top);
  field_set_text(s_data.screenA_layer.field_bottom_left);
  field_set_text(s_data.screenA_layer.field_bottom_right);

  text_layer_set_text(s_data.topbar_layer.time_layer, field_get_title(s_data.screenA_layer.field_top.type));
  layer_mark_dirty(s_data.topbar_layer.layer);

  //text_layer_set_text(s_data.screenA_layer.field_bottom_right.data_layer, field_get_text(s_data.screenA_layer.field_bottom_right.type));
    //text_layer_set_text(s_data.screenA_layer.distance_layer, s_data.avgspeed);
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
  if (config_field == CONFIG_FIELD_DISABLED) {
    vibes_short_pulse();
    config_field = CONFIG_FIELD_TOP;
    cur_field = &s_data.screenA_layer.field_top;
    if (config_timer != NULL) {
      app_timer_cancel(config_timer);
    }
    config_timer = app_timer_register(1000, config_timer_callback, NULL);
    config_hidden = false;
    screen_speed_update_config();
  } else {
    config_stop();
  }
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "config_field %d", config_field);
}
void config_stop() {
  config_field = CONFIG_FIELD_DISABLED;
  config_change_visibility(cur_field, false);
  app_timer_cancel(config_timer);
  config_timer = NULL;
  text_layer_set_text(s_data.topbar_layer.time_layer, s_data.time);
  layer_mark_dirty(s_data.topbar_layer.layer);
  config_save();
}
void config_change_field() {
  config_field++;
  if (config_field == CONFIG_FIELD__MAX) {
    config_field = CONFIG_FIELD__MIN;
  }
  config_change_visibility(cur_field, false);
  if (config_field == CONFIG_FIELD_BOTTOM_LEFT) {
    cur_field = &s_data.screenA_layer.field_bottom_left;
  } else if (config_field == CONFIG_FIELD_BOTTOM_RIGHT) {
    cur_field = &s_data.screenA_layer.field_bottom_right;    
  } else {
    //default
    cur_field = &s_data.screenA_layer.field_top;
  }
  config_change_visibility(cur_field, true);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "config_field %d", config_field);
}
void config_change_type() {
  cur_field->type++;
  if (cur_field->type == FIELD__MAX) {
    cur_field->type = FIELD__MIN;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "type %d", cur_field->type);
  screen_speed_update_config();
}

void config_load() {
  if (persist_exists(PERSIST_CONFIG_KEY)) {
    persist_read_data(PERSIST_CONFIG_KEY, &config, sizeof(config));
  } else {
    config.screenA_top_type = FIELD_SPEED;
    config.screenA_bottom_left_type = FIELD_DISTANCE;
    config.screenA_bottom_right_type = FIELD_AVGSPEED;
  }
}
void config_save() {
  config.screenA_top_type = s_data.screenA_layer.field_top.type;
  config.screenA_bottom_left_type = s_data.screenA_layer.field_bottom_left.type;
  config.screenA_bottom_right_type = s_data.screenA_layer.field_bottom_right.type;
  persist_write_data(PERSIST_CONFIG_KEY, &config, sizeof(config));
}
