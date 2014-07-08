#ifndef SCREEN_MAP_H
#define SCREEN_MAP_H

// in meters/pixels
#define MAP_SCALE_MIN 500
#define MAP_SCALE_MAX 64000
#define MAP_SCALE_INI (MAP_SCALE_MIN*4)

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

#endif // SCREEN_MAP_H
