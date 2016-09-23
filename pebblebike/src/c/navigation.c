#include "pebble.h"
#include "pebblebike.h"
#include "navigation.h"

void nav_draw_compass(GContext* ctx, GPoint center, GRect box, bool small) {
  if (s_gpsdata.nav_nb_pages > 0) {
//  s_gpsdata.nav_bearing = 270;
#ifdef ENABLE_NAVIGATION_FULL
    if (!nav_is_error_ok()) {
      graphics_context_set_stroke_width(ctx, COLOR_FALLBACK(small ? 2 : 3, small ? 1 : 2));
      #ifdef PBL_COLOR
        graphics_context_set_stroke_color(ctx, GColorOrange);
      #endif
    }
#endif
//  s_gpsdata.bearing = 320;
    graphics_draw_circle(ctx, center, box.size.w / 2);
    int direction = (s_gpsdata.nav_bearing - s_gpsdata.bearing + 360) % 360;

#ifdef ENABLE_NAVIGATION_FULL
    // compass
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, center, gpoint_from_polar(box, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(360 - s_gpsdata.bearing)));
#endif

#if defined(ENABLE_NAVIGATION_FULL) && defined(PBL_COLOR)
    if (nav_is_bearing_ok()) {
      graphics_context_set_stroke_color(ctx, GColorGreen);
    } else {
      graphics_context_set_stroke_color(ctx, GColorRed);
    }
    graphics_context_set_stroke_width(ctx, small ? 5 : 7);
    graphics_draw_line(ctx, center, gpoint_from_polar(box, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(direction)));
#else
  #ifdef PBL_SDK_2
    // no support for graphics_fill_radial and GColorLightGray with sdk_2
    GPoint gpoint_from_polar;
    gpoint_from_polar.x = center.x + (sin_lookup(DEG_TO_TRIGANGLE(direction)) * box.size.w / 2 / TRIG_MAX_RATIO);
    gpoint_from_polar.y = center.y - (cos_lookup(DEG_TO_TRIGANGLE(direction)) * box.size.w / 2 / TRIG_MAX_RATIO);
    //graphics_context_set_stroke_width(ctx, small ? 5 : 7);
    #define DIRECTION_PADDING 60
    if (!small && ((direction > 90 - DIRECTION_PADDING && direction < 90 + DIRECTION_PADDING) || (direction > 270 - DIRECTION_PADDING && direction < 270 + DIRECTION_PADDING))) {
      graphics_context_set_stroke_color(ctx, GColorWhite);
    }
    graphics_draw_line(ctx, center, gpoint_from_polar);
  #else
    if (small) {
      #ifdef ENABLE_NAVIGATION_FULL
        graphics_context_set_stroke_width(ctx, 3);
      #endif
      graphics_draw_line(ctx, center, gpoint_from_polar(box, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(direction)));
    } else {
      // GColorLightGray only works for _fill_ functions, cannot use line...
      graphics_context_set_fill_color(ctx, GColorLightGray);
      graphics_fill_radial(ctx, box, GOvalScaleModeFitCircle, box.size.w, DEG_TO_TRIGANGLE(direction-5), DEG_TO_TRIGANGLE(direction+5));
    }
  #endif
#endif
  }
}

void nav_add_data() {
  static bool can_notify = false; // do not notify 1rst error

  if (nav_is_error_ok() && nav_is_bearing_ok()) {
    // everything ok, can do next notification
    can_notify = true;
  } else {
    // !nav_is_error_ok() || !nav_is_bearing_ok()
    if (s_data.nav_notification && can_notify) {
      vibes_short_pulse();
      can_notify = false;
    }
  }
}

bool nav_is_error_ok() {
  // 2 * accuracy: 1 for gps (live) + 1 for gpx (recorded): same place, accuracy could have been similar
  return s_gpsdata.nav_error1000 - 2 * s_gpsdata.accuracy < NAV_TRACK_ERROR_DIST_MIN;
}

bool nav_is_bearing_ok() {
  int direction = (s_gpsdata.nav_bearing - s_gpsdata.bearing + 360) % 360;
  if (direction > 180) {
    direction = 360 - direction;
  }
  return direction < 45;
}
