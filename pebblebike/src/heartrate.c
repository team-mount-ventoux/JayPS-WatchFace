#include "pebble.h"
#include "pebblebike.h"
#include "heartrate.h"

char heartrate_zone[40];
#ifdef PBL_COLOR
GColor heartrate_color;
GColor heartrate_zones_color[NB_HR_ZONES + 1];
#endif

uint8_t zone = 1;
time_t time_prev = 0;
uint8_t zone_prev = 0;
time_t zone_time_ini = 0;

uint8_t max_heartrate = 185;

uint8_t heartrate_zones_min_hr[NB_HR_ZONES + 1] = { 0, 0, 0, 0, 0, 0 };
uint16_t zone_durations[NB_HR_ZONES + 1] = { 0, 0, 0, 0, 0, 0 };
char zone_textes[NB_HR_ZONES + 1][15] = { "", "Very Light", "Light", "Moderate", "Hard", "Maximum" };

void heartrate_new_data(uint8_t heartrate) {
  if (s_gpsdata.heartrate == 0 || s_gpsdata.heartrate == 255) {
    return;
  }
  if (heartrate_zones_min_hr[1] == 0) {
    // not initialized
    heartrate_zones_min_hr[1] = 50 * max_heartrate / 100;
    heartrate_zones_min_hr[2] = 60 * max_heartrate / 100;
    heartrate_zones_min_hr[3] = 70 * max_heartrate / 100;
    heartrate_zones_min_hr[4] = 80 * max_heartrate / 100;
    heartrate_zones_min_hr[5] = 90 * max_heartrate / 100;
#ifdef PBL_COLOR
    heartrate_zones_color[0] = BG_COLOR_SPEED_MAIN;
    heartrate_zones_color[1] = BG_COLOR_SPEED_MAIN;
    heartrate_zones_color[2] = GColorIslamicGreen;
    heartrate_zones_color[3] = GColorDarkGreen;
    heartrate_zones_color[4] = GColorWindsorTan; //GColorOrange;
    heartrate_zones_color[5] = GColorDarkCandyAppleRed;
#endif
  }
  uint8_t ratio = 100 * s_gpsdata.heartrate / max_heartrate;
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
    if (zone_time_ini > 0 && time(NULL) - zone_time_ini > 30) {
      vibes_short_pulse();
      LOG_INFO("vibes_short_pulse");
    }
    zone_durations[zone_prev] += delta_duration;
    zone_prev = zone;
    zone_time_ini = time(NULL);
  }
  time_prev = time(NULL);
  zone_durations[zone] += delta_duration;
#ifdef PBL_COLOR
  heartrate_color = heartrate_zones_color[zone];
#endif
  char buffer_duration[10];
  if (zone_durations[zone] < 60) {
    snprintf(buffer_duration, sizeof(buffer_duration), "%02d\"", zone_durations[zone]);
  } else {
    snprintf(buffer_duration, sizeof(buffer_duration), "%d'%02d\"", zone_durations[zone] / 60, zone_durations[zone] % 60);
  }
  snprintf(heartrate_zone, sizeof(heartrate_zone), "%d - %s - %s", zone, zone_textes[zone], buffer_duration);
  LOG_DEBUG("h=%d r=%d z=%d %d/%d/%d/%d/%d %s", s_gpsdata.heartrate, ratio, zone, zone_durations[1], zone_durations[2], zone_durations[3], zone_durations[4], zone_durations[5], heartrate_zone);
}
