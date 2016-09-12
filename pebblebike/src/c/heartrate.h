#ifndef HEARTRATE_H
#define HEARTRATE_H

#define NB_HR_ZONES 5

typedef enum {
  HR_ZONE_NOTIFICATION_DISABLE = 0,
  HR_ZONE_NOTIFICATION_VIBRATE_AT_EVERY_ZONE_CHANGE,
  HR_ZONE_NOTIFICATION_VIBRATE_ENTERING_MAXIMUM_ZONE,
} HR_ZONE_NOTIFICATION_MODES;

void heartrate_init();
void heartrate_new_data(uint8_t heartrate);
uint8_t heartrate_zones_min_hr(uint8_t zone);

extern uint8_t heartrate_max;
extern HR_ZONE_NOTIFICATION_MODES heartrate_zones_notification_mode;
extern char heartrate_zone[40];
extern uint16_t heartrate_zones_duration[NB_HR_ZONES + 1];
extern char heartrate_zones_name[NB_HR_ZONES + 1][15];
#ifdef PBL_COLOR
extern GColor heartrate_color;
extern GColor heartrate_zones_color[NB_HR_ZONES + 1];
#endif

#endif // HEARTRATE_H
