#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screen_speed.h"
#include "screens.h"
#include "buttons.h"
#include "screen_config.h"
#ifdef PBL_HEALTH
  #include "health.h"
#endif


FieldLayer *cur_field;
bool config_hidden = false;
static AppTimer *config_timer;
uint8_t config_screen = CONFIG_SCREEN_DISABLED;
uint8_t config_field = CONFIG_FIELD_SCREEN_A__MIN;

#define CONFIG_NB_FIELD_ORDER 25
uint8_t config_order[CONFIG_NB_FIELD_ORDER] = {
    FIELD_ACCURACY,
    FIELD_ALTITUDE,
    FIELD_ALTITUDE_GRAPH_ONLY,
#ifdef PBL_COLOR
    FIELD_ALTITUDE_DATA_AND_GRAPH,
#endif
    FIELD_ASCENT,
    FIELD_ASCENTRATE,
    FIELD_ASCENTRATE_GRAPH_ONLY,
#ifdef PBL_COLOR
    FIELD_ASCENTRATE_DATA_AND_GRAPH,
#endif
    FIELD_AVGSPEED,
    FIELD_BEARING,
    FIELD_CADENCE,
    FIELD_DISTANCE,
    FIELD_DURATION,
    FIELD_HEARTRATE,
    FIELD_HEARTRATE_GRAPH_ONLY,
#ifdef PBL_COLOR
    FIELD_HEARTRATE_DATA_AND_GRAPH,
#endif
    //FIELD_LAT,
    //FIELD_LON,
    FIELD_MAXSPEED,
    //FIELD_NBASCENT,
    FIELD_SLOPE,
    FIELD_SPEED,
    FIELD_SPEED_GRAPH_ONLY,
#ifdef PBL_COLOR
    FIELD_SPEED_DATA_AND_GRAPH,
#endif
#ifdef PBL_HEALTH
    FIELD_STEPS,
    FIELD_STEPS_CADENCE,
#endif
    FIELD_TEMPERATURE,
    //FIELD_TIME,
    FIELD__UNUSED,
};

ConfigData config;

