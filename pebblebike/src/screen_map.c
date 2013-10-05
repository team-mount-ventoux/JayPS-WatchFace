#include "pebble_os.h"
#include "config.h"
#include "pebblebike.h"
#include "screen_map.h"

// map layer
Layer path_layer;
Layer bearing_layer;

#if ROCKSHOT
#define NUM_POINTS 120
#endif
#if !ROCKSHOT
#define NUM_POINTS 2200
#endif
GPoint pts[NUM_POINTS];
int cur_point = 0;
int map_scale = MAP_SCALE_MIN * 2;
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

GPath bearing_gpath;


void update_map(bool force_recenter);

void screen_map_zoom_out(int factor) {
  map_scale = map_scale * factor;
  if (map_scale > MAP_SCALE_MAX) {
      map_scale = MAP_SCALE_MIN;
  }
  update_map(true);
}
void screen_map_zoom_in(int factor) {
  map_scale = map_scale / factor;
  if (map_scale < MAP_SCALE_MIN) {
      map_scale = MAP_SCALE_MAX;
  }
  update_map(true);
}




void screen_map_update_location() {
  
  if ((xposprev - s_gpsdata.xpos)*(xposprev - s_gpsdata.xpos) + (yposprev - s_gpsdata.ypos)*(yposprev - s_gpsdata.ypos) < 3*3) {
      // distance with previous position < 3*10 (m)
      /*snprintf(s_data.debug2, sizeof(s_data.debug2),
        "#11 nbpoints:%u\npos : %ld|%ld\nposprev : %ld|%ld\n",
        nb_points,
        s_gpsdata.xpos, s_gpsdata.ypos,
        xposprev, yposprev
      );*/
      return;
  }
  //vibes_short_pulse();
  xposprev = s_gpsdata.xpos;
  yposprev = s_gpsdata.ypos;
    
  cur_point = nb_points % NUM_POINTS;
  nb_points++;

  pts[cur_point] = GPoint(s_gpsdata.xpos, s_gpsdata.ypos);
  
  
  if (s_data.page_number == PAGE_MAP) {
    // refresh displayed map only if current page is PAGE_MAP
    update_map(false);
  }

}

