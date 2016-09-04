#ifndef NAVIGATION_H
#define NAVIGATION_H

void nav_draw_compass(GContext* ctx, GPoint center, GRect box, bool small);
void nav_add_data();
bool nav_is_error_ok();
bool nav_is_bearing_ok();

  #define NAV_TRACK_ERROR_DIST_MIN 50

#endif // NAVIGATION_H
