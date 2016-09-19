#include "pebble.h"
#include "pebblebike.h"
#include "screen_config.h"
#include "health.h"
#include "graph.h"
#include "heartrate.h"
#ifdef PBL_HEALTH
bool s_health_available = false;
time_t prev_time = 0;
int prev_steps = 0;

int health_get_metric_sum(HealthMetric metric) {
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, time_start_of_today(), time(NULL));
  if (mask == HealthServiceAccessibilityMaskAvailable) {
    return (int) health_service_sum_today(metric);
  } else {
    LOG_DEBUG("Data unavailable!");
    return 0;
  }
}
static void health_handler(HealthEventType type, void *context) {
  LOG_INFO("health_handler type=%d", type);
  #ifdef ENABLE_INTERNAL_HRM
    if (type == HealthEventHeartRateUpdate || type == HealthEventSignificantUpdate) {
      HealthValue hrmValue = health_service_peek_current_value(HealthMetricHeartRateBPM);
      LOG_INFO("hrmValue=%lu", hrmValue);
      if (!s_gpsdata.received_external_hr) {
        s_gpsdata.heartrate = hrmValue;
        snprintf(s_data.heartrate,  sizeof(s_data.heartrate),  "%d",   s_gpsdata.heartrate);
        if (s_gpsdata.heartrate > 0) {
          graph_add_data(&graph_heartrates, s_gpsdata.heartrate);
          heartrate_new_data(s_gpsdata.heartrate);
          #ifdef HAS_TWO_HRM
            if (type == HealthEventHeartRateUpdate) {
              s_gpsdata.received_internal_hr = true;
              LOG_INFO("received_internal_hr:%d received_external_hr:%d", s_gpsdata.received_internal_hr, s_gpsdata.received_external_hr);
            }
          #endif
        }
      }
    }
  #endif

  if (type == HealthEventMovementUpdate || type == HealthEventSignificantUpdate) {
    int steps = health_get_metric_sum(HealthMetricStepCount);
    //LOG_DEBUG("steps:%d", steps);
    snprintf(s_data.steps, sizeof(s_data.steps), "%d", steps);
    time_t time_cur = time(NULL);
    if (prev_time == 0) {
      prev_time = time_cur;
      prev_steps = steps;
    } else if (steps > prev_steps && time_cur > prev_time + 30) {
      int cadence = (steps - prev_steps) * 60 / (time_cur - prev_time);
      //LOG_DEBUG("delta_steps:%d deltatime:%d cadence:%d", steps - prev_steps, (int) (time_cur - prev_time), cadence);
      snprintf(s_data.steps_cadence, sizeof(s_data.steps_cadence), "%d", cadence);
      prev_time = time_cur;
      prev_steps = steps;
    }

    //LOG_DEBUG("distance:%d", health_get_metric_sum(HealthMetricWalkedDistanceMeters));
    //snprintf(s_data.distance,     sizeof(s_data.distance),     "%d",   health_get_metric_sum(HealthMetricWalkedDistanceMeters));
    //LOG_DEBUG("cal:%d", health_get_metric_sum(HealthMetricActiveKCalories));
  }

  //todo add function to mark current page dirty
  if (s_data.data_subpage != SUBPAGE_UNDEF) {
    layer_mark_dirty(s_data.page_data);
  }
}

void health_init() {
  LOG_INFO("health_init");
  #ifdef ENABLE_INTERNAL_HRM
    #if PBL_API_EXISTS(health_service_set_heart_rate_sample_period)
      LOG_DEBUG("health_service_set_heart_rate_sample_period(1)");
      health_service_set_heart_rate_sample_period(1);
    #endif
  #endif
  s_health_available = health_service_events_subscribe(health_handler, NULL);
  if(!s_health_available) {
    LOG_DEBUG("Health not available!");
  }
  prev_time = 0;
  prev_steps = 0;
}
void health_deinit() {
  LOG_INFO("health_deinit");
  health_service_events_unsubscribe();

#if PBL_API_EXISTS(health_service_set_heart_rate_sample_period)
  // doc says "it is best practice to always cancel the sample period request before exiting in order to maximize battery life."
  health_service_set_heart_rate_sample_period(0);
#endif

}

void health_init_if_needed() {
  bool health = false;

  health = health || config.screenA_top_type == FIELD_STEPS;
  health = health || config.screenA_top_type == FIELD_STEPS_CADENCE;
  health = health || config.screenA_top2_type == FIELD_STEPS;
  health = health || config.screenA_top2_type == FIELD_STEPS_CADENCE;
  health = health || config.screenA_bottom_left_type == FIELD_STEPS;
  health = health || config.screenA_bottom_left_type == FIELD_STEPS_CADENCE;
  health = health || config.screenA_bottom_right_type == FIELD_STEPS;
  health = health || config.screenA_bottom_right_type == FIELD_STEPS_CADENCE;
  health = health || config.screenA_topbar_center_type == FIELD_STEPS;
  health = health || config.screenA_topbar_center_type == FIELD_STEPS_CADENCE;
  health = health || config.screenB_top_type == FIELD_STEPS;
  health = health || config.screenB_top_type == FIELD_STEPS_CADENCE;
  health = health || config.screenB_top2_type == FIELD_STEPS;
  health = health || config.screenB_top2_type == FIELD_STEPS_CADENCE;
  health = health || config.screenB_bottom_left_type == FIELD_STEPS;
  health = health || config.screenB_bottom_left_type == FIELD_STEPS_CADENCE;
  health = health || config.screenB_bottom_right_type == FIELD_STEPS;
  health = health || config.screenB_bottom_right_type == FIELD_STEPS_CADENCE;
  health = health || config.screenB_topbar_center_type == FIELD_STEPS;
  health = health || config.screenB_topbar_center_type == FIELD_STEPS_CADENCE;

#ifdef ENABLE_INTERNAL_HRM
  if (!s_gpsdata.received_external_hr) {
    health = health || config.screenA_top_type == FIELD_HEARTRATE;
    health = health || config.screenA_top2_type == FIELD_HEARTRATE;
    health = health || config.screenA_bottom_left_type == FIELD_HEARTRATE;
    health = health || config.screenA_bottom_right_type == FIELD_HEARTRATE;
    health = health || config.screenA_topbar_center_type == FIELD_HEARTRATE;
    health = health || config.screenB_top_type == FIELD_HEARTRATE;
    health = health || config.screenB_top2_type == FIELD_HEARTRATE;
    health = health || config.screenB_bottom_left_type == FIELD_HEARTRATE;
    health = health || config.screenB_bottom_right_type == FIELD_HEARTRATE;
    health = health || config.screenB_topbar_center_type == FIELD_HEARTRATE;

    // only top2 can contain graphes
    health = health || config.screenA_top2_type == FIELD_HEARTRATE_GRAPH_ONLY;
    health = health || config.screenB_top2_type == FIELD_HEARTRATE_GRAPH_ONLY;
    #ifdef PBL_COLOR
      health = health || config.screenA_top2_type == FIELD_HEARTRATE_DATA_AND_GRAPH;
      health = health || config.screenB_top2_type == FIELD_HEARTRATE_DATA_AND_GRAPH;
    #endif
  }
#endif

  if (health) {
    health_init();
  } else {
    health_deinit();
  }
}
#endif // PBL_HEALTH
