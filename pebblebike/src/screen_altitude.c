#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screens.h"
#include "screen_config.h"
#include "screen_altitude.h"

#ifdef PBL_PLATFORM_CHALK
  #define PAGE_ALTITUDE_TOP_H SCREEN_H / 2 - TOPBAR_HEIGHT - 1
#else
  // 74
  #define PAGE_ALTITUDE_TOP_H PAGE_H / 2 - 1
#endif

void page_altitude_update_proc(Layer *page_altitude, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, COLOR_LINES);

  // vertical line
  graphics_draw_line(ctx, GPoint(PAGE_OFFSET_X + PAGE_W / 2, 0), GPoint(PAGE_OFFSET_X + PAGE_W / 2, SCREEN_H));

  // horizontal line
  graphics_draw_line(ctx, GPoint(0, PAGE_ALTITUDE_TOP_H), GPoint(SCREEN_W, PAGE_ALTITUDE_TOP_H));
}

void screen_altitude_layer_init(Window* window) {
  s_data.page_altitude = layer_create(PAGE_GRECT);
  layer_set_update_proc(s_data.page_altitude, page_altitude_update_proc);
  layer_add_child(window_get_root_layer(window), s_data.page_altitude);

  int16_t w = PAGE_W / 2; //61
  int16_t h = PAGE_ALTITUDE_TOP_H;

  field_layer_init(s_data.page_altitude, &s_data.screenB_layer.field_top_left,     PAGE_OFFSET_X,     0,     w, h, "Altitude", s_data.altitude, s_data.screenB_layer.field_top_left.units);
  field_layer_init(s_data.page_altitude, &s_data.screenB_layer.field_top_right,    PAGE_OFFSET_X + w + 1, 0,     w, h, "Ascent", s_data.ascent, s_data.screenB_layer.field_top_right.units);
  field_layer_init(s_data.page_altitude, &s_data.screenB_layer.field_bottom_left,  PAGE_OFFSET_X,     h + 1, w, h, "Ascent rate", s_data.ascentrate, s_data.screenB_layer.field_bottom_left.units);
  field_layer_init(s_data.page_altitude, &s_data.screenB_layer.field_bottom_right, PAGE_OFFSET_X + w + 1, h + 1, w, h, "Slope", s_data.slope, "%");

  s_data.screenB_layer.field_top_left.type = config.screenB_top_left_type;
  s_data.screenB_layer.field_top_right.type = config.screenB_top_right_type;
  s_data.screenB_layer.field_bottom_left.type = config.screenB_bottom_left_type;
  s_data.screenB_layer.field_bottom_right.type = config.screenB_bottom_right_type;

  layer_set_hidden(s_data.page_altitude, true);
}

void screen_altitude_layer_deinit() {
  field_layer_deinit(&s_data.screenB_layer.field_top_left);
  field_layer_deinit(&s_data.screenB_layer.field_top_right);
  field_layer_deinit(&s_data.screenB_layer.field_bottom_left);
  field_layer_deinit(&s_data.screenB_layer.field_bottom_right);
  layer_destroy(s_data.page_altitude);
}

