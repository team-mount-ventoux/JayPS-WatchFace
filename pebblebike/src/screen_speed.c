#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screen_speed.h"
#include "screens.h"
#include "screen_config.h"

Layer *line_layer;

#if ROTATION
enum {
  ROTATION_DISABLED,
  ROTATION_MIN,
  ROTATION_SPEED = ROTATION_MIN,
  ROTATION_HEARTRATE,
  ROTATION_ALTITUDE,
  ROTATION_MAX,
};
int rotation = ROTATION_DISABLED;
static AppTimer *rotation_timer;
#endif

#ifdef PBL_ROUND
  #define PAGE_SPEED_TOP_H SCREEN_H / 2 - TOPBAR_HEIGHT + 10
  #define PAGE_SPEED_TOP_OFFSET_Y TOPBAR_HEIGHT
  #define PAGE_SPEED_MAIN_H 76
#else
  #define PAGE_SPEED_TOP_H SCREEN_H / 2 - TOPBAR_HEIGHT + 20
  #define PAGE_SPEED_TOP_OFFSET_Y TOPBAR_HEIGHT
  #define PAGE_SPEED_MAIN_H 76
#endif

#define PAGE_SPEED_MIDDLE_DATA_H PAGE_SCREEN_CENTER_H + PBL_IF_ROUND_ELSE(0, 8)
#define PAGE_SPEED_TOP_DATA_H PAGE_SPEED_MIDDLE_DATA_H - PAGE_SPEED_MAIN_H / 2
#define PAGE_SPEED_BOTTOM_DATA_H PAGE_SPEED_MIDDLE_DATA_H + PAGE_SPEED_MAIN_H / 2

void line_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;
  graphics_context_set_stroke_color(ctx, COLOR_LINES);
  graphics_draw_line(ctx, GPoint(PAGE_OFFSET_X + PAGE_W / 2, PAGE_SPEED_TOP_H + 2), GPoint(PAGE_OFFSET_X + PAGE_W / 2, PAGE_H - 2));

  graphics_context_set_fill_color(ctx, BG_COLOR_SPEED_MAIN);
  graphics_fill_rect(ctx, GRect(0, PAGE_SPEED_TOP_DATA_H, SCREEN_W, PAGE_SPEED_MAIN_H), 0, GCornerNone);

#ifndef PBL_SDK_2
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(0, PAGE_SPEED_TOP_DATA_H + 1), GPoint(SCREEN_W, PAGE_SPEED_TOP_DATA_H + 1));
  graphics_draw_line(ctx, GPoint(0, PAGE_SPEED_BOTTOM_DATA_H - 1), GPoint(SCREEN_W, PAGE_SPEED_BOTTOM_DATA_H - 1));
#endif
}
void screen_speed_layer_init(Window* window) {
  s_data.screenA_layer.field_top.type = config.screenA_top_type;
  s_data.screenA_layer.field_top2.type = FIELD_AVGSPEED;
  s_data.screenA_layer.field_bottom_left.type = config.screenA_bottom_left_type;
  s_data.screenA_layer.field_bottom_right.type = config.screenA_bottom_right_type;

  s_data.page_speed = layer_create(PAGE_GRECT);
  layer_add_child(window_get_root_layer(window), s_data.page_speed);
  Layer *window_layer = window_get_root_layer(window);

  // BEGIN bottom left "distance"

  s_data.screenA_layer.field_bottom_left.unit_layer = text_layer_create(GRect(PAGE_OFFSET_X + 1, PAGE_SPEED_BOTTOM_DATA_H + 20, PAGE_W / 2 - 4, 18));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_left.unit_layer, s_data.unitsDistance, fonts_get_system_font(FONT_KEY_GOTHIC_14), PBL_IF_ROUND_ELSE(GTextAlignmentRight, GTextAlignmentCenter), COLOR_UNITS, BG_COLOR_UNITS, s_data.page_speed);

  s_data.screenA_layer.field_bottom_left.data_layer = text_layer_create(GRect(PAGE_OFFSET_X + 1, PAGE_SPEED_BOTTOM_DATA_H - 5, PAGE_W / 2 - 4, 30));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_left.data_layer, s_data.distance, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), PBL_IF_ROUND_ELSE(GTextAlignmentRight, GTextAlignmentCenter), COLOR_DATA, BG_COLOR_DATA, s_data.page_speed);

  // END bottom left

  // BEGIN bottom right "avg"
  s_data.screenA_layer.field_bottom_right.unit_layer = text_layer_create(GRect(PAGE_OFFSET_X + PAGE_W / 2 + PBL_IF_ROUND_ELSE(4, 0), PAGE_SPEED_BOTTOM_DATA_H + 20, PAGE_W / 2 - 2*PBL_IF_ROUND_ELSE(4, 0), 18));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_right.unit_layer, s_data.unitsSpeed, fonts_get_system_font(FONT_KEY_GOTHIC_14), PBL_IF_ROUND_ELSE(GTextAlignmentLeft, GTextAlignmentCenter), COLOR_UNITS, BG_COLOR_UNITS, s_data.page_speed);

  s_data.screenA_layer.field_bottom_right.data_layer = text_layer_create(GRect(PAGE_OFFSET_X + PAGE_W / 2  + PBL_IF_ROUND_ELSE(4, 0), PAGE_SPEED_BOTTOM_DATA_H - 5, PAGE_W / 2 - 2*PBL_IF_ROUND_ELSE(4, 0), 30));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_right.data_layer, s_data.avgspeed, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), PBL_IF_ROUND_ELSE(GTextAlignmentLeft, GTextAlignmentCenter), COLOR_DATA, BG_COLOR_DATA, s_data.page_speed);


  // END bottom right
  line_layer = layer_create(layer_get_frame(window_layer));
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(s_data.page_speed, line_layer);

  // BEGIN top2 "speed"

  s_data.screenA_layer.field_top2.unit_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(19, 3), PAGE_SPEED_TOP_DATA_H - 17, SCREEN_W - 2*PBL_IF_ROUND_ELSE(19, 3), 18));
  set_layer_attr_full(s_data.screenA_layer.field_top2.unit_layer, s_data.unitsSpeed, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentRight, COLOR_UNITS, BG_COLOR_UNITS, s_data.page_speed);

  s_data.screenA_layer.field_top2.data_layer = text_layer_create(GRect(0, PAGE_SPEED_TOP_DATA_H - PBL_IF_ROUND_ELSE(34, 42), SCREEN_W, 30));
  set_layer_attr_full(s_data.screenA_layer.field_top2.data_layer, s_data.avgspeed, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GTextAlignmentCenter, COLOR_DATA, BG_COLOR_DATA, s_data.page_speed);

  // END top2


  // BEGIN top "speed"

  s_data.screenA_layer.field_top.unit_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(10, 3), PAGE_SPEED_MIDDLE_DATA_H + 19, SCREEN_W - 2*PBL_IF_ROUND_ELSE(10, 3), 22));
  set_layer_attr_full(s_data.screenA_layer.field_top.unit_layer, s_data.unitsSpeedOrHeartRate, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentRight, COLOR_SPEED_UNITS, BG_COLOR_SPEED_UNITS, s_data.page_speed);

  s_data.screenA_layer.field_top.data_layer = text_layer_create(GRect(0, PAGE_SPEED_MIDDLE_DATA_H - 42 - PBL_IF_ROUND_ELSE(0,0), SCREEN_W, 80));
  set_layer_attr_full(s_data.screenA_layer.field_top.data_layer, s_data.speed, font_roboto_bold_62, GTextAlignmentCenter, COLOR_SPEED_DATA, BG_COLOR_SPEED_DATA, s_data.page_speed);

  // END top


  layer_set_hidden(s_data.page_speed, false);
}

