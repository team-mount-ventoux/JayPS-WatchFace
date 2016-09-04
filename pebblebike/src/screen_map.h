#ifndef SCREEN_MAP_H
#define SCREEN_MAP_H

// at level 7 (MAP_SCALE=16000), the screen width is approximatively 14.5km => 100m/px)
// level 1: MAP_SCALE=250 => 1.5m/px - screen 225m
// level 8: MAP_SCALE_MAX 32000 => 200m/px - screen: 29km
#define MAP_SCALE_MIN 250
#define MAP_SCALE_MAX 32000
//2000=MAP_SCALE_MIN * 8 => level 4 (250-500-1000-2000)
#define MAP_SCALE_INI 2000

#define MAP_VSIZE_X 4000
#define MAP_VSIZE_Y 4000

#define XINI MAP_VSIZE_X/2
#define YINI MAP_VSIZE_Y/2

// external variables (debug purpose)
extern int map_scale;
extern int nb_points;

void screen_map_zoom_in(int factor);
void screen_map_zoom_out(int factor);
void screen_map_update_location();
void screen_map_layer_init(Window* window);
void screen_map_layer_deinit();
void screen_map_update_map(bool force_recenter);

#endif // SCREEN_MAP_H
