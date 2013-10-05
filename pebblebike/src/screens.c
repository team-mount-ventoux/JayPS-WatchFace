#include "pebble_os.h"
#include "config.h"
#include "pebblebike.h"
#include "screens.h"
#include "buttons.h"

ActionBarLayer action_bar;

void update_screens() {
  layer_set_hidden(&s_data.page_speed, true);
  layer_set_hidden(&s_data.page_altitude, true);
  layer_set_hidden(&s_data.page_live_tracking, true);
  layer_set_hidden(&s_data.page_map, true);

  #if DEBUG
  layer_set_hidden(&s_data.page_debug1, true);
  layer_set_hidden(&s_data.page_debug2, true);
  #endif
  if (s_data.page_number == PAGE_SPEED) {
    layer_set_hidden(&s_data.page_speed, false);
//    layer_mark_dirty(&s_data.speed_layer.layer);
//    layer_mark_dirty(&s_data.distance_layer.layer);
//    layer_mark_dirty(&s_data.avgspeed_layer.layer);
  }
  if (s_data.page_number == PAGE_ALTITUDE) {
    layer_set_hidden(&s_data.page_altitude, false);
  }
  if (s_data.page_number == PAGE_LIVE_TRACKING) {
    layer_set_hidden(&s_data.page_live_tracking, false);
  }
  if (s_data.page_number == PAGE_MAP) {
    layer_set_hidden(&s_data.page_map, false);
      layer_mark_dirty(&s_data.page_map); // TODO: really needed?
      //vibes_short_pulse();
  }
  #if DEBUG
  if (s_data.page_number == PAGE_DEBUG1) {
    layer_set_hidden(&s_data.page_debug1, false);
      layer_mark_dirty(&s_data.page_debug1); // TODO: really needed?
  }
  if (s_data.page_number == PAGE_DEBUG2) {
    layer_set_hidden(&s_data.page_debug2, false);
      layer_mark_dirty(&s_data.page_debug2); // TODO: really needed?
  }
  #endif
}


void field_layer_init(Layer* parent, FieldLayer* field_layer, int16_t x, int16_t y, int16_t w, int16_t h, char* title_text, char* data_text, char* unit_text) {
  layer_init(&field_layer->main_layer, GRect(x, y, w, h));
  layer_add_child(parent, &field_layer->main_layer);


  // title
  text_layer_init(&field_layer->title_layer, GRect(1, 2, w - 2, 14));
  text_layer_set_text(&field_layer->title_layer, title_text);
  text_layer_set_text_color(&field_layer->title_layer, GColorBlack);
  text_layer_set_background_color(&field_layer->title_layer, GColorClear);
  text_layer_set_font(&field_layer->title_layer, font_12);
  text_layer_set_text_alignment(&field_layer->title_layer, GTextAlignmentCenter);
  layer_add_child(&field_layer->main_layer, &field_layer->title_layer.layer);

  // data
  text_layer_init(&field_layer->data_layer, GRect(1, 21, w - 2, 32));
  text_layer_set_text_color(&field_layer->data_layer, GColorBlack);
  text_layer_set_background_color(&field_layer->data_layer, GColorClear);
  text_layer_set_font(&field_layer->data_layer, font_24);
  text_layer_set_text_alignment(&field_layer->data_layer, GTextAlignmentCenter);
  text_layer_set_text(&field_layer->data_layer, data_text);
  layer_add_child(&field_layer->main_layer, &field_layer->data_layer.layer);

  // unit
  text_layer_init(&field_layer->unit_layer, GRect(1, h - 14, w - 2, 14));
  text_layer_set_text(&field_layer->unit_layer, unit_text);
  text_layer_set_text_color(&field_layer->unit_layer, GColorBlack);
  text_layer_set_background_color(&field_layer->unit_layer, GColorClear);
  text_layer_set_font(&field_layer->unit_layer, font_12);
  text_layer_set_text_alignment(&field_layer->unit_layer, GTextAlignmentCenter);
  layer_add_child(&field_layer->main_layer, &field_layer->unit_layer.layer);

}


void topbar_layer_init(Window* window) {
  int16_t w = SCREEN_W - MENU_WIDTH;

  layer_init(&s_data.topbar_layer.layer, GRect(0,0,w,SCREEN_H));
  layer_add_child(&window->layer, &s_data.topbar_layer.layer);

  // time (centered in top bar)
  text_layer_init(&s_data.topbar_layer.time_layer, GRect(0,0,w,TOPBAR_HEIGHT));
  text_layer_set_text(&s_data.topbar_layer.time_layer, s_data.time);
  text_layer_set_text_color(&s_data.topbar_layer.time_layer, GColorClear);
  text_layer_set_background_color(&s_data.topbar_layer.time_layer, GColorBlack);
  text_layer_set_font(&s_data.topbar_layer.time_layer, font_12);
  text_layer_set_text_alignment(&s_data.topbar_layer.time_layer, GTextAlignmentCenter);
  layer_add_child(&window->layer, &s_data.topbar_layer.time_layer.layer);

  // accuracy (1/3, right)
  strcpy(s_data.accuracy, "-");
  text_layer_init(&s_data.topbar_layer.accuracy_layer, GRect(w*2/3,0,w/3,TOPBAR_HEIGHT));
  text_layer_set_text(&s_data.topbar_layer.accuracy_layer, s_data.accuracy);
  text_layer_set_text_color(&s_data.topbar_layer.accuracy_layer, GColorClear);
  text_layer_set_background_color(&s_data.topbar_layer.accuracy_layer, GColorBlack);
  text_layer_set_font(&s_data.topbar_layer.accuracy_layer, font_12);
  text_layer_set_text_alignment(&s_data.topbar_layer.accuracy_layer, GTextAlignmentRight);
  layer_add_child(&window->layer, &s_data.topbar_layer.accuracy_layer.layer);

}
void action_bar_init(Window* window) {
  // Initialize the action bar:
  action_bar_layer_init(&action_bar);
  action_bar_layer_add_to_window(&action_bar, window);
  action_bar_layer_set_click_config_provider(&action_bar,
                                             click_config_provider);

  action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &start_button.bmp);
  //action_bar_layer_set_icon(&action_bar, BUTTON_ID_DOWN, &reset_button.bmp);
  action_bar_layer_set_icon(&action_bar, BUTTON_ID_DOWN, &menu_button.bmp);
}