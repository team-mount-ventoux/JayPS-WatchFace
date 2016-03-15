#ifndef GRAPH_H
#define GRAPH_H

#include "time.h"

typedef struct GraphRange {
  int min;
  GColor color;
} GraphRange;

#define GRAPH_NB_POINTS 20
#define GRAPH_UNDEF_POINT INT16_MIN

typedef struct GraphData {
  int16_t points[GRAPH_NB_POINTS];
  time_t time_ini;
  uint16_t last_index;
  int32_t last_sum;
  uint8_t last_nb;
  uint8_t point_duration;
} GraphData;

extern GraphData graph_altitudes;
extern GraphData graph_speeds;
extern GraphData graph_heartrates;
extern GraphData graph_ascentrates;

void graph_init();
void graph_add_data(GraphData* data, uint16_t value);
void graph_draw(GContext* ctx, GRect bounds, GraphData* graph, GraphRange* colors);

#endif // GRAPH_H
