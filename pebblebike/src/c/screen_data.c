#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screens.h"
#include "screen_config.h"
#include "graph.h"
#include "heartrate.h"
#include "navigation.h"
#include "screen_data.h"
#include "overlord.h"
#include "overlays.h"
#include "ovl/screen_config.h"

Layer *line_layer;

void line_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;
  if (need_launch_config) {
    ///@todo(overlay) HACK (cannot call an overlay inside another one) improve me!
    need_launch_config = false;
    overlay_load(OVL_CONFIG_OVL);
    config_start();
  }
  graphics_context_set_stroke_color(ctx, COLOR_LINES);
  graphics_draw_line(ctx, GPoint(PAGE_OFFSET_X + PAGE_W / 2, PAGE_DATA_TOP_H + 2), GPoint(PAGE_OFFSET_X + PAGE_W / 2, PAGE_H - 2));

  graphics_context_set_fill_color(ctx, bg_color_data_main);
  graphics_fill_rect(ctx, GRect(0, PAGE_DATA_TOP_DATA_H, SCREEN_W, PAGE_DATA_MAIN_H), 0, GCornerNone);

#ifndef PBL_SDK_2
  graphics_context_set_fill_color(ctx, COLOR_LINES_DATA_MAIN);
  graphics_fill_rect(ctx, GRect(0, PAGE_DATA_TOP_DATA_H, SCREEN_W, 2), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(0, PAGE_DATA_BOTTOM_DATA_H, SCREEN_W, 2), 0, GCornerNone);
#endif

#ifdef ENABLE_NAVIGATION
  nav_draw_compass(ctx, NAVIGATION_COMPASS_CENTER, NAVIGATION_COMPASS_RECT, false);
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


void screen_data_deinit() {
  field_layer_deinit(&s_data.screenData_layer.field_top);
  field_layer_deinit(&s_data.screenData_layer.field_top2);
  field_layer_deinit(&s_data.screenData_layer.field_bottom_left);
  field_layer_deinit(&s_data.screenData_layer.field_bottom_right);
  layer_destroy(line_layer);
  layer_destroy(s_data.page_data);
}

