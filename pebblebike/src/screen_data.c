#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screens.h"
#include "screen_config.h"
#include "graph.h"
#include "heartrate.h"
#include "screen_data.h"

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
  #define PAGE_DATA_TOP_H SCREEN_H / 2 - TOPBAR_HEIGHT + 10
  #define PAGE_DATA_TOP_OFFSET_Y TOPBAR_HEIGHT
  #define PAGE_DATA_MAIN_H 76
#else
  #define PAGE_DATA_TOP_H SCREEN_H / 2 - TOPBAR_HEIGHT + 20
  #define PAGE_DATA_TOP_OFFSET_Y TOPBAR_HEIGHT
  #define PAGE_DATA_MAIN_H 76
#endif

#define PAGE_DATA_MIDDLE_DATA_H PAGE_SCREEN_CENTER_H + PBL_IF_ROUND_ELSE(0, 8)
#define PAGE_DATA_TOP_DATA_H PAGE_DATA_MIDDLE_DATA_H - PAGE_DATA_MAIN_H / 2
#define PAGE_DATA_BOTTOM_DATA_H PAGE_DATA_MIDDLE_DATA_H + PAGE_DATA_MAIN_H / 2

void line_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;
  graphics_context_set_stroke_color(ctx, COLOR_LINES);
  graphics_draw_line(ctx, GPoint(PAGE_OFFSET_X + PAGE_W / 2, PAGE_DATA_TOP_H + 2), GPoint(PAGE_OFFSET_X + PAGE_W / 2, PAGE_H - 2));

  graphics_context_set_fill_color(ctx, bg_color_data_main);
  graphics_fill_rect(ctx, GRect(0, PAGE_DATA_TOP_DATA_H, SCREEN_W, PAGE_DATA_MAIN_H), 0, GCornerNone);

#ifndef PBL_SDK_2
  graphics_context_set_fill_color(ctx, COLOR_LINES_DATA_MAIN);
  graphics_fill_rect(ctx, GRect(0, PAGE_DATA_TOP_DATA_H, SCREEN_W, 2), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(0, PAGE_DATA_BOTTOM_DATA_H, SCREEN_W, 2), 0, GCornerNone);
#endif
  if (s_data.data_subpage == SUBPAGE_UNDEF) {
    return;
  }

#define GRECT_GRAPH GRect(PBL_IF_ROUND_ELSE(19, 1), 1, SCREEN_W - 2*PBL_IF_ROUND_ELSE(19, 1), PBL_IF_ROUND_ELSE(25,34))
#ifdef ENABLE_DEMO
  GraphData heartrates;
  int16_t points[GRAPH_NB_POINTS] = {135,145,150,148,150,155,162,170,180,185,182,175,170,160,155,163,165,155,162,164};
  memcpy(heartrates.points, points, sizeof(int16_t)*GRAPH_NB_POINTS);
  GraphRange colors_heartrates[3] = {
      {.min = 140, .color = PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorBlack)},
      {.min = 150, .color = PBL_IF_COLOR_ELSE(GColorWindsorTan, GColorBlack)},
      {.min = 170, .color = PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorBlack)}
  };
  graph_draw(ctx, GRECT_GRAPH, &heartrates, colors_heartrates, ARRAY_LENGTH(colors_heartrates), s_data.screenData_layer.field_top2.data_layer, 1, false);
