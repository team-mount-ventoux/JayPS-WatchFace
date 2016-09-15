#include "config.h"
#include "pebblebike.h"
#include "screen_map.h"

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

void screen_map_init() {
    for (int i = 0; i < MAP_NUM_POINTS; i++) {
        pts[i] = GPoint(0, 0);
    }

    s_gpsdata.xpos=0;
    s_gpsdata.ypos=0;
    s_gpsdata.nb_received=0;

#ifdef ENABLE_DEMO
    srand(0);
    #define MIN(a, b) (a < b ? a : b)
    for(int j = MAP_NUM_POINTS; j>0; j--) {
      pts[MAP_NUM_POINTS-j] = GPoint(2*j, j*j/50 + 5 + rand()%2);
      cur_point++;
    }
    nb_points = cur_point;
#endif
}

void screen_map_init_gpath() {
  bearing_gpath = gpath_create(&BEARING_PATH_POINTS);
}