const char *field_get_title(uint8_t field) {
  switch(field) {
    case FIELD_AVGSPEED: return "Avg speed"; break;
    case FIELD_DISTANCE: return "Distance"; break;
    case FIELD_ALTITUDE: return "Altitude data"; break;
    case FIELD_ALTITUDE_DATA_AND_GRAPH: return "Altitude all"; break;
    case FIELD_ALTITUDE_GRAPH_ONLY: return "Altitude graph"; break;
    case FIELD_ASCENT: return "Ascent"; break;
    case FIELD_SPEED: return "Speed data"; break;
    case FIELD_SPEED_DATA_AND_GRAPH: return "Speed all"; break;
    case FIELD_SPEED_GRAPH_ONLY: return "Speed graph"; break;
    case FIELD_BEARING: return "Bearing"; break;
    case FIELD_DURATION: return "Duration"; break;
    case FIELD_MAXSPEED: return "Max speed"; break;
    //case FIELD_LAT: return "Lat"; break;
    //case FIELD_LON: return "Lon"; break;
    case FIELD_ASCENTRATE: return "Ascent rate data"; break;
    case FIELD_ASCENTRATE_DATA_AND_GRAPH: return "Ascent rate all"; break;
    case FIELD_ASCENTRATE_GRAPH_ONLY: return "Ascent rate graph"; break;
    //case FIELD_NBASCENT: return "Nb ascent"; break;
    case FIELD_SLOPE: return "Slope"; break;
    case FIELD_ACCURACY: return "Accuracy"; break;
    case FIELD_HEARTRATE: return "Heartrate data"; break;
    case FIELD_HEARTRATE_DATA_AND_GRAPH: return "Heartrate all"; break;
    case FIELD_HEARTRATE_GRAPH_ONLY: return "Heartrate graph"; break;
    case FIELD_CADENCE: return "Cadence"; break;
    case FIELD_TEMPERATURE: return "Temperature"; break;
#ifdef PBL_HEALTH
    case FIELD_STEPS: return "Steps"; break;
    case FIELD_STEPS_CADENCE: return "Steps cadence"; break;
#endif
    default: return "Unknown";
  }
}
const char *field_get_text(uint8_t field) {
  switch(field) {
    case FIELD_AVGSPEED: return s_data.avgspeed; break;
    case FIELD_DISTANCE: return s_data.distance; break;
    case FIELD_ALTITUDE:
    case FIELD_ALTITUDE_DATA_AND_GRAPH:
      return s_data.altitude; break;
    case FIELD_ASCENT: return s_data.ascent; break;
    case FIELD_SPEED:
    case FIELD_SPEED_DATA_AND_GRAPH:
      return s_data.speed; break;
    case FIELD_BEARING: return s_data.bearing; break;
    case FIELD_DURATION: return s_data.elapsedtime; break;
    case FIELD_MAXSPEED: return s_data.maxspeed; break;
    //case FIELD_LAT: return s_data.lat; break;
    //case FIELD_LON: return s_data.lon; break;
    case FIELD_ASCENTRATE:
    case FIELD_ASCENTRATE_DATA_AND_GRAPH:
      return s_data.ascentrate; break;
    //case FIELD_NBASCENT: return s_data.nbascent; break;
    case FIELD_SLOPE: return s_data.slope; break;
    case FIELD_ACCURACY: return s_data.accuracy; break;
    case FIELD_HEARTRATE:
    case FIELD_HEARTRATE_DATA_AND_GRAPH:
      return s_data.heartrate; break;
    case FIELD_CADENCE: return s_data.cadence; break;
    case FIELD_TEMPERATURE: return s_data.temperature; break;
#ifdef PBL_HEALTH
    case FIELD_STEPS: return s_data.steps; break;
    case FIELD_STEPS_CADENCE: return s_data.steps_cadence; break;
#endif
    case FIELD_ALTITUDE_GRAPH_ONLY:
    case FIELD_ASCENTRATE_GRAPH_ONLY:
    case FIELD_HEARTRATE_GRAPH_ONLY:
    case FIELD_SPEED_GRAPH_ONLY:
      return ""; break;
    default: return "-";
  }
}
const char *field_get_units(uint8_t field) {
  switch(field) {
    case FIELD_AVGSPEED: return s_data.unitsSpeed; break;
    case FIELD_DISTANCE: return s_data.unitsDistance; break;
    case FIELD_ALTITUDE:
    case FIELD_ALTITUDE_DATA_AND_GRAPH:
      return s_data.unitsAltitude; break;
    case FIELD_ASCENT: return s_data.unitsAltitude; break;
    case FIELD_SPEED:
    case FIELD_SPEED_DATA_AND_GRAPH:
      return s_data.unitsSpeed; break;
    case FIELD_BEARING: return "°"; break;
    case FIELD_DURATION: return "s"; break;
    case FIELD_MAXSPEED: return s_data.unitsSpeed; break;
    //case FIELD_LAT: return ""; break;
    //case FIELD_LON: return ""; break;
    case FIELD_ASCENTRATE:
    case FIELD_ASCENTRATE_DATA_AND_GRAPH:
      return s_data.unitsAscentRate; break;
    //case FIELD_NBASCENT: return ""; break;
    case FIELD_SLOPE: return "%"; break;
    case FIELD_ACCURACY: return "m"; break;
    case FIELD_HEARTRATE:
    case FIELD_HEARTRATE_DATA_AND_GRAPH:
      return HEART_RATE_UNIT; break;
    case FIELD_CADENCE: return "rpm"; break;
    case FIELD_TEMPERATURE: return s_data.unitsTemperature; break;
#ifdef PBL_HEALTH
    case FIELD_STEPS: return "steps"; break;
    case FIELD_STEPS_CADENCE: return "spm"; break;
#endif
    default: return "";
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
#ifdef PBL_HEALTH
  health_init();
#endif
  if (s_data.page_number == PAGE_SPEED) {
    config_screen = CONFIG_SCREEN_A;
    config_field = CONFIG_FIELD_SCREEN_A_TOP;
    cur_field = &s_data.screenA_layer.field_top;
    screen_speed_update_config();
  } else if (s_data.page_number == PAGE_ALTITUDE) {
    config_screen = CONFIG_SCREEN_B;
    config_field = CONFIG_FIELD_SCREEN_B_TOP_LEFT;
    cur_field = &s_data.screenB_layer.field_top_left;
    screen_altitude_update_config();
  } else {
    return;
  }
  layer_set_hidden(text_layer_get_layer(s_data.topbar_layer.accuracy_layer), true);
  layer_set_hidden(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer), true);
  text_layer_set_text(s_data.topbar_layer.time_layer, field_get_title(cur_field->type));
  layer_mark_dirty(s_data.topbar_layer.layer);

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
  layer_set_hidden(text_layer_get_layer(s_data.topbar_layer.accuracy_layer), s_data.state == STATE_STOP);
  layer_set_hidden(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer), false);
  layer_mark_dirty(s_data.topbar_layer.layer);
  config_save();
#ifdef PBL_HEALTH
  health_init_if_needed();