#else
  if (
#ifdef PBL_COLOR
      s_data.screen_config[s_data.data_subpage].field_top2.type == FIELD_HEARTRATE_DATA_AND_GRAPH ||
#endif
      s_data.screen_config[s_data.data_subpage].field_top2.type == FIELD_HEARTRATE_GRAPH_ONLY) {
#ifdef PBL_COLOR
    GraphRange colors_heartrates[4] = {
        {.min = heartrate_zones_min_hr(2), .color = heartrate_zones_color[2]},
        {.min = heartrate_zones_min_hr(3), .color = heartrate_zones_color[3]},
        {.min = heartrate_zones_min_hr(4), .color = heartrate_zones_color[4]},
        {.min = heartrate_zones_min_hr(5), .color = heartrate_zones_color[5]},
    };
    graph_draw(ctx, GRECT_GRAPH, &graph_heartrates, colors_heartrates, ARRAY_LENGTH(colors_heartrates), s_data.screen_config[s_data.data_subpage].field_top2.type == FIELD_HEARTRATE_DATA_AND_GRAPH ? s_data.screenData_layer.field_top2.data_layer : NULL, 0, false);
#else
    graph_draw(ctx, GRECT_GRAPH, &graph_heartrates, NULL, 0, NULL, 0, false);
#endif
    //layer_set_hidden(text_layer_get_layer(s_data.screenData_layer.field_top2.unit_layer), true);
  } else if (
#ifdef PBL_COLOR
      s_data.screen_config[s_data.data_subpage].field_top2.type == FIELD_ALTITUDE_DATA_AND_GRAPH ||
#endif
      s_data.screen_config[s_data.data_subpage].field_top2.type == FIELD_ALTITUDE_GRAPH_ONLY) {
#ifdef PBL_COLOR
    GraphRange colors_altitudes[3] = {
        {.min = 100, .color = PBL_IF_COLOR_ELSE(GColorGreen, GColorBlack)},
        {.min = 200, .color = PBL_IF_COLOR_ELSE(GColorOrange, GColorBlack)},
        {.min = 300, .color = PBL_IF_COLOR_ELSE(GColorRed, GColorBlack)}
    };
    graph_draw(ctx, GRECT_GRAPH, &graph_altitudes, colors_altitudes, ARRAY_LENGTH(colors_altitudes), s_data.screen_config[s_data.data_subpage].field_top2.type == FIELD_ALTITUDE_DATA_AND_GRAPH ? s_data.screenData_layer.field_top2.data_layer : NULL, 10, true);
#else
    graph_draw(ctx, GRECT_GRAPH, &graph_altitudes, NULL, 0, NULL, 10, true);
#endif

  } else if (
#ifdef PBL_COLOR
      s_data.screen_config[s_data.data_subpage].field_top2.type == FIELD_ASCENTRATE_DATA_AND_GRAPH ||
#endif
      s_data.screen_config[s_data.data_subpage].field_top2.type == FIELD_ASCENTRATE_GRAPH_ONLY) {
#ifdef PBL_COLOR
    GraphRange colors_ascentrates[3] = {
        {.min = 300, .color = PBL_IF_COLOR_ELSE(GColorGreen, GColorBlack)},
        {.min = 600, .color = PBL_IF_COLOR_ELSE(GColorOrange, GColorBlack)},
        {.min = 900, .color = PBL_IF_COLOR_ELSE(GColorRed, GColorBlack)}
    };
    graph_draw(ctx, GRECT_GRAPH, &graph_ascentrates, colors_ascentrates, ARRAY_LENGTH(colors_ascentrates), s_data.screen_config[s_data.data_subpage].field_top2.type == FIELD_ASCENTRATE_DATA_AND_GRAPH ? s_data.screenData_layer.field_top2.data_layer : NULL, 10, false);
#else
    graph_draw(ctx, GRECT_GRAPH, &graph_ascentrates, NULL, 0, NULL, 10, false);
#endif
  } else if (
#ifdef PBL_COLOR
      s_data.screen_config[s_data.data_subpage].field_top2.type == FIELD_SPEED_DATA_AND_GRAPH ||
#endif
      s_data.screen_config[s_data.data_subpage].field_top2.type == FIELD_SPEED_GRAPH_ONLY) {
#ifdef PBL_COLOR
    GraphRange colors_speeds[3] = {
        {.min = 0, .color = PBL_IF_COLOR_ELSE(GColorYellow, GColorBlack)},
        {.min = 0.9 * s_gpsdata.avgspeed100/10, .color = PBL_IF_COLOR_ELSE(GColorOrange, GColorBlack)},
        {.min = 1.1 * s_gpsdata.avgspeed100/10, .color = PBL_IF_COLOR_ELSE(GColorRed, GColorBlack)}
    };
    graph_draw(ctx, GRECT_GRAPH, &graph_speeds, colors_speeds, ARRAY_LENGTH(colors_speeds), s_data.screen_config[s_data.data_subpage].field_top2.type == FIELD_SPEED_DATA_AND_GRAPH ? s_data.screenData_layer.field_top2.data_layer : NULL, 1, true);
#else
    graph_draw(ctx, GRECT_GRAPH, &graph_speeds, NULL, 0, NULL, 1, true);
#endif
  }
#endif
}
void screen_data_layer_init(Window* window) {
  config_init();
  config_affect_type(&s_data.screen_config[SUBPAGE_A].field_top, config.screenA_top_type);
  config_affect_type(&s_data.screen_config[SUBPAGE_A].field_top2, config.screenA_top2_type);
  config_affect_type(&s_data.screen_config[SUBPAGE_A].field_bottom_left, config.screenA_bottom_left_type);
  config_affect_type(&s_data.screen_config[SUBPAGE_A].field_bottom_right, config.screenA_bottom_right_type);

  s_data.page_data = layer_create(PAGE_GRECT);
  layer_add_child(window_get_root_layer(window), s_data.page_data);
  Layer *window_layer = window_get_root_layer(window);

  // BEGIN bottom left "distance"

  s_data.screenData_layer.field_bottom_left.unit_layer = text_layer_create(GRect(PAGE_OFFSET_X + 1, PAGE_DATA_BOTTOM_DATA_H + 20, PAGE_W / 2 - 4, 18));
  set_layer_attr_full(s_data.screenData_layer.field_bottom_left.unit_layer, s_data.unitsDistance, fonts_get_system_font(FONT_KEY_GOTHIC_14), PBL_IF_ROUND_ELSE(GTextAlignmentRight, GTextAlignmentCenter), COLOR_UNITS, BG_COLOR_UNITS, s_data.page_data);

  s_data.screenData_layer.field_bottom_left.data_layer = text_layer_create(GRect(PAGE_OFFSET_X + 1, PAGE_DATA_BOTTOM_DATA_H - 5, PAGE_W / 2 - 4, 30));
  set_layer_attr_full(s_data.screenData_layer.field_bottom_left.data_layer, s_data.distance, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), PBL_IF_ROUND_ELSE(GTextAlignmentRight, GTextAlignmentCenter), COLOR_DATA, BG_COLOR_DATA, s_data.page_data);

  // END bottom left

  // BEGIN bottom right "avg"
  s_data.screenData_layer.field_bottom_right.unit_layer = text_layer_create(GRect(PAGE_OFFSET_X + PAGE_W / 2 + PBL_IF_ROUND_ELSE(4, 0), PAGE_DATA_BOTTOM_DATA_H + 20, PAGE_W / 2 - 2*PBL_IF_ROUND_ELSE(4, 0), 18));
  set_layer_attr_full(s_data.screenData_layer.field_bottom_right.unit_layer, s_data.unitsSpeed, fonts_get_system_font(FONT_KEY_GOTHIC_14), PBL_IF_ROUND_ELSE(GTextAlignmentLeft, GTextAlignmentCenter), COLOR_UNITS, BG_COLOR_UNITS, s_data.page_data);

  s_data.screenData_layer.field_bottom_right.data_layer = text_layer_create(GRect(PAGE_OFFSET_X + PAGE_W / 2  + PBL_IF_ROUND_ELSE(4, 0), PAGE_DATA_BOTTOM_DATA_H - 5, PAGE_W / 2 - 2*PBL_IF_ROUND_ELSE(4, 0), 30));
  set_layer_attr_full(s_data.screenData_layer.field_bottom_right.data_layer, s_data.avgspeed, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), PBL_IF_ROUND_ELSE(GTextAlignmentLeft, GTextAlignmentCenter), COLOR_DATA, BG_COLOR_DATA, s_data.page_data);


  // END bottom right
  line_layer = layer_create(layer_get_frame(window_layer));
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(s_data.page_data, line_layer);

  // BEGIN top2 "speed"

  s_data.screenData_layer.field_top2.unit_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(19, 3), PAGE_DATA_TOP_DATA_H - 17, SCREEN_W - 2*PBL_IF_ROUND_ELSE(19, 3), 18));
  set_layer_attr_full(s_data.screenData_layer.field_top2.unit_layer, s_data.unitsSpeed, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentRight, COLOR_UNITS, BG_COLOR_UNITS, s_data.page_data);

  s_data.screenData_layer.field_top2.data_layer = text_layer_create(GRect(0, PAGE_DATA_TOP_DATA_H - PBL_IF_ROUND_ELSE(34, 42), SCREEN_W, 30));
  set_layer_attr_full(s_data.screenData_layer.field_top2.data_layer, s_data.avgspeed, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GTextAlignmentCenter, COLOR_DATA, BG_COLOR_DATA, s_data.page_data);

  // END top2


  // BEGIN top "speed"

  s_data.screenData_layer.field_top.unit_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(10, 3), PAGE_DATA_MIDDLE_DATA_H + 19, SCREEN_W - 2*PBL_IF_ROUND_ELSE(10, 3), 22));
  set_layer_attr_full(s_data.screenData_layer.field_top.unit_layer, s_data.unitsSpeedOrHeartRate, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentRight, COLOR_DATA_UNITS, BG_COLOR_DATA_UNITS, s_data.page_data);

  s_data.screenData_layer.field_top.data_layer = text_layer_create(GRect(-20, PAGE_DATA_MIDDLE_DATA_H - 42 - PBL_IF_ROUND_ELSE(0,0), SCREEN_W + 40, 80));
  set_layer_attr_full(s_data.screenData_layer.field_top.data_layer, s_data.speed, font_roboto_bold_62, GTextAlignmentCenter, COLOR_DATA_DATA, BG_COLOR_DATA_DATA, s_data.page_data);

  // END top


  layer_set_hidden(s_data.page_data, false);
}

