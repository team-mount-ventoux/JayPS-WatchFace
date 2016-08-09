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

void nav_add_data() {
  static int16_t last_nav_bearing = -1;
  if (last_nav_bearing > 0) {
    int delta = (s_gpsdata.nav_bearing - last_nav_bearing + 360) % 360;
    if (delta > 180) {
      delta = 360 - delta;
    }
    LOG_INFO("nav_add_data last_nav_bearing:%d nav_bearing:%d delta:%d", last_nav_bearing, s_gpsdata.nav_bearing, delta);
    if (delta > 30) {
      vibes_short_pulse();
    }
  }
  last_nav_bearing = s_gpsdata.nav_bearing;
}
