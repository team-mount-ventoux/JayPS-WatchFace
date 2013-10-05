#include "pebble_os.h"
#include "config.h"
#include "pebblebike.h"
#include "screens.h"
#include "screen_altitude.h"


void page_altitude_update_proc(Layer *page_altitude, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, GColorBlack);

  // vertical line
  graphics_draw_line(ctx, GPoint((SCREEN_W - MENU_WIDTH) / 2, TOPBAR_HEIGHT), GPoint((SCREEN_W - MENU_WIDTH) / 2, SCREEN_H));

  // horizontal line
  graphics_draw_line(ctx, GPoint(0, TOPBAR_HEIGHT + (SCREEN_H - TOPBAR_HEIGHT) / 2), GPoint(SCREEN_W - MENU_WIDTH, TOPBAR_HEIGHT + (SCREEN_H - TOPBAR_HEIGHT) / 2));
}


void screen_altitude_layer_init(Window* window) {
  layer_init(&s_data.page_altitude, GRect(0,0,SCREEN_W-MENU_WIDTH,SCREEN_H));
  s_data.page_altitude.update_proc = &page_altitude_update_proc;
  layer_add_child(&window->layer, &s_data.page_altitude);

  strcpy(s_data.altitude, "-");
  strcpy(s_data.ascent, "-");
  strcpy(s_data.ascentrate, "-");
  strcpy(s_data.slope, "-");
  
  int16_t w = (SCREEN_W - MENU_WIDTH) / 2; //61
  int16_t h = (SCREEN_H - TOPBAR_HEIGHT) / 2 - 1; // 75

  field_layer_init(&s_data.page_altitude, &s_data.altitude_layer,       0,     TOPBAR_HEIGHT + 0,     w, h, "Altitude", s_data.altitude, s_data.altitude_layer.units);
  field_layer_init(&s_data.page_altitude, &s_data.altitude_ascent,      w + 1, TOPBAR_HEIGHT + 0,     w, h, "Ascent", s_data.ascent, s_data.altitude_ascent.units);
  field_layer_init(&s_data.page_altitude, &s_data.altitude_ascent_rate, 0,     TOPBAR_HEIGHT + h + 1, w, h, "Ascent rate", s_data.ascentrate, s_data.altitude_ascent_rate.units);
  field_layer_init(&s_data.page_altitude, &s_data.altitude_slope,       w + 1, TOPBAR_HEIGHT + h + 1, w, h, "Slope", s_data.slope, "%");
  //field_layer_init(&s_data.page_altitude, &s_data.altitude_accuracy,    w + 1, TOPBAR_HEIGHT + h + 1, w, h, "Accuracy", s_data.accuracy, "m");


  layer_set_hidden(&s_data.page_altitude, true);
}