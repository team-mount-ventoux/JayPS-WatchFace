#ifndef HEARTRATE_H
#define HEARTRATE_H

#define NB_HR_ZONES 5

void heartrate_new_data(uint8_t heartrate);
extern char heartrate_zone[40];
extern uint8_t heartrate_zones_min_hr[NB_HR_ZONES + 1];
#ifdef PBL_COLOR
extern GColor heartrate_color;
extern GColor heartrate_zones_color[NB_HR_ZONES + 1];
#endif

#endif // HEARTRATE_H
