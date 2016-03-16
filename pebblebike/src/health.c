#include "pebble.h"
#include "pebblebike.h"
#include "screen_config.h"
#include "health.h"

int health_get_metric_sum(HealthMetric metric) {
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, time_start_of_today(), time(NULL));
  if (mask == HealthServiceAccessibilityMaskAvailable) {
    return (int) health_service_sum_today(metric);
  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Data unavailable!");
    return 0;
  }
}
static void health_handler(HealthEventType event, void *context) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "steps:%d", health_get_metric_sum(HealthMetricStepCount));
  snprintf(s_data.steps, sizeof(s_data.steps), "%d", health_get_metric_sum(HealthMetricStepCount));
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "distance:%d", health_get_metric_sum(HealthMetricWalkedDistanceMeters));
  //snprintf(s_data.distance,     sizeof(s_data.distance),     "%d",   health_get_metric_sum(HealthMetricWalkedDistanceMeters));
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "cal:%d", health_get_metric_sum(HealthMetricActiveKCalories));

  //todo add function to mark current page dirty
  if (s_data.page_number == PAGE_SPEED) {
    layer_mark_dirty(s_data.page_speed);
  }
  if (s_data.page_number == PAGE_ALTITUDE) {
    layer_mark_dirty(s_data.page_altitude);
  }
}

bool s_health_available = false;
void health_init() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "health_init");
  s_health_available = health_service_events_subscribe(health_handler, NULL);
  if(!s_health_available) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Health not available!");
  }
}
void health_deinit() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "health_deinit");
  health_service_events_unsubscribe();
}

void health_init_if_needed() {
  bool health = false;
  health = health || config.screenA_top_type == FIELD_STEPS;
  health = health || config.screenA_top2_type == FIELD_STEPS;
  health = health || config.screenA_bottom_left_type == FIELD_STEPS;
  health = health || config.screenA_bottom_right_type == FIELD_STEPS;
  health = health || config.screenB_top_left_type == FIELD_STEPS;
  health = health || config.screenB_top_right_type == FIELD_STEPS;
  health = health || config.screenB_bottom_left_type == FIELD_STEPS;
  health = health || config.screenB_bottom_right_type == FIELD_STEPS;
  if (health) {
    health_init();
  } else {
    health_deinit();
  }
}
