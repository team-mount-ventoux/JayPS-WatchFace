#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screen_debug.h"
#include "screens.h"

//#if DEBUG
void screen_debug1_layer_init(Window* window) {
  s_data.page_debug1 = layer_create(GRect(0,TOPBAR_HEIGHT,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
  layer_add_child(window_get_root_layer(window), s_data.page_debug1);

  s_data.debug1_layer = text_layer_create(GRect(0,0,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
  set_layer_attr_full(s_data.debug1_layer, s_data.debug1, font_18, GTextAlignmentLeft, GColorBlack, GColorClear, s_data.page_debug1);


  layer_set_hidden(s_data.page_debug1, true);
}
void screen_debug1_layer_deinit() {
  layer_destroy(s_data.page_debug1);
  text_layer_destroy(s_data.debug1_layer);
}

void screen_debug2_layer_init(Window* window) {
  s_data.page_debug2 = layer_create(GRect(0,TOPBAR_HEIGHT,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
  layer_add_child(window_get_root_layer(window), s_data.page_debug2);

  s_data.debug2_layer = text_layer_create(GRect(0,0,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
  set_layer_attr_full(s_data.debug2_layer, s_data.debug2, font_18, GTextAlignmentLeft, GColorBlack, GColorClear, s_data.page_debug2);

  layer_set_hidden(s_data.page_debug2, true);
}

void screen_debug2_layer_deinit() {
  layer_destroy(s_data.page_debug2);
  text_layer_destroy(s_data.debug2_layer);
}

//#endif
