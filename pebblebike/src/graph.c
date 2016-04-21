#include "pebble.h"
#include "config.h"
#include "colors.h"
#include "graph.h"

GraphData graph_altitudes;
GraphData graph_speeds;
GraphData graph_heartrates;
GraphData graph_ascentrates;
GBitmap* mask = NULL;

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

#ifdef PBL_COLOR
  mask = gbitmap_create_with_resource(RESOURCE_ID_MASK);
#endif
}
void graph_deinit() {
#ifdef PBL_COLOR
  gbitmap_destroy(mask);
#endif
}

void graph_add_data(GraphData* graph, int16_t value) {
  time_t time_cur = time(NULL);
  if (graph->time_ini == 0) {
    graph->time_ini = time_cur;
    // clock align graph
#ifdef PBL_HEALTH
    ///@todo test SDK>=3.9
    // time_start_of_today was introduced with 3.9 (at same time than Pebble Health)
    // condition on PBL_HEALTH instead of SDK>=3.9 will be only different on APLITE => doesn't matter
    int delta = (graph->time_ini - time_start_of_today()) % graph->point_duration;
    graph->time_ini = graph->time_ini + delta - graph->point_duration;
#endif
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "delta:%d (duration:%d)", delta, graph->point_duration);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "time_ini:%d -> %d", (int) time_cur, (int) graph->time_ini);
  }
  int index = (time_cur - graph->time_ini) / graph->point_duration;
  if (index > graph->last_index) {
    if (index >= GRAPH_NB_POINTS) {
      //shift data
      int shift = index - graph->last_index;
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "shift from %d to %d: %d", graph->last_index, index, shift);
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
  /*APP_LOG(APP_LOG_LEVEL_DEBUG, "%d pts[%d]=%d "
          //"(%d: %d/%d) "
          "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d-%d|%d|%d|%d|%d|%d|%d|%d|%d|%d",
          (int) (time_cur - graph->time_ini), index, (int16_t) (graph->last_sum / graph->last_nb),
          //value, (int16_t) graph->last_sum, graph->last_nb,
          p(0), p(1), p(2), p(3), p(4), p(5), p(6), p(7), p(8), p(9),
          p(10), p(11), p(12), p(13), p(14), p(15), p(16), p(17), p(18), p(19)
  );*/
  graph->last_index = index;
}

#define GRAPH_BLOCK_SIZE 6
void graph_draw(GContext* ctx, GRect bounds, GraphData* graph, GraphRange* colors, uint8_t nb_colors, TextLayer* text_layer, int min_block_value, bool stacked) {
  //LOG_INFO("graph_draw nb_colors:%d stacked:%d", nb_colors, stacked);
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
  if (min_block_value > 0) {
    int min_delta_min_max = min_block_value * bounds.size.h / GRAPH_BLOCK_SIZE;
    if (max - min < min_delta_min_max) {
      // min and max are to close, force bigger max
      // "flatter" graph if values are too close
      int available = min_delta_min_max - (max - min);
      min = min - available / 3;
      max = max + 2 * available / 3;
    }
  }

  int coeff100 = max != min ? 100 * bounds.size.h / (max-min) : 0;
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "min=%d max=%d coeff100=%d", min, max, coeff100);

  for (int i = 0; i < GRAPH_NB_POINTS; i++) {
    if (graph->points[i] != GRAPH_UNDEF_POINT) {
      int height = (graph->points[i] - min) * coeff100 / 100;
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "%d: pts=%d height=%d", i, graph->points[i], height);
      for (int j = 0; j <= (int) (height / GRAPH_BLOCK_SIZE); j++) {
#ifdef PBL_COLOR
        GColor color = GColorWhite;
        for (int k = 0; k < nb_colors; k++) {
          if (stacked) {
            if ((j * GRAPH_BLOCK_SIZE * 100 / coeff100) + min >= colors[k].min) {
              color = colors[k].color;
            }
          } else {
            if (graph->points[i] >= colors[k].min) {
              color = colors[k].color;
            }
          }
        }
        //LOG_INFO("color=%x", color.argb);
        graphics_context_set_fill_color(ctx, color);
#else
        graphics_context_set_fill_color(ctx, GColorBlack);
#endif
        int height2 = j < (int) (height / GRAPH_BLOCK_SIZE) ? GRAPH_BLOCK_SIZE : height % GRAPH_BLOCK_SIZE;
        graphics_fill_rect(ctx, GRect(bounds.origin.x + i * GRAPH_BLOCK_SIZE, bounds.origin.y + bounds.size.h - j * GRAPH_BLOCK_SIZE, GRAPH_BLOCK_SIZE - 1, -(height2-1)), 0, GCornerNone);
      }
    }
  }
  if (text_layer != NULL) {
    GRect rect = layer_get_bounds(text_layer_get_layer(text_layer));
    int w0 = rect.size.w;
    int h0 = rect.size.h;

    rect.size = text_layer_get_content_size(text_layer);
    rect.size.h -= 8;
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "rect.size.w=%d", rect.size.w);

    rect.origin.x += (w0-rect.size.w) / 2;// + 1;
    rect.origin.y += (h0-rect.size.h) / 2 - 2;

#ifndef PBL_PLATFORM_APLITE
    // https://developer.pebble.com/docs/c/Graphics/Graphics_Context/#graphics_context_set_compositing_mode
    // At the moment, this only affects the bitmaps drawing operations - graphics_draw_bitmap_in_rect(), graphics_draw_rotated_bitmap, and anything that uses those APIs -, but it currently does not affect the filling or stroking operations.
    graphics_context_set_compositing_mode(ctx, GCompOpSet);

    graphics_draw_bitmap_in_rect(ctx, mask, rect);
#else
    graphics_context_set_fill_color(ctx, BG_COLOR_WINDOW);
    graphics_fill_rect(ctx, rect, 0, GCornerNone);
#endif
  }
}
