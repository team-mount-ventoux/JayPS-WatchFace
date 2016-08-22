#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "navigation.h"
#include "screen_map.h"
#include "screens.h"

// map layer
Layer *path_layer;
Layer *bearing_layer;

// 4 Bytes/point
// to compute correct values, use DEMO mode, cycle through all screens including menu
#if PBL_PLATFORM_APLITE
  #define NUM_POINTS 100
#else
  #define NUM_POINTS 1500
#endif

GPoint pts[NUM_POINTS];
int cur_point = 0;
int map_scale = MAP_SCALE_INI;
int nb_points = 0;

int32_t xposprev = 0, yposprev = 0;

GRect pathFrame;


const GPathInfo BEARING_PATH_POINTS = {
    4,
    (GPoint []) {
        {0, 3},
        {-4, 6},
        {0, -6},
        {4, 6},
    }
};

GPath *bearing_gpath;

#define NAVIGATION_COMPASS_RADIUS 15
#define NAVIGATION_COMPASS_PADDING 3
#define NAVIGATION_COMPASS_CENTER_X (NAVIGATION_COMPASS_RADIUS + NAVIGATION_COMPASS_PADDING - pathFrame.origin.x)
#define NAVIGATION_COMPASS_CENTER_Y (NAVIGATION_COMPASS_RADIUS + NAVIGATION_COMPASS_PADDING - pathFrame.origin.y)
#define NAVIGATION_COMPASS_CENTER GPoint(NAVIGATION_COMPASS_CENTER_X, NAVIGATION_COMPASS_CENTER_Y)
#define NAVIGATION_COMPASS_RECT GRect(NAVIGATION_COMPASS_PADDING - pathFrame.origin.x, NAVIGATION_COMPASS_PADDING - pathFrame.origin.y, 2 * NAVIGATION_COMPASS_RADIUS, 2 * NAVIGATION_COMPASS_RADIUS)
#define NAVIGATION_DISTANCE_RECT_W 50
#define NAVIGATION_DISTANCE_RECT_H 18
#define NAVIGATION_DISTANCE_RECT GRect(SCREEN_W - pathFrame.origin.x - NAVIGATION_DISTANCE_RECT_W, - pathFrame.origin.y, NAVIGATION_DISTANCE_RECT_W, NAVIGATION_DISTANCE_RECT_H)

#ifdef ENABLE_MAP_SKIP_POINT_OUTSIDE
  #define MAP_DRAW_LINE(p0, p1) \
  if (p0.x + pathFrame.origin.x < 0 && p1.x + pathFrame.origin.x < 0) { \
    /* LOG_DEBUG("x<0 o:%d", pathFrame.origin.x); */ \
    continue; \
  } \
  if (p0.x + pathFrame.origin.x > SCREEN_W && p1.x + pathFrame.origin.x > SCREEN_W) { \
    /* LOG_DEBUG("x>W o:%d", pathFrame.origin.x); */ \
    continue; \
  } \
  if (p0.y + pathFrame.origin.y < 0 && p1.y + pathFrame.origin.y < 0) { \
    /* LOG_DEBUG("y<0 o:%d", pathFrame.origin.y); */ \
    continue; \
  } \
  if (p0.y + pathFrame.origin.y > SCREEN_H && p1.y + pathFrame.origin.y > SCREEN_H) { \
    /* LOG_DEBUG("y>H o:%d", pathFrame.origin.y); */ \
    continue; \
  } \
  graphics_draw_line(ctx, p0, p1);
#else
  #define MAP_DRAW_LINE(p0, p1) graphics_draw_line(ctx, p0, p1);
#endif



void screen_map_zoom_out(int factor) {
    map_scale = map_scale * factor;
    if (map_scale > MAP_SCALE_MAX) {
        map_scale = MAP_SCALE_MIN;
    }
    screen_map_update_map(true);
}
void screen_map_zoom_in(int factor) {
    map_scale = map_scale / factor;
    if (map_scale < MAP_SCALE_MIN) {
        map_scale = MAP_SCALE_MAX;
    }
    screen_map_update_map(true);
}


