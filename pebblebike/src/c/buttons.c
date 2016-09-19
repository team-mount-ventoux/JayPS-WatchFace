#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "buttons.h"
#include "communication.h"
#include "screens.h"
#include "ovl/screen_map.h"
#ifdef ENABLE_FUNCTION_LIVE
  #include "screen_live.h"
#endif
#include "screen_data.h"
#include "screen_config.h"
#include "ovl/screen_config.h"

#include "overlord.h"
#include "overlays.h"
#include "ovl/menu.h"


GBitmap *start_button;
GBitmap *stop_button;
//GBitmap *reset_button;
GBitmap *menu_button;
GBitmap *zoom_button;
GBitmap *next_button;
GBitmap *menu_up_button;
GBitmap *menu_down_button;
static AppTimer *button_timer;
#ifdef ENABLE_ROTATION
  static AppTimer *rotation_timer;
#endif
static void button_timer_callback(void *data) {
  button_timer = NULL;
  layer_set_hidden(action_bar_layer_get_layer(action_bar), true);
  title_instead_of_units = false;
  //todo
  screen_data_update_config(false);
  //screen_altitude_update_config();
}
void button_click() {
  layer_set_hidden(action_bar_layer_get_layer(action_bar), false);
  if (button_timer) {
    app_timer_cancel(button_timer);
  }
  // schedule a timer to hide action bar in X milliseconds
  button_timer = app_timer_register(2000, button_timer_callback, NULL);
}

void action_bar_set_menu_up_down_buttons() {
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, menu_up_button);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, menu_down_button);
}

void handle_topbutton_longclick(ClickRecognizerRef recognizer, void *context) {
  button_click();
  vibes_short_pulse();
  send_cmd(REFRESH_PRESS);
}
void handle_topbutton_click(ClickRecognizerRef recognizer, void *context) {
  button_click();
#ifdef ENABLE_FUNCTION_LIVE
  if (s_data.page_number == PAGE_LIVE_TRACKING) {
    screen_live_menu(true);
  } else
#endif
  if (config_screen != CONFIG_SCREEN_DISABLED) {
    config_change_type(CONFIG_CHANGE_TYPE_PREVIOUS);
  } else {
    if (s_data.state == STATE_STOP) {
        send_cmd(PLAY_PRESS);
    } else {
        send_cmd(STOP_PRESS);
    }
  }
}
void next_page(bool rotation) {
  uint8_t prev_page_number = s_data.page_number;
  s_data.page_number++;

  if (s_data.page_number == PAGE_HEARTRATE && (s_gpsdata.heartrate == 255 || config.screenA_top_type == FIELD_HEARTRATE)) {
    s_data.page_number++;
  }
#ifdef ENABLE_FUNCTION_LIVE
  if (s_data.page_number == PAGE_LIVE_TRACKING) {
    if (s_data.live == 0) {
      s_data.page_number++;
    }
  }
#endif
  if (s_data.page_number >= NUMBER_OF_PAGES) {
    s_data.page_number = PAGE_DATA;
  }
#ifdef ENABLE_ROTATION
  if (rotation && s_gpsdata.nav_distance_to_destination100 == 0 && s_data.page_number > PAGE_ALTITUDE) {
    s_data.page_number = PAGE_DATA;
  }
#endif
  s_data.data_subpage = SUBPAGE_UNDEF;
  if (s_data.page_number == PAGE_DATA || s_data.page_number == PAGE_HEARTRATE || s_data.page_number == PAGE_ALTITUDE) {
    s_data.data_subpage = s_data.page_number == PAGE_ALTITUDE ? SUBPAGE_B : SUBPAGE_A;
    title_instead_of_units = true;
    screen_data_update_config(true);
  } else {
    config_field_set_text(s_data.topbar_layer.field_center_layer, FIELD_TIME, GTextAlignmentCenter);
  }
#ifdef ENABLE_FUNCTION_LIVE
  if (prev_page_number == PAGE_LIVE_TRACKING) {
    buttons_update();
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, menu_button);
  } else if (s_data.page_number == PAGE_LIVE_TRACKING) {
    action_bar_set_menu_up_down_buttons();
  }
