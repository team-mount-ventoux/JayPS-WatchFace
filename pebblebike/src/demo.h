#ifndef DEMO_H
#define DEMO_H

#ifdef ENABLE_DEMO

#include "config.h"

#define DEMO_INIT_TRACK_NAV() \
int i = 0; \
s_gpsdata.nav_xpos[i] = 367; s_gpsdata.nav_ypos[i++] = 474; \
s_gpsdata.nav_xpos[i] = 314; s_gpsdata.nav_ypos[i++] = 567; \
s_gpsdata.nav_xpos[i] = 313; s_gpsdata.nav_ypos[i++] = 547; \
s_gpsdata.nav_xpos[i] = 307; s_gpsdata.nav_ypos[i++] = 547; \
s_gpsdata.nav_xpos[i] = 289; s_gpsdata.nav_ypos[i++] = 560; \
s_gpsdata.nav_xpos[i] = 281; s_gpsdata.nav_ypos[i++] = 580; \
s_gpsdata.nav_xpos[i] = 284; s_gpsdata.nav_ypos[i++] = 599; \
s_gpsdata.nav_xpos[i] = 277; s_gpsdata.nav_ypos[i++] = 616; \
s_gpsdata.nav_xpos[i] = 279; s_gpsdata.nav_ypos[i++] = 636; \
s_gpsdata.nav_xpos[i] = 285; s_gpsdata.nav_ypos[i++] = 643; \
s_gpsdata.nav_xpos[i] = 281; s_gpsdata.nav_ypos[i++] = 659; \
s_gpsdata.nav_xpos[i] = 295; s_gpsdata.nav_ypos[i++] = 702; \
s_gpsdata.nav_xpos[i] = 292; s_gpsdata.nav_ypos[i++] = 720; \
s_gpsdata.nav_xpos[i] = 295; s_gpsdata.nav_ypos[i++] = 713; \
s_gpsdata.nav_xpos[i] = 301; s_gpsdata.nav_ypos[i++] = 711; \
s_gpsdata.nav_xpos[i] = 302; s_gpsdata.nav_ypos[i++] = 724; \
s_gpsdata.nav_xpos[i] = 314; s_gpsdata.nav_ypos[i++] = 710; \
s_gpsdata.nav_xpos[i] = 315; s_gpsdata.nav_ypos[i++] = 715; \
s_gpsdata.nav_xpos[i] = 319; s_gpsdata.nav_ypos[i++] = 715; \
s_gpsdata.nav_xpos[i] = 312; s_gpsdata.nav_ypos[i++] = 730; \
s_gpsdata.nav_xpos[i] = 317; s_gpsdata.nav_ypos[i++] = 731; \
s_gpsdata.nav_xpos[i] = 318; s_gpsdata.nav_ypos[i++] = 739; \
s_gpsdata.nav_xpos[i] = 321; s_gpsdata.nav_ypos[i++] = 739; \
s_gpsdata.nav_xpos[i] = 329; s_gpsdata.nav_ypos[i++] = 734; \
s_gpsdata.nav_xpos[i] = 342; s_gpsdata.nav_ypos[i++] = 724; \
s_gpsdata.nav_xpos[i] = 345; s_gpsdata.nav_ypos[i++] = 724; \
s_gpsdata.nav_xpos[i] = 348; s_gpsdata.nav_ypos[i++] = 728; \
s_gpsdata.nav_xpos[i] = 347; s_gpsdata.nav_ypos[i++] = 734; \
s_gpsdata.nav_nb_pages = i/20; \
s_gpsdata.nav_next_index = 12; \
s_gpsdata.nav_first_index_in_storage = 0;


#define DEMO_INIT_TRACK() \
s_gpsdata.xpos = 367; s_gpsdata.ypos = 474; screen_map_update_location(); \
s_gpsdata.xpos = 314; s_gpsdata.ypos = 567; screen_map_update_location(); \
s_gpsdata.xpos = 313; s_gpsdata.ypos = 547; screen_map_update_location(); \
s_gpsdata.xpos = 307; s_gpsdata.ypos = 547; screen_map_update_location(); \
s_gpsdata.xpos = 287; s_gpsdata.ypos = 560; screen_map_update_location(); \
s_gpsdata.xpos = 283; s_gpsdata.ypos = 580; screen_map_update_location(); \
s_gpsdata.xpos = 285; s_gpsdata.ypos = 599; screen_map_update_location(); \
s_gpsdata.xpos = 277; s_gpsdata.ypos = 616; screen_map_update_location(); \
s_gpsdata.xpos = 279; s_gpsdata.ypos = 636; screen_map_update_location(); \
s_gpsdata.xpos = 283; s_gpsdata.ypos = 643; screen_map_update_location(); \
s_gpsdata.xpos = 281; s_gpsdata.ypos = 669; screen_map_update_location(); \
s_gpsdata.xpos = 295; s_gpsdata.ypos = 702; screen_map_update_location(); \
s_gpsdata.bearing = 340; \
map_scale = MAP_SCALE_MIN*4; \
screen_map_update_map(true);


#endif
#endif // DEMO_H
