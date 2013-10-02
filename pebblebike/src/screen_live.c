#include "pebble_os.h"
#include "config.h"
#include "pebblebike.h"
#include "screen_live.h"

void screen_live_layer_init(Window* window) {
  layer_init(&s_data.page_live_tracking, GRect(0,TOPBAR_HEIGHT,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
  layer_add_child(&window->layer, &s_data.page_live_tracking);

  text_layer_init(&s_data.live_tracking_layer, GRect(0,0,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
  text_layer_set_text_color(&s_data.live_tracking_layer, GColorBlack);
  text_layer_set_background_color(&s_data.live_tracking_layer, GColorClear);
  text_layer_set_font(&s_data.live_tracking_layer, font_18);
  text_layer_set_text_alignment(&s_data.live_tracking_layer, GTextAlignmentLeft);
  text_layer_set_text(&s_data.live_tracking_layer, s_data.friends);
  layer_add_child(&s_data.page_live_tracking, &s_data.live_tracking_layer.layer);

  layer_set_hidden(&s_data.page_live_tracking, true);
  
  strncpy(
    s_data.friends, 
    "+ Live Tracking +\n"
    "Setup your account\n\n"
    "Or join the beta:\n"
    "pebblebike.com\n"
    "/live",
    90-1
  );
  
  s_live.nb = 0;
  for(int i = 0; i < NUM_LIVE_FRIENDS; i++) {
    //snprintf(s_live.friends[i].name, sizeof(s_live.friends[i].name), "fr%d", i);
    strcpy(s_live.friends[i].name, "");
  }
}