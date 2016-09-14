#include "pebble.h"
#include "../config.h"
#include "../pebblebike.h"
#include "../screens.h"
#include "../buttons.h"
#include "../screen_config.h"
#include "../heartrate.h"
#include "../screen_data.h"
#include "screen_config.h"
#ifdef ENABLE_LOCALIZE
  #include "../localize.h"
#endif
#ifdef PBL_HEALTH
  #include "../health.h"
#endif

FieldConfig *cur_fieldconfig;
FieldLayer *cur_fieldlayer;
bool config_hidden = false;
static AppTimer *config_timer;
uint8_t config_field = CONFIG_FIELD_SCREEN__MIN;

void config_change_visibility(FieldLayer* field_layer, bool hidden) {
//  if (field_layer->title_layer != NULL) {
//    layer_set_hidden(text_layer_get_layer(field_layer->title_layer), hidden);
//  }
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
  config_change_visibility(cur_fieldlayer, config_hidden);
}
void config_start() {
  if (s_data.data_subpage == SUBPAGE_UNDEF) {
    return;
  }

#ifdef PBL_HEALTH
  health_init();
#endif

  config_field = CONFIG_FIELD_SCREEN_TOP;
  config_screen = s_data.data_subpage == SUBPAGE_B ? CONFIG_SCREEN_B : CONFIG_SCREEN_A;
  cur_fieldconfig = &s_data.screen_config[s_data.data_subpage].field_top;
  screen_data_update_config(false);
  cur_fieldlayer = &s_data.screenData_layer.field_top;
  layer_set_hidden(text_layer_get_layer(s_data.topbar_layer.accuracy_layer), true);
  layer_set_hidden(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer), true);
  text_layer_set_text(s_data.topbar_layer.field_center_layer.data_layer, field_get_title(cur_fieldconfig->type));
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
  config_change_visibility(cur_fieldlayer, false);
  app_timer_cancel(config_timer);
  config_timer = NULL;
  config_field_set_text(s_data.topbar_layer.field_center_layer, s_data.screen_config[s_data.data_subpage].field_topbar_center.type, GTextAlignmentCenter);
  layer_set_hidden(text_layer_get_layer(s_data.topbar_layer.accuracy_layer), s_data.state == STATE_STOP);
  layer_set_hidden(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer), false);
  layer_mark_dirty(s_data.topbar_layer.layer);
  config_save();
#ifdef PBL_HEALTH
  health_init_if_needed();
#endif
}
void config_change_field() {
  config_change_visibility(cur_fieldlayer, false);
  config_field++;
  if (config_screen == CONFIG_SCREEN_A && config_field == CONFIG_FIELD_SCREEN__MAX) {
    config_field = CONFIG_FIELD_SCREEN__MIN;
  } else if (config_screen == CONFIG_SCREEN_B && config_field == CONFIG_FIELD_SCREEN__MAX) {
    config_field = CONFIG_FIELD_SCREEN__MIN;
  }
  switch (config_field) {
    case CONFIG_FIELD_SCREEN_TOP:           cur_fieldconfig = &s_data.screen_config[s_data.data_subpage].field_top;          cur_fieldlayer = &s_data.screenData_layer.field_top; break;
    case CONFIG_FIELD_SCREEN_TOP2:          cur_fieldconfig = &s_data.screen_config[s_data.data_subpage].field_top2;         cur_fieldlayer = &s_data.screenData_layer.field_top2; break;
    case CONFIG_FIELD_SCREEN_TOP_BAR:       cur_fieldconfig = &s_data.screen_config[s_data.data_subpage].field_topbar_center;         cur_fieldlayer = &s_data.topbar_layer.field_center_layer; break;
    case CONFIG_FIELD_SCREEN_BOTTOM_LEFT:   cur_fieldconfig = &s_data.screen_config[s_data.data_subpage].field_bottom_left;  cur_fieldlayer = &s_data.screenData_layer.field_bottom_left; break;
    case CONFIG_FIELD_SCREEN_BOTTOM_RIGHT:  cur_fieldconfig = &s_data.screen_config[s_data.data_subpage].field_bottom_right; cur_fieldlayer = &s_data.screenData_layer.field_bottom_right; break;
  }

  config_change_visibility(cur_fieldlayer, true);

  text_layer_set_text(s_data.topbar_layer.field_center_layer.data_layer, field_get_title(cur_fieldconfig->type));
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
  cur_fieldconfig->type_index++;
  if ((cur_fieldconfig->type_index >= CONFIG_NB_FIELD_ORDER) || (config_order[cur_fieldconfig->type_index] == FIELD__UNUSED)) {
    cur_fieldconfig->type_index = 0;
  }
}
void config_next_type_graph(bool can_display_graph) {
  config_next_type();
  if (!can_display_graph && config_field_contain_graph(cur_fieldconfig->type_index)) {
    config_next_type();
  }
  // twice because _DATA_AND_GRAPH and _ALTITUDE_GRAPH_ONLY,
  if (!can_display_graph && config_field_contain_graph(cur_fieldconfig->type_index)) {
    config_next_type();
  }
}

void config_previous_type() {
  if (cur_fieldconfig->type_index == 0) {
    // search for last index not FIELD__UNUSED
    for (int i = 0; i < CONFIG_NB_FIELD_ORDER; i++) {
      if (config_order[i] == FIELD__UNUSED) {
        cur_fieldconfig->type_index = i -1 ;
        return;
      }
    }
  } else {
    cur_fieldconfig->type_index--;
  }
}
void config_previous_type_graph(bool can_display_graph) {
  config_previous_type();
  if (!can_display_graph && config_field_contain_graph(cur_fieldconfig->type_index)) {
    config_previous_type();
  }
  // twice because _DATA_AND_GRAPH and _ALTITUDE_GRAPH_ONLY,
  if (!can_display_graph && config_field_contain_graph(cur_fieldconfig->type_index)) {
    config_previous_type();
  }
}


void config_change_type(uint8_t direction) {
  if (direction == CONFIG_CHANGE_TYPE_NEXT) {
    config_next_type_graph(config_field == CONFIG_FIELD_SCREEN_TOP2);
    cur_fieldconfig->type = config_order[cur_fieldconfig->type_index];
  } else {
    // CONFIG_CHANGE_TYPE_PREVIOUS
    config_previous_type_graph(config_field == CONFIG_FIELD_SCREEN_TOP2);
    cur_fieldconfig->type = config_order[cur_fieldconfig->type_index];
  }
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "%d: type %d", cur_fieldconfig->type_index, cur_fieldconfig->type);

  //todo
  screen_data_update_config(false);
  text_layer_set_text(s_data.topbar_layer.field_center_layer.data_layer, field_get_title(cur_fieldconfig->type));
  layer_mark_dirty(s_data.topbar_layer.layer);
  //LOG_DEBUG("config_change_type newtype=%d [index:%d]", cur_fieldconfig->type, cur_fieldconfig->type_index);
}

