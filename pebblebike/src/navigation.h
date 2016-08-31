#ifndef NAVIGATION_H
#define NAVIGATION_H

void nav_draw_compass(GContext* ctx, GPoint center, GRect box, bool small);
void nav_add_data();

#define NAV_TRACK_ERROR_DIST_MIN 50

#endif // NAVIGATION_H
