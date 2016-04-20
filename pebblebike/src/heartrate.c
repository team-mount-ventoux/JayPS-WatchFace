#include "pebble.h"
#include "pebblebike.h"
#include "heartrate.h"
#ifdef ENABLE_LOCALIZE
  #include "localize.h"
#endif
char heartrate_zone[40];
#ifdef PBL_COLOR
GColor heartrate_color;
GColor heartrate_zones_color[NB_HR_ZONES + 1];
#endif

uint8_t zone = 1;
time_t time_prev = 0;
uint8_t zone_prev = 0;
time_t zone_time_ini = 0;

uint8_t heartrate_max = 0;
HR_ZONE_NOTIFICATION_MODES heartrate_zones_notification_mode = 0;

uint16_t heartrate_zones_duration[NB_HR_ZONES + 1] = { 0, 0, 0, 0, 0, 0 };
char heartrate_zones_name[NB_HR_ZONES + 1][15] = { "", "", "", "", "", ""};

void heartrate_init() {
#ifdef PBL_COLOR
  heartrate_zones_color[0] = BG_COLOR_DATA_MAIN;
  heartrate_zones_color[1] = BG_COLOR_DATA_MAIN;
  heartrate_zones_color[2] = GColorIslamicGreen;
  heartrate_zones_color[3] = GColorDarkGreen;
  heartrate_zones_color[4] = GColorWindsorTan; //GColorOrange;
  heartrate_zones_color[5] = GColorDarkCandyAppleRed;
#endif
  strcpy(heartrate_zones_name[1], _("Very Light"));
  strcpy(heartrate_zones_name[2], _("Light"));
  strcpy(heartrate_zones_name[3], _("Moderate"));
  strcpy(heartrate_zones_name[4], _("Hard"));
  strcpy(heartrate_zones_name[5], _("Maximum"));
}
void heartrate_new_data(uint8_t heartrate) {
  if (s_gpsdata.heartrate == 0 || s_gpsdata.heartrate == 255) {
    return;
  }
  uint8_t ratio = 100 * s_gpsdata.heartrate / heartrate_max;
  if (ratio < 60) {
    zone = 1;
  } else if (ratio >= 90) {
    zone = 5;
  } else {
    // 50-60=>1, 90-100=>5
    zone = ratio / 10 - 4;
  }

  uint16_t delta_duration = time_prev != 0 ? time(NULL) - time_prev : 0;
  if (zone != zone_prev) {
    LOG_INFO("mode=%d zone=%d", heartrate_zones_notification_mode, zone);
    if (
      (heartrate_zones_notification_mode == HR_ZONE_NOTIFICATION_VIBRATE_AT_EVERY_ZONE_CHANGE && zone_time_ini > 0 && time(NULL) - zone_time_ini > 30)
    ||
      (heartrate_zones_notification_mode == HR_ZONE_NOTIFICATION_VIBRATE_ENTERING_MAXIMUM_ZONE && zone == 5)
    ) {
      vibes_short_pulse();
      LOG_INFO("vibes_short_pulse");
    }
    heartrate_zones_duration[zone_prev] += delta_duration;
    zone_prev = zone;
    zone_time_ini = time(NULL);
  }
  time_prev = time(NULL);
  heartrate_zones_duration[zone] += delta_duration;
#ifdef PBL_COLOR
  heartrate_color = heartrate_zones_color[zone];
#endif
  char buffer_duration[10];
  if (heartrate_zones_duration[zone] < 60) {
    snprintf(buffer_duration, sizeof(buffer_duration), "%02d\"", heartrate_zones_duration[zone]);
  } else {
    snprintf(buffer_duration, sizeof(buffer_duration), "%d'%02d\"", heartrate_zones_duration[zone] / 60, heartrate_zones_duration[zone] % 60);
  }
  snprintf(heartrate_zone, sizeof(heartrate_zone), "%d - %s - %s", zone, heartrate_zones_name[zone], buffer_duration);
  LOG_DEBUG("h=%d r=%d z=%d %d/%d/%d/%d/%d %s", s_gpsdata.heartrate, ratio, zone, heartrate_zones_duration[1], heartrate_zones_duration[2], heartrate_zones_duration[3], heartrate_zones_duration[4], heartrate_zones_duration[5], heartrate_zone);
}
uint8_t heartrate_zones_min_hr(uint8_t zone) {
  // zone1: 50% heartrate_max
  // zone5: 90% heartrate_max
  return (4 + zone) * heartrate_max / 10;
}
