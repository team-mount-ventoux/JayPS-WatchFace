#include "screen_map_gpath.h"
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

void screen_map_init_gpath() {
  bearing_gpath = gpath_create(&BEARING_PATH_POINTS);
}