void screen_speed_deinit() {
  field_layer_deinit(&s_data.screenA_layer.field_top);
  field_layer_deinit(&s_data.screenA_layer.field_top2);
  field_layer_deinit(&s_data.screenA_layer.field_bottom_left);
  field_layer_deinit(&s_data.screenA_layer.field_bottom_right);
  layer_destroy(line_layer);
  layer_destroy(s_data.page_speed);
}

void screen_speed_show_speed(bool force_units) {
  /*
  TODO(config)
  if (s_data.page_number != PAGE_SPEED && s_data.page_number != PAGE_HEARTRATE) {
    // nothing to do here
    return;
  }*/


  if (0) {
#if ROTATION
  } else if (rotation == ROTATION_HEARTRATE) {
    if (force_units) {
      screen_speed_update_config();
    }
#endif
#if ROTATION  
  } else if (rotation == ROTATION_ALTITUDE) {
    snprintf(s_data.speed, sizeof(s_data.speed), "%d", s_gpsdata.altitude);
    if (force_units) {
      strncpy(s_data.unitsSpeedOrHeartRate, s_data.unitsAltitude, 8);
    }
#endif
  } else {
    copy_speed(s_data.speed, sizeof(s_data.speed), s_gpsdata.speed100);
    if (force_units) {
      screen_speed_update_config();
    }
  }
}
#if ROTATION
static void rotation_timer_callback(void *data) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "rotation_timer");
  rotation_timer = app_timer_register(2000, rotation_timer_callback, NULL);
  rotation++;
  if (rotation == ROTATION_HEARTRATE && s_gpsdata.heartrate == 255) {
    rotation++;
  }
  if (rotation == ROTATION_MAX) {
    rotation = ROTATION_MIN;
  }
   if (rotation == ROTATION_MIN) {
    s_data.screenA_layer.field_top.type = FIELD_SPEED;
    s_data.screenA_layer.field_bottom_left.type = FIELD_DISTANCE;
    s_data.screenA_layer.field_bottom_right.type = FIELD_AVGSPEED;
  } else {
    s_data.screenA_layer.field_top.type = FIELD_ALTITUDE;
    s_data.screenA_layer.field_bottom_left.type = FIELD_ASCENT;
    s_data.screenA_layer.field_bottom_right.type = FIELD_DISTANCE;
  }
  screen_speed_show_speed(true);
  screen_speed_update_config();
  update_screens();
}
void screen_speed_start_rotation() {
  if (rotation_timer == NULL) {
    rotation_timer = app_timer_register(500, rotation_timer_callback, NULL);
  } else {
    app_timer_cancel(rotation_timer);
    rotation_timer = NULL;
    rotation = ROTATION_DISABLED;
  }
}
#endif

