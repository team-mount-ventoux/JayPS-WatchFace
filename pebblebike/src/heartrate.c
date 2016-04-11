#include "pebble.h"
#include "pebblebike.h"
#include "heartrate.h"

char heartrate_zone[40];
#ifdef PBL_COLOR
GColor heartrate_color;
#endif

uint8_t zone = 1;
time_t time_prev = 0;
uint8_t zone_prev = 0;
time_t zone_time_ini = 0;

uint8_t max_heartrate = 185;

uint16_t zone_durations[6] = { 0, 0, 0, 0, 0, 0 };
char zone_textes[6][15] = { "", "Very Light", "Light", "Moderate", "Hard", "Maximum" };

void heartrate_new_data(uint8_t heartrate) {
  if (s_gpsdata.heartrate == 0 || s_gpsdata.heartrate == 255) {
    return;
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
  switch (zone) {
    default:
    case 1:
      heartrate_color = BG_COLOR_SPEED_MAIN;
      break;
    case 2:
      heartrate_color = GColorIslamicGreen;
      break;
    case 3:
      heartrate_color = GColorDarkGreen;
      break;
    case 4:
      heartrate_color = GColorWindsorTan; //GColorOrange;
      break;
    case 5:
      heartrate_color = GColorDarkCandyAppleRed;
      break;
  }
#endif
  char buffer_duration[10];
  if (zone_durations[zone] < 60) {
    snprintf(buffer_duration, sizeof(buffer_duration), "%d\"", zone_durations[zone]);
  } else {
    snprintf(buffer_duration, sizeof(buffer_duration), "%d'%d\"", zone_durations[zone] / 60, zone_durations[zone] % 60);
  }
  snprintf(heartrate_zone, sizeof(heartrate_zone), "%d - %s - %s", zone, zone_textes[zone], buffer_duration);
  LOG_DEBUG("h=%d r=%d z=%d %d/%d/%d/%d/%d %s", s_gpsdata.heartrate, ratio, zone, zone_durations[1], zone_durations[2], zone_durations[3], zone_durations[4], zone_durations[5], heartrate_zone);
}
