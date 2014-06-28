#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screens.h"
#include "buttons.h"

ActionBarLayer *action_bar;
static AppTimer *disconnect_timer;

void update_screens() {
  layer_set_hidden(s_data.page_speed, true);
  layer_set_hidden(s_data.page_altitude, true);
  layer_set_hidden(menu_layer_get_layer(s_data.page_live_tracking), true);
  layer_set_hidden(s_data.page_map, true);

  #if DEBUG
    layer_set_hidden(s_data.page_debug1, true);
    layer_set_hidden(s_data.page_debug2, true);
  #endif
  if (s_data.page_number == PAGE_SPEED) {
    layer_set_hidden(s_data.page_speed, false);
//    layer_mark_dirty(&s_data.speed_layer.layer);
//    layer_mark_dirty(&s_data.distance_layer.layer);
//    layer_mark_dirty(&s_data.avgspeed_layer.layer);
  }
  if (s_data.page_number == PAGE_HEARTRATE) {
    layer_set_hidden(s_data.page_speed, false);
  }
  if (s_data.page_number == PAGE_ALTITUDE) {
    layer_set_hidden(s_data.page_altitude, false);
  }
  if (s_data.page_number == PAGE_LIVE_TRACKING) {
    layer_set_hidden(menu_layer_get_layer(s_data.page_live_tracking), false);
  }
  if (s_data.page_number == PAGE_MAP) {
    layer_set_hidden(s_data.page_map, false);
    layer_mark_dirty(s_data.page_map); // TODO: really needed?
    //vibes_short_pulse();
  }
  #if DEBUG
    if (s_data.page_number == PAGE_DEBUG1) {
      layer_set_hidden(s_data.page_debug1, false);
      layer_mark_dirty(s_data.page_debug1); // TODO: really needed?
    }
    if (s_data.page_number == PAGE_DEBUG2) {
      layer_set_hidden(s_data.page_debug2, false);
      layer_mark_dirty(s_data.page_debug2); // TODO: really needed?
    }
  #endif
}

void set_layer_attr_full(TextLayer *textlayer, const char *text, GFont font, GTextAlignment text_alignment, GColor color, GColor bg_color, Layer *ParentLayer)
{
  text_layer_set_text(textlayer, text);
  text_layer_set_text_alignment(textlayer, text_alignment);
  text_layer_set_text_color(textlayer, color);
  text_layer_set_background_color(textlayer, bg_color);
  text_layer_set_font(textlayer, font);
  if (ParentLayer != NULL) {
      layer_add_child(ParentLayer, text_layer_get_layer(textlayer));
  }
}

void field_layer_init(Layer* parent, FieldLayer* field_layer, int16_t x, int16_t y, int16_t w, int16_t h, char* title_text, char* data_text, char* unit_text) {
  field_layer->main_layer = layer_create(GRect(x, y, w, h));
  layer_add_child(parent, field_layer->main_layer);


  // title
  field_layer->title_layer = text_layer_create(GRect(1, 2, w - 2, 14));
  set_layer_attr_full(field_layer->title_layer, title_text, font_12, GTextAlignmentCenter, GColorBlack, GColorClear, field_layer->main_layer);

  // data
  field_layer->data_layer = text_layer_create(GRect(1, 21, w - 2, 32));
  set_layer_attr_full(field_layer->data_layer, data_text, font_24, GTextAlignmentCenter, GColorBlack, GColorClear, field_layer->main_layer);

  // unit
  field_layer->unit_layer = text_layer_create(GRect(1, h - 14, w - 2, 14));
  set_layer_attr_full(field_layer->unit_layer, unit_text, font_12, GTextAlignmentCenter, GColorBlack, GColorClear, field_layer->main_layer);

}
void field_layer_deinit(FieldLayer* field_layer) {
  layer_destroy(field_layer->main_layer);
  text_layer_destroy(field_layer->title_layer);
  text_layer_destroy(field_layer->data_layer);
  text_layer_destroy(field_layer->unit_layer);
}

void topbar_layer_init(Window* window) {
  int16_t w = SCREEN_W - MENU_WIDTH;

  s_data.topbar_layer.layer = layer_create(GRect(0,0,w,SCREEN_H));
  layer_add_child(window_get_root_layer(window), s_data.topbar_layer.layer);

  // time (centered in top bar)
  s_data.topbar_layer.time_layer = text_layer_create(GRect(0,0,w,TOPBAR_HEIGHT));
  set_layer_attr_full(s_data.topbar_layer.time_layer, s_data.time, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_16)), GTextAlignmentCenter, GColorClear, GColorBlack, window_get_root_layer(window));

  // bluetooth icon
  s_data.topbar_layer.bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
  s_data.topbar_layer.bluetooth_layer = bitmap_layer_create(s_data.topbar_layer.bluetooth_image->bounds);
  bitmap_layer_set_bitmap(s_data.topbar_layer.bluetooth_layer, s_data.topbar_layer.bluetooth_image);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer), !bluetooth_connection_service_peek());

  // accuracy (1/3, right)
  strcpy(s_data.accuracy, "-");
  s_data.topbar_layer.accuracy_layer = text_layer_create(GRect(w*2/3,0,w/3,TOPBAR_HEIGHT));
  set_layer_attr_full(s_data.topbar_layer.accuracy_layer, s_data.accuracy, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_16)), GTextAlignmentRight, GColorClear, GColorBlack, window_get_root_layer(window));

}

static void disconnect_timer_callback(void *data) {
  disconnect_timer = NULL;
  vibes_short_pulse();
}

void topbar_toggle_bluetooth_icon(bool connected) {
  layer_set_hidden(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer), !connected);
  if (connected) {
    //vibes_short_pulse();
    if (disconnect_timer) {
      app_timer_cancel(disconnect_timer);
    }
  } else {
    if (s_data.debug) {
      vibes_short_pulse();
    } else {
      // schedule a timer to viber in X milliseconds
      disconnect_timer = app_timer_register(5000, disconnect_timer_callback, NULL);
    }
  }
}

void topbar_layer_deinit() {
  layer_destroy(s_data.topbar_layer.layer);
  text_layer_destroy(s_data.topbar_layer.time_layer);
  text_layer_destroy(s_data.topbar_layer.accuracy_layer);
  if (disconnect_timer) {
    app_timer_cancel(disconnect_timer);
  }
  layer_remove_from_parent(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer));
  bitmap_layer_destroy(s_data.topbar_layer.bluetooth_layer);
  gbitmap_destroy(s_data.topbar_layer.bluetooth_image);
}  
void action_bar_init(Window* window) {
  // Initialize the action bar:
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, start_button);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, next_button);
  //action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, reset_buttonp);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, menu_button);
}
void action_bar_deinit() {
 action_bar_layer_destroy(action_bar);
}