#endif
  if (prev_page_number == PAGE_MAP) {
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, menu_button);
    screen_map_layer_deinit();
  } else if (s_data.page_number == PAGE_MAP) {
    overlay_load(OVL_MAP_OVL);
    screen_map_layer_init(s_data.window);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, zoom_button);
  }

  update_screens();
  LOG_DEBUG("page:%d sp:%d", s_data.page_number, s_data.data_subpage);
}
#ifdef ENABLE_ROTATION
static void rotation_timer_callback(void *data) {
  if ((s_data.page_number <= PAGE_ALTITUDE || s_gpsdata.nav_distance_to_destination100 > 0) && config_screen == CONFIG_SCREEN_DISABLED) {
    next_page(true);
    // restore title_instead_of_units false in 1s
    button_timer = app_timer_register(1000, button_timer_callback, NULL);
  }
  // register next rotation in 5s
  rotation_timer = app_timer_register(5000, rotation_timer_callback, NULL);
}
#endif
void handle_selectbutton_click(ClickRecognizerRef recognizer, void *context) {
  button_click();
  if (config_screen != CONFIG_SCREEN_DISABLED) {
    config_change_field();
  } else {
    next_page(false);
  }
}
void handle_selectbutton_longclick(ClickRecognizerRef recognizer, void *context) {
  button_click();
#ifdef ENABLE_ROTATION
  vibes_short_pulse();
  if (rotation_timer == NULL) {
    rotation_timer = app_timer_register(5000, rotation_timer_callback, NULL);
  } else {
    app_timer_cancel(rotation_timer);
    rotation_timer = NULL;
  }
#endif
}
void handle_bottombutton_click(ClickRecognizerRef recognizer, void *context) {
  button_click();
  if (s_data.page_number == PAGE_MAP) {
    screen_map_zoom_out(2);
  } else
#ifdef ENABLE_FUNCTION_LIVE
  if (s_data.page_number == PAGE_LIVE_TRACKING) {
    screen_live_menu(false);
  } else
#endif
  if (config_screen != CONFIG_SCREEN_DISABLED) {
    config_change_type(CONFIG_CHANGE_TYPE_NEXT);
  } else {
    need_launch_config = false;
    overlay_load(OVL_MENU_OVL);
    menu_init();
    menu_show();
  }
#ifdef ENABLE_DEMO
//  s_gpsdata.heartrate += 8;
//  snprintf(s_data.heartrate, 5, "%d", s_gpsdata.heartrate);
//  screen_data_update_config(true);
#endif
}
void handle_bottombutton_longclick(ClickRecognizerRef recognizer, void *context) {
  button_click();
  if (s_data.page_number == PAGE_MAP) {
    screen_map_zoom_in(2);
  } else if (s_data.page_number == PAGE_DATA || s_data.page_number == PAGE_ALTITUDE) {
    if (config_screen == CONFIG_SCREEN_DISABLED) {
      overlay_load(OVL_CONFIG_OVL);
      config_start();
    } else {
      config_stop();
      buttons_update();
      action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, menu_button);
    }
  }
}
void handle_backbutton_click(ClickRecognizerRef recognizer, void *context) {
  button_click();
  if (config_screen != CONFIG_SCREEN_DISABLED) {
    config_stop();
    buttons_update();
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, menu_button);
  } else {
    // do nothing
    // just prevent to leave the app
    // TODO: show notif

    title_instead_of_units = true;
    screen_data_update_config(true);
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
#ifdef ENABLE_FUNCTION_LIVE
  if (s_data.page_number == PAGE_LIVE_TRACKING) {
    // MenuLayer
    return;
  }
#endif
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
  button_timer = app_timer_register(2000, button_timer_callback, NULL);
  start_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_START_BUTTON);
  stop_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STOP_BUTTON);
  //reset_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RESET_BUTTON);
  menu_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_BUTTON);
  zoom_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ZOOM_BUTTON);
  next_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NEXT_BUTTON);
  menu_up_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_UP_BUTTON);
  menu_down_button = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_DOWN_BUTTON);
  
  s_data.page_number = PAGE_DATA;
  s_data.data_subpage = SUBPAGE_A;
}
void buttons_deinit() {
  if (button_timer) {
    app_timer_cancel(button_timer);
  }
#ifdef ENABLE_ROTATION
  if (rotation_timer) {
    app_timer_cancel(rotation_timer);
  }
#endif
  gbitmap_destroy(start_button);
  gbitmap_destroy(stop_button);
  //gbitmap_destroy(reset_button);
  gbitmap_destroy(menu_button);
  gbitmap_destroy(zoom_button);
  gbitmap_destroy(next_button);
  gbitmap_destroy(menu_up_button);
  gbitmap_destroy(menu_down_button);
}
