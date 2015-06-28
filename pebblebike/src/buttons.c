#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "buttons.h"
#include "communication.h"
#include "screens.h"
#include "screen_map.h"
#include "screen_live.h"
#include "screen_speed.h"
#include "menu.h"

GBitmap *start_button;
GBitmap *stop_button;
//GBitmap *reset_button;
GBitmap *menu_button;
GBitmap *zoom_button;
GBitmap *next_button;
GBitmap *menu_up_button;
GBitmap *menu_down_button;

enum {
  CONFIG_FIELD_DISABLED,
  CONFIG_FIELD__MIN,
  CONFIG_FIELD_TOP = CONFIG_FIELD__MIN,
  CONFIG_FIELD_BOTTOM_LEFT,
  CONFIG_FIELD_BOTTOM_RIGHT,
  CONFIG_FIELD__MAX,
};
FieldLayer *cur_field;
uint8_t config_field= CONFIG_FIELD_DISABLED;
void config_start();
void config_stop();
void config_change_field();
void config_change_type();
static AppTimer *config_timer;

void handle_topbutton_longclick(ClickRecognizerRef recognizer, void *context) {
  vibes_short_pulse();
  send_cmd(REFRESH_PRESS);
}
void handle_topbutton_click(ClickRecognizerRef recognizer, void *context) {
  if (s_data.page_number == PAGE_LIVE_TRACKING) {
    screen_live_menu(true);
  } else {
    if (s_data.state == STATE_STOP) {
        send_cmd(PLAY_PRESS);
    } else {
        send_cmd(STOP_PRESS);
    }
  }
}
void handle_selectbutton_click(ClickRecognizerRef recognizer, void *context) {
  if (config_field != CONFIG_FIELD_DISABLED) {
    config_change_field();
  } else {
    int prev_page_number = s_data.page_number;
    s_data.page_number++;

    if (s_data.page_number == PAGE_HEARTRATE && s_gpsdata.heartrate == 255) {
      s_data.page_number++;
    }
    if (s_data.page_number == PAGE_LIVE_TRACKING && s_data.live == 0) {
      s_data.page_number++;
    }
    if (!s_data.debug) {
      if (s_data.page_number == PAGE_DEBUG1 || s_data.page_number == PAGE_DEBUG2) {
        // debug option not checked on android app
        s_data.page_number = PAGE_FIRST;
      }
    }
    if (s_data.page_number >= NUMBER_OF_PAGES) {
      s_data.page_number = PAGE_FIRST;
    }

    if (prev_page_number == PAGE_LIVE_TRACKING) {
      buttons_update();
      action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, menu_button);
    } else if (s_data.page_number == PAGE_LIVE_TRACKING) {
      action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, menu_up_button);
      action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, menu_down_button);
    }
    if (prev_page_number == PAGE_MAP) {
      action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, menu_button);
    } else if (s_data.page_number == PAGE_MAP) {
      action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, zoom_button);
    }
    if (s_data.page_number == PAGE_SPEED || s_data.page_number == PAGE_HEARTRATE) {
      screen_speed_show_speed(true);
    }
    update_screens();
  }
}

void handle_bottombutton_click(ClickRecognizerRef recognizer, void *context) {
  if (s_data.page_number == PAGE_MAP) {
    screen_map_zoom_out(2);
  } else if (s_data.page_number == PAGE_LIVE_TRACKING) {
    screen_live_menu(false);
  } else if (config_field != CONFIG_FIELD_DISABLED) {
    config_change_type();
  } else {
    menu_show();
  }
}
void handle_selectbutton_longclick(ClickRecognizerRef recognizer, void *context) {
  screen_speed_start_rotation();
}
void handle_bottombutton_longclick(ClickRecognizerRef recognizer, void *context) {
  if (s_data.page_number == PAGE_MAP) {
    screen_map_zoom_in(2);
  } else {
    config_start();
  }
}
void handle_backbutton_click(ClickRecognizerRef recognizer, void *context) {
  if (config_field != CONFIG_FIELD_DISABLED) {
    config_stop();
  } else {
    // do nothing
    // just prevent to leave the app
    // TODO: show notif
  }
}
void handle_backbutton_exit(ClickRecognizerRef recognizer, void *context) {
  window_stack_pop(true);
}

void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_DOWN, handle_bottombutton_click);
  window_single_click_subscribe(BUTTON_ID_SELECT, handle_selectbutton_click);
  window_single_click_subscribe(BUTTON_ID_UP, handle_topbutton_click);
  window_single_click_subscribe(BUTTON_ID_BACK, handle_backbutton_click);

  // long click config:
  window_long_click_subscribe(BUTTON_ID_DOWN, 1000, handle_bottombutton_longclick, NULL /* No handler on button release */);
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, handle_selectbutton_longclick, NULL /* No handler on button release */);
  window_long_click_subscribe(BUTTON_ID_UP, 1000, handle_topbutton_longclick, NULL /* No handler on button release */);
  window_multi_click_subscribe(BUTTON_ID_BACK, 2, 2, 300, true, handle_backbutton_exit);
}

void buttons_update() {
  if (s_data.page_number == PAGE_LIVE_TRACKING) {
    // MenuLayer
    return;
  }
  if(s_data.state == STATE_STOP) {
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, start_button);
  } else if(s_data.state == STATE_START) {
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, stop_button);
  } else {
    // bug?
    //vibes_short_pulse();
  }
}

void buttons_init() {
  
  start_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_START_BUTTON);
  stop_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STOP_BUTTON);
  //reset_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RESET_BUTTON);
  menu_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_BUTTON);
  zoom_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ZOOM_BUTTON);
  next_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NEXT_BUTTON);
  menu_up_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_UP_BUTTON);
  menu_down_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_DOWN_BUTTON);
  
  s_data.page_number = PAGE_FIRST;
}
void buttons_deinit() {

  gbitmap_destroy(start_button);
  gbitmap_destroy(stop_button);
  //gbitmap_destroy(reset_button);
  gbitmap_destroy(menu_button);
  gbitmap_destroy(zoom_button);
  gbitmap_destroy(next_button);
  gbitmap_destroy(menu_up_button);
  gbitmap_destroy(menu_down_button);
}
bool config_hidden = false;
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
