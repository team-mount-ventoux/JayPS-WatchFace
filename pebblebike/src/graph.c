#include "pebble.h"
#include "graph.h"

GraphData graph_altitudes;
GraphData graph_speeds;
GraphData graph_heartrates;
GraphData graph_ascentrates;

void graph_init_data(GraphData* graph, uint8_t point_duration) {
  for(int i = 0; i < GRAPH_NB_POINTS; i++) {
    graph->points[i] = GRAPH_UNDEF_POINT;
  }
  graph->last_sum = 0;
  graph->last_nb = 0;
  graph->last_index = 0;
  graph->time_ini = 0;
  graph->point_duration = point_duration;
}

void graph_init() {
  graph_init_data(&graph_altitudes, 120);
  graph_init_data(&graph_ascentrates, 120);
  graph_init_data(&graph_heartrates, 30);
  graph_init_data(&graph_speeds, 30);
}

void graph_add_data(GraphData* graph, uint16_t value) {
  time_t time_cur = time(NULL);
  if (graph->time_ini == 0) {
    graph->time_ini = time_cur;
  }
  int index = (time_cur - graph->time_ini) / graph->point_duration;
  if (index > graph->last_index) {
    if (index >= GRAPH_NB_POINTS) {
      //shift data
      int shift = index - graph->last_index;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "shift from %d to %d: %d", graph->last_index, index, shift);
      if (shift > GRAPH_NB_POINTS) {
        shift = GRAPH_NB_POINTS -1;
      }
      for(int i = 0; i < GRAPH_NB_POINTS - shift; i++) {
        graph->points[i] = graph->points[i + shift];
      }
      for(int i = GRAPH_NB_POINTS - shift; i < GRAPH_NB_POINTS - 1; i++) {
        graph->points[i] = GRAPH_UNDEF_POINT;
      }
    }
    graph->last_sum = 0;
    graph->last_nb = 0;
  }
  graph->last_sum += value;
  graph->last_nb++;
  graph->points[index < GRAPH_NB_POINTS ? index : GRAPH_NB_POINTS -1] = graph->last_sum / graph->last_nb;

  #define p(i) graph->points[i] != GRAPH_UNDEF_POINT ? graph->points[i] : -1
  APP_LOG(APP_LOG_LEVEL_DEBUG, "%d pts[%d]=%d "
          //"(%d: %d/%d) "
          "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d-%d|%d|%d|%d|%d|%d|%d|%d|%d|%d",
          (int) (time_cur - graph->time_ini), index, (int16_t) (graph->last_sum / graph->last_nb),
          //value, (int16_t) graph->last_sum, graph->last_nb,
          p(0), p(1), p(2), p(3), p(4), p(5), p(6), p(7), p(8), p(9),
          p(10), p(11), p(12), p(13), p(14), p(15), p(16), p(17), p(18), p(19)
  );
  graph->last_index = index;
}

void graph_draw(GContext* ctx, GRect bounds, GraphData* graph, GraphRange* colors) {
  //graphics_context_set_fill_color(ctx, GColorRed);
  //graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  int min = 10000;
  int max = -10000;
  for (int i = 0; i < GRAPH_NB_POINTS; i++) {
    if (graph->points[i] != GRAPH_UNDEF_POINT) {
      max = graph->points[i] > max ? graph->points[i] : max;
      min = graph->points[i] < min ? graph->points[i] : min;
    }
  }
  int coeff100 = max != min ? 100 * bounds.size.h / (max-min) : 0;
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "min=%d max=%d coeff100=%d", min, max, coeff100);
  int size = 6;

  for (int i = 0; i < GRAPH_NB_POINTS; i++) {
    if (graph->points[i] != GRAPH_UNDEF_POINT) {
      int height = (graph->points[i] - min) * coeff100 / 100;
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "%d: pts=%d height=%d", i, graph->points[i], height);
      for (int j = 0; j <= (int) (height / size); j++) {
        GColor color = GColorWhite;
        for (int k = 0; k < 3; k++) {
          //if (graph->points[i] >= options.colors[k].min) {
          if ((j * size * 100 / coeff100) + min >= colors[k].min) {
            color = colors[k].color;
          }
        }
        graphics_context_set_fill_color(ctx, color);

        int height2 = j < (int) (height / size) ? size : height % size;
        graphics_fill_rect(ctx, GRect(bounds.origin.x + i * size, bounds.origin.y + bounds.size.h - j * size, size - 1, -(height2-1)), 0, GCornerNone);
      }
    }
  }
}