#endif
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
bool config_field_contain_graph(uint8_t type_index) {
  bool result = false;
  result = result || config_order[type_index] == FIELD_ALTITUDE_GRAPH_ONLY;
  result = result || config_order[type_index] == FIELD_ASCENTRATE_GRAPH_ONLY;
  result = result || config_order[type_index] == FIELD_HEARTRATE_GRAPH_ONLY;
  result = result || config_order[type_index] == FIELD_SPEED_GRAPH_ONLY;
#ifdef PBL_COLOR
  result = result || config_order[type_index] == FIELD_ALTITUDE_DATA_AND_GRAPH;
  result = result || config_order[type_index] == FIELD_ASCENTRATE_DATA_AND_GRAPH;
  result = result || config_order[type_index] == FIELD_HEARTRATE_DATA_AND_GRAPH;
  result = result || config_order[type_index] == FIELD_SPEED_DATA_AND_GRAPH;
#endif
  return result;
}
void config_next_type() {
  cur_field->type_index++;
  if ((cur_field->type_index >= CONFIG_NB_FIELD_ORDER) || (config_order[cur_field->type_index] == FIELD__UNUSED)) {
    cur_field->type_index = 0;
  }
}
void config_next_type_graph(bool can_display_graph) {
  config_next_type();
  if (!can_display_graph && config_field_contain_graph(cur_field->type_index)) {
    config_next_type();
  }
  // twice because _DATA_AND_GRAPH and _ALTITUDE_GRAPH_ONLY,
  if (!can_display_graph && config_field_contain_graph(cur_field->type_index)) {
    config_next_type();
  }
}

void config_previous_type() {
  if (cur_field->type_index == 0) {
    // search for last index not FIELD__UNUSED
    for (int i = 0; i < CONFIG_NB_FIELD_ORDER; i++) {
      if (config_order[i] == FIELD__UNUSED) {
        cur_field->type_index = i -1 ;
        return;
      }
    }
  } else {
    cur_field->type_index--;
  }
}
void config_previous_type_graph(bool can_display_graph) {
  config_previous_type();
  if (!can_display_graph && config_field_contain_graph(cur_field->type_index)) {
    config_previous_type();
  }
  // twice because _DATA_AND_GRAPH and _ALTITUDE_GRAPH_ONLY,
  if (!can_display_graph && config_field_contain_graph(cur_field->type_index)) {
    config_previous_type();
  }
}


void config_change_type(uint8_t direction) {
  if (direction == CONFIG_CHANGE_TYPE_NEXT) {
    config_next_type_graph(config_field == CONFIG_FIELD_SCREEN_A_TOP2);
    cur_field->type = config_order[cur_field->type_index];
  } else {
    // CONFIG_CHANGE_TYPE_PREVIOUS
    config_previous_type_graph(config_field == CONFIG_FIELD_SCREEN_A_TOP2);
    cur_field->type = config_order[cur_field->type_index];
  }
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "%d: type %d", cur_field->type_index, cur_field->type);
  
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
    int version = 0;
    if (persist_exists(PERSIST_VERSION)) {
      version = persist_read_int(PERSIST_VERSION);
    }
    if (version < MIN_VERSION_PEBBLE_SCREEN_A_TOP2) {
      config.screenA_top2_type          = FIELD_ALTITUDE;
    }
  } else {
#if DEMO
    config.screenA_top_type           = FIELD_MAXSPEED;
#else
    config.screenA_top_type           = FIELD_SPEED;
#endif
#ifdef PBL_COLOR
    config.screenA_top2_type          = FIELD_ALTITUDE_DATA_AND_GRAPH;
#else
    config.screenA_top2_type          = FIELD_ALTITUDE;
#endif
    config.screenA_bottom_left_type   = FIELD_DISTANCE;
    config.screenA_bottom_right_type  = FIELD_AVGSPEED;
    config.screenB_top_left_type      = FIELD_ALTITUDE;
    config.screenB_top_right_type     = FIELD_ASCENT;
    config.screenB_bottom_left_type   = FIELD_ASCENTRATE;
    config.screenB_bottom_right_type  = FIELD_SLOPE;
  }
#ifdef PBL_HEALTH
  health_init_if_needed();
#endif
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
  persist_write_int(PERSIST_VERSION, VERSION_PEBBLE);
}
void config_affect_type(FieldLayer *field, uint8_t type) {
  field->type = type;
  field->type_index = 0;
  for(int i = 0; i < CONFIG_NB_FIELD_ORDER; i++) {
    if (config_order[i] == type) {
      field->type_index = i;
      break;
    }
  }
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "type %d -> index: %d", field->type, field->type_index);
}
