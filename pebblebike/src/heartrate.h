#ifndef HEARTRATE_H
#define HEARTRATE_H

void heartrate_new_data(uint8_t heartrate);
extern char heartrate_zone[40];
#ifdef PBL_COLOR
extern GColor heartrate_color;
#endif

#endif // HEARTRATE_H
