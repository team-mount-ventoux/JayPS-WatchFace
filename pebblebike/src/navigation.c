#include "pebble.h"
#include "pebblebike.h"
#include "navigation.h"

void nav_draw_compass(GContext* ctx, GPoint center, GRect box) {
  if (s_gpsdata.nav_distance_to_destination100 > 0) {
//  s_gpsdata.nav_bearing = 270;
//  s_gpsdata.bearing = 320;
    graphics_draw_circle(ctx, center, box.size.w / 2);
    int direction = (s_gpsdata.nav_bearing - s_gpsdata.bearing + 360) % 360;

#ifdef ENABLE_NAVIGATION_FULL
    // compass
    graphics_draw_line(ctx, center, gpoint_from_polar(box, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(360 - s_gpsdata.bearing)));

    if (direction < 45 || direction > 315) {
      if (s_gpsdata.nav_error1000 >= 20) {
         graphics_context_set_stroke_color(ctx, GColorOrange);
      } else {
        graphics_context_set_stroke_color(ctx, GColorGreen);
      }
    } else {
      graphics_context_set_stroke_color(ctx, GColorRed);
    }
#else
    graphics_context_set_stroke_color(ctx, GColorLightGray);
#endif
    graphics_context_set_stroke_width(ctx, 7);
    graphics_draw_line(ctx, center, gpoint_from_polar(box, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(direction)));
  }
}
