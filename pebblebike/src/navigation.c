#include "pebble.h"
#include "pebblebike.h"
#include "navigation.h"

void nav_draw_compass(GContext* ctx, GPoint center, GRect box, bool small) {
  if (s_gpsdata.nav_distance_to_destination100 > 0) {
//  s_gpsdata.nav_bearing = 270;
#ifdef ENABLE_NAVIGATION_FULL
    if (s_gpsdata.nav_error1000 - 2 * s_gpsdata.accuracy >= NAV_TRACK_ERROR_DIST_MIN) {
      graphics_context_set_stroke_width(ctx, small ? 2 : 3);
      graphics_context_set_stroke_color(ctx, GColorOrange);
    }
#endif
//  s_gpsdata.bearing = 320;
    graphics_draw_circle(ctx, center, box.size.w / 2);
    int direction = (s_gpsdata.nav_bearing - s_gpsdata.bearing + 360) % 360;

#ifdef ENABLE_NAVIGATION_FULL
    // compass
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, center, gpoint_from_polar(box, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(360 - s_gpsdata.bearing)));

    if (direction < 45 || direction > 315) {
      graphics_context_set_stroke_color(ctx, GColorGreen);
    } else {
      graphics_context_set_stroke_color(ctx, GColorRed);
    }
    graphics_context_set_stroke_width(ctx, small ? 5 : 7);
    graphics_draw_line(ctx, center, gpoint_from_polar(box, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(direction)));
#else
    if (small) {
      graphics_draw_line(ctx, center, gpoint_from_polar(box, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(direction)));
    } else {
      // GColorLightGray only works for _fill_ functions, cannot use line...
      graphics_context_set_fill_color(ctx, GColorLightGray);
      graphics_fill_radial(ctx, box, GOvalScaleModeFitCircle, box.size.w, DEG_TO_TRIGANGLE(direction-5), DEG_TO_TRIGANGLE(direction+5));
    }
#endif
  }
}

void nav_add_data() {
  static int16_t last_nav_bearing = -1;
  if (last_nav_bearing >= 0) {
    int delta = (s_gpsdata.nav_bearing - last_nav_bearing + 360) % 360;
    if (delta > 180) {
      delta = 360 - delta;
    }
    LOG_INFO("nav_add_data last_nav_bearing:%d nav_bearing:%d delta:%d notif:%d", last_nav_bearing, s_gpsdata.nav_bearing, delta, s_data.nav_notification);
    if (delta > 30 && s_data.nav_notification) {
      vibes_short_pulse();
    }
  }
  last_nav_bearing = s_gpsdata.nav_bearing;
}