void update_map(bool force_recenter) {
  int x, y;
  int debug = 0, debug2 = 0;
  
  x = (XINI + (s_gpsdata.xpos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
  y = (YINI - (s_gpsdata.ypos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;

  bool need_recenter = false;
  if (x + pathFrame.origin.x < SCREEN_W/4) {
    need_recenter = true;
    debug += 1;
  }
  if (3*SCREEN_W/4 < x + pathFrame.origin.x) {
    need_recenter = true;
    debug += 1;
  }
  if (y + pathFrame.origin.y < SCREEN_H/4) {
    need_recenter = true;
    debug2 += 1;
  }
  if (3*SCREEN_H/4 < y + pathFrame.origin.y) {
    need_recenter = true;
    debug2 += 1;
  }

  if (need_recenter || force_recenter) {
    //vibes_short_pulse();
    pathFrame.origin.x = -x + SCREEN_W/2;
    pathFrame.origin.y = -y + SCREEN_H/2;
    layer_set_frame(&path_layer, pathFrame);  
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
  // Update the layer
  layer_mark_dirty(&s_data.page_map);
}

void path_layer_update_callback(Layer *me, GContext *ctx) {
  (void)me;

  graphics_context_set_stroke_color(ctx, GColorBlack);

  GPoint p0, p1;
  
  if (nb_points < 2) {
      return;
  }

  for (int i = 0; i < ((nb_points > NUM_POINTS ? NUM_POINTS : nb_points) - 1); i++) {
    p0 = pts[(cur_point-i) % NUM_POINTS];
    p1 = pts[(cur_point-i-1) % NUM_POINTS];

    p0.x = (XINI + (p0.x * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
    p0.y = (YINI - (p0.y * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;
    p1.x = (XINI + (p1.x * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
    p1.y = (YINI - (p1.y * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y; 

    graphics_draw_line(
        ctx,
        p0,
        p1
    );
  }
  
  for (int i = 0; i < s_live.nb; i++) {
    p0.x = (XINI + (s_live.friends[i].xpos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
    p0.y = (YINI - (s_live.friends[i].ypos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;

    graphics_draw_pixel(ctx, p0);
    graphics_draw_circle(ctx, p0, 3);
    //vibes_short_pulse();

    s_live.friends[i].name_frame.origin.x = (pathFrame.origin.x)+p0.x + 6;
    s_live.friends[i].name_frame.origin.y = (pathFrame.origin.y)+p0.y - 7;
    layer_set_frame(&s_live.friends[i].name_layer.layer, s_live.friends[i].name_frame);
        
    if (i == 0) {
      #if DEBUG
      snprintf(s_data.debug2, sizeof(s_data.debug2),
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
  
}
void bearing_layer_update_callback(Layer *me, GContext *ctx) {
  int x, y;
  
  x = (XINI + (s_gpsdata.xpos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
  y = (YINI - (s_gpsdata.ypos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;

  gpath_move_to(&bearing_gpath, GPoint(x + pathFrame.origin.x, y + pathFrame.origin.y));  
  
  gpath_rotate_to(&bearing_gpath, (TRIG_MAX_ANGLE / 360) * s_gpsdata.bearing);

  // Fill the path:
  //graphics_context_set_fill_color(ctx, GColorBlack);
  //gpath_draw_filled(ctx, &bearing_gpath);
  
  // Stroke the path:
  graphics_context_set_stroke_color(ctx, GColorBlack);
  gpath_draw_outline(ctx, &bearing_gpath);
}
void screen_map_layer_init(Window* window) {

  for (int i = 0; i < NUM_POINTS; i++) {
    pts[i] = GPoint(0, 0);
  }

  layer_init(&s_data.page_map, GRect(0,TOPBAR_HEIGHT,SCREEN_W,SCREEN_H-TOPBAR_HEIGHT));
  layer_add_child(&window->layer, &s_data.page_map);

  for(int i = 0; i < NUM_LIVE_FRIENDS; i++) {
    s_live.friends[i].name_frame = GRect(0, 15, 100, 15);
    text_layer_init(&s_live.friends[i].name_layer, s_live.friends[i].name_frame);
    text_layer_set_text(&s_live.friends[i].name_layer, s_live.friends[i].name);
    text_layer_set_text_color(&s_live.friends[i].name_layer, GColorBlack);
    text_layer_set_background_color(&s_live.friends[i].name_layer, GColorWhite);
    text_layer_set_font(&s_live.friends[i].name_layer, font_12);
    text_layer_set_text_alignment(&s_live.friends[i].name_layer, GTextAlignmentLeft);
    layer_add_child(&s_data.page_map, &s_live.friends[i].name_layer.layer);
  }

  pathFrame = GRect(0, 0, MAP_VSIZE_X, MAP_VSIZE_Y);
  layer_init(&path_layer, pathFrame);
  pathFrame.origin.x = -XINI + SCREEN_W/2;
  pathFrame.origin.y = -YINI + SCREEN_H/2;
  layer_set_frame(&path_layer, pathFrame);
  path_layer.update_proc = path_layer_update_callback;
  layer_add_child(&s_data.page_map, &path_layer);
  
  layer_init(&bearing_layer, GRect(0, 0, SCREEN_W, SCREEN_H));
  bearing_layer.update_proc = bearing_layer_update_callback;
  layer_add_child(&s_data.page_map, &bearing_layer);

  gpath_init(&bearing_gpath, &BEARING_PATH_POINTS);
  gpath_move_to(&bearing_gpath, GPoint(SCREEN_W/2, SCREEN_H/2));

  layer_set_hidden(&s_data.page_map, true);
  
  
  s_gpsdata.xpos=0;
  s_gpsdata.ypos=0;
  s_gpsdata.nb_received=0;
}