void screen_data_deinit() {
  field_layer_deinit(&s_data.screenData_layer.field_top);
  field_layer_deinit(&s_data.screenData_layer.field_top2);
  field_layer_deinit(&s_data.screenData_layer.field_bottom_left);
  field_layer_deinit(&s_data.screenData_layer.field_bottom_right);
  layer_destroy(line_layer);
  layer_destroy(s_data.page_data);
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
     s_data.screen_config[SUBPAGE_A].field_top.type = FIELD_SPEED;
     s_data.screen_config[SUBPAGE_A].field_bottom_left.type = FIELD_DISTANCE;
     s_data.screen_config[SUBPAGE_A].field_bottom_right.type = FIELD_AVGSPEED;
  } else {
    s_data.screen_config[SUBPAGE_A].field_top.type = FIELD_ALTITUDE;
    s_data.screen_config[SUBPAGE_A].field_bottom_left.type = FIELD_ASCENT;
    s_data.screen_config[SUBPAGE_A].field_bottom_right.type = FIELD_DISTANCE;
  }
  screen_data_update_config(true);
  update_screens();
}
void screen_data_start_rotation() {
  if (rotation_timer == NULL) {
    rotation_timer = app_timer_register(500, rotation_timer_callback, NULL);
  } else {
    app_timer_cancel(rotation_timer);
    rotation_timer = NULL;
    rotation = ROTATION_DISABLED;
  }
}
#endif