// in 10m
#define SCREEN_MAP_MIN_DIST 5
void screen_map_update_location() {

    if ((xposprev - s_gpsdata.xpos)*(xposprev - s_gpsdata.xpos) + (yposprev - s_gpsdata.ypos)*(yposprev - s_gpsdata.ypos) < SCREEN_MAP_MIN_DIST*SCREEN_MAP_MIN_DIST) {
        // distance with previous position < SCREEN_MAP_MIN_DIST*10 (m)
        /*snprintf(s_data.debug2, sizeof(s_data.debug2),
          "#11 nbpoints:%u\npos : %ld|%ld\nposprev : %ld|%ld\n",
          nb_points,
          s_gpsdata.xpos, s_gpsdata.ypos,
          xposprev, yposprev
        );*/
    } else {
      // add new point
      xposprev = s_gpsdata.xpos;
      yposprev = s_gpsdata.ypos;

      cur_point = nb_points % NUM_POINTS;
      nb_points++;
    }

    // update cur point or add new one
    pts[cur_point] = GPoint(s_gpsdata.xpos, s_gpsdata.ypos);

    if (s_data.page_number == PAGE_MAP) {
        // refresh displayed map only if current page is PAGE_MAP
      screen_map_update_map(false);
    }
}

void screen_map_update_map(bool force_recenter) {
    int x, y;
    //int debug = 0, debug2 = 0;

    x = (XINI + (s_gpsdata.xpos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
    y = (YINI - (s_gpsdata.ypos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;

    bool need_recenter = false;
    if (x + pathFrame.origin.x < SCREEN_W/4) {
        need_recenter = true;
        //debug += 1;
    }
    if (3*SCREEN_W/4 < x + pathFrame.origin.x) {
        need_recenter = true;
        //debug += 1;
    }
    if (y + pathFrame.origin.y < SCREEN_H/4) {
        need_recenter = true;
        //debug2 += 1;
    }
    if (3*SCREEN_H/4 < y + pathFrame.origin.y) {
        need_recenter = true;
        //debug2 += 1;
    }

    if (need_recenter || force_recenter) {
        //vibes_short_pulse();
        pathFrame.origin.x = -x + SCREEN_W/2;
        pathFrame.origin.y = -y + SCREEN_H/2;
        layer_set_frame(path_layer, pathFrame);
    }
    /*
      #if DEBUG
      snprintf(s_data.debug2, sizeof(s_data.debug2),
        "#12 nbpts:%u\npos : %d|%d\nx|y:%d|%d\ndebug:%u|%u\nscale:%d\nvsize:%d|%d",
        nb_points,
        s_gpsdata.xpos, s_gpsdata.ypos,
        x, y,
        debug, debug2,
        map_scale,
        MAP_VSIZE_X, MAP_VSIZE_Y
      );
      #endif
    */
#ifdef ENABLE_FUNCTION_LIVE
    // update name_layers before marking page_map dirty to have them correctly positioned on the map
    GPoint p0;
    for (int i = 0; i < s_live.nb; i++) {
        p0.x = (XINI + (s_live.friends[i].xpos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
        p0.y = (YINI - (s_live.friends[i].ypos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;
        s_live.friends[i].name_frame.origin.x = (pathFrame.origin.x)+p0.x + 6;
        s_live.friends[i].name_frame.origin.y = (pathFrame.origin.y)+p0.y - 7;
        layer_set_frame(text_layer_get_layer(s_live.friends[i].name_layer), s_live.friends[i].name_frame);
    }
#endif
    // Update the layer
    layer_mark_dirty(s_data.page_map);
}

void path_layer_update_callback(Layer *me, GContext *ctx) {
    (void)me;
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "path_layer_update_callback");

    GPoint p0, p1;

#ifdef ENABLE_NAVIGATION
  if (s_gpsdata.nav_distance_to_destination100 > 0) {

    graphics_context_set_stroke_width(ctx, 2);

    for (uint16_t i = 1; i < NAV_NB_POINTS_STORAGE - 1; i++) {
      if (s_gpsdata.nav_xpos[i] == INT16_MAX) {
        //LOG_DEBUG("%d: skip point", i);
        continue;
      }
      if (s_gpsdata.nav_xpos[i-1] == INT16_MAX) {
        //LOG_DEBUG("%d: skip point-1", i);
        continue;
      }
      //LOG_DEBUG("%d: xpos:%d ypos:%d %s", i, s_gpsdata.nav_xpos[i], s_gpsdata.nav_ypos[i], s_gpsdata.nav_first_index_in_storage + i == s_gpsdata.nav_next_index ? "NEXT" : "");
#ifdef ENABLE_NAVIGATION_FULL
      if (s_gpsdata.nav_first_index_in_storage + i < s_gpsdata.nav_next_index) {
        // previous points
        graphics_context_set_stroke_color(ctx, GColorLightGray);
      } else if (s_gpsdata.nav_first_index_in_storage + i == s_gpsdata.nav_next_index) {
        // cur point
        int direction = (s_gpsdata.nav_bearing - s_gpsdata.bearing + 360) % 360;
        if (direction > 180) {
          direction = 360 - direction;
        }
        if (s_gpsdata.nav_error1000 >= 20) {
          graphics_context_set_stroke_color(ctx, GColorOrange);
        } else if (direction < 45) {
          graphics_context_set_stroke_color(ctx, GColorGreen);
        } else {
          graphics_context_set_stroke_color(ctx, GColorRed);
        }
      } else {
        // future points
        graphics_context_set_stroke_color(ctx, GColorBlue);
      }
#endif
      p0.x = (XINI + (s_gpsdata.nav_xpos[i-1] * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
      p0.y = (YINI - (s_gpsdata.nav_ypos[i-1] * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;
      p1.x = (XINI + (s_gpsdata.nav_xpos[i] * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
      p1.y = (YINI - (s_gpsdata.nav_ypos[i] * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;

      MAP_DRAW_LINE(p0, p1);
    }
  }
#endif
    graphics_context_set_stroke_width(ctx, 1);
    graphics_context_set_stroke_color(ctx, COLOR_MAP);

    if (nb_points >= 2) {
      for (int i = 0; i < ((nb_points > NUM_POINTS ? NUM_POINTS : nb_points) - 1); i++) {
          p0 = pts[(NUM_POINTS+cur_point-i) % NUM_POINTS];
          p1 = pts[(NUM_POINTS+cur_point-i-1) % NUM_POINTS];

          p0.x = (XINI + (p0.x * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
          p0.y = (YINI - (p0.y * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;
          p1.x = (XINI + (p1.x * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
          p1.y = (YINI - (p1.y * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;

          MAP_DRAW_LINE(p0, p1);
      }
    }

#ifdef ENABLE_FUNCTION_LIVE
    for (int i = 0; i < s_live.nb; i++) {
        p0.x = (XINI + (s_live.friends[i].xpos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
        p0.y = (YINI - (s_live.friends[i].ypos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;

        graphics_draw_pixel(ctx, p0);
        graphics_draw_circle(ctx, p0, 3);

        if (i == 0) {
#ifdef ENABLE_DEBUG
          APP_LOG(APP_LOG_LEVEL_DEBUG,
                     "%d|%d\n"
                     "%d|%d\n"
                     "%d|%d\n"
                     "%d|%d\n",
                     s_live.friends[i].xpos,s_live.friends[i].ypos,
                     p0.x,p0.y,
                     pathFrame.origin.x,pathFrame.origin.y,
                     s_live.friends[i].name_frame.origin.x,s_live.friends[i].name_frame.origin.y
                    );
#endif
        }
    }
#endif


#ifdef ENABLE_NAVIGATION
  if (s_gpsdata.nav_distance_to_destination100 > 0) {
    graphics_context_set_stroke_width(ctx, 1);
    graphics_context_set_stroke_color(ctx, GColorBlack);

#ifdef ENABLE_NAVIGATION_FULL
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, NAVIGATION_DISTANCE_RECT, 0, GCornerNone);
    //graphics_draw_rect(ctx, NAVIGATION_DISTANCE_RECT);
#endif

    ///@todo(nav)
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, s_data.cadence, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), NAVIGATION_DISTANCE_RECT, GTextOverflowModeFill, GTextAlignmentRight, NULL);

    nav_draw_compass(ctx, NAVIGATION_COMPASS_CENTER, NAVIGATION_COMPASS_RECT, true);
  }
#endif
}
void bearing_layer_update_callback(Layer *me, GContext *ctx) {
  int x, y;

    x = (XINI + (s_gpsdata.xpos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
    y = (YINI - (s_gpsdata.ypos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;

    gpath_move_to(bearing_gpath, GPoint(x + pathFrame.origin.x, y + pathFrame.origin.y));

    gpath_rotate_to(bearing_gpath, (TRIG_MAX_ANGLE / 360) * s_gpsdata.bearing);

    // Fill the path:
    //graphics_context_set_fill_color(ctx, GColorBlack);
    //gpath_draw_filled(ctx, &bearing_gpath);

    // Stroke the path:
    graphics_context_set_stroke_color(ctx, COLOR_MAP);
    gpath_draw_outline(ctx, bearing_gpath);
}
void screen_map_layer_init(Window* window) {

    for (int i = 0; i < NUM_POINTS; i++) {
        pts[i] = GPoint(0, 0);
    }

    s_data.page_map = layer_create(GRect(0,TOPBAR_HEIGHT,SCREEN_W,SCREEN_H-TOPBAR_HEIGHT));
    layer_add_child(window_get_root_layer(window), s_data.page_map);
#ifdef ENABLE_FUNCTION_LIVE
    for(int i = 0; i < NUM_LIVE_FRIENDS; i++) {
        s_live.friends[i].name_frame = GRect(0, 15, 100, 15);
        s_live.friends[i].name_layer = text_layer_create(s_live.friends[i].name_frame);
        set_layer_attr_full(s_live.friends[i].name_layer, s_live.friends[i].name, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentLeft, COLOR_MAP, GColorClear, s_data.page_map);
    }
#endif
    pathFrame = GRect(0, 0, MAP_VSIZE_X, MAP_VSIZE_Y);
    path_layer = layer_create(pathFrame);
    pathFrame.origin.x = -XINI + SCREEN_W/2;
    pathFrame.origin.y = -YINI + SCREEN_H/2;
    layer_set_frame(path_layer, pathFrame);
    layer_set_update_proc(path_layer, path_layer_update_callback);
    layer_add_child(s_data.page_map, path_layer);

    bearing_layer = layer_create(GRect(0, 0, SCREEN_W, SCREEN_H));
    layer_set_update_proc(bearing_layer, bearing_layer_update_callback);
    layer_add_child(s_data.page_map, bearing_layer);

    bearing_gpath = gpath_create(&BEARING_PATH_POINTS);
    gpath_move_to(bearing_gpath, GPoint(SCREEN_W/2, SCREEN_H/2));

    layer_set_hidden(s_data.page_map, true);


    s_gpsdata.xpos=0;
    s_gpsdata.ypos=0;
    s_gpsdata.nb_received=0;

#ifdef ENABLE_DEMO
    srand(0);
    #define MIN(a, b) (a < b ? a : b)
    for(int j = NUM_POINTS; j>0; j--) {
      pts[NUM_POINTS-j] = GPoint(2*j, j*j/50 + 5 + rand()%2);
      cur_point++;
    }
    nb_points = cur_point;
#endif
}
void screen_map_layer_deinit() {
#ifdef ENABLE_FUNCTION_LIVE
  for(int i = 0; i < NUM_LIVE_FRIENDS; i++) {
    text_layer_destroy(s_live.friends[i].name_layer);
  }
#endif
  layer_destroy(path_layer);
  layer_destroy(bearing_layer);
  gpath_destroy(bearing_gpath);
  layer_destroy(s_data.page_map);
}

