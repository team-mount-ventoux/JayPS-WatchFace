#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screens.h"
#include "screen_map.h"
#include "buttons.h"

ActionBarLayer *action_bar;
static AppTimer *disconnect_timer;

void update_screens() {
  layer_set_hidden(s_data.page_speed, true);
  //layer_set_hidden(s_data.page_altitude, true);
#ifdef ENABLE_FUNCTION_LIVE
  layer_set_hidden(menu_layer_get_layer(s_data.page_live_tracking), true);
#endif
  layer_set_hidden(s_data.page_map, true);
  window_set_background_color(s_data.window, BG_COLOR_WINDOW);
  if (s_data.data_subpage != SUBPAGE_UNDEF) {
    layer_set_hidden(s_data.page_speed, false);
  }
#ifdef ENABLE_FUNCTION_LIVE
  if (s_data.page_number == PAGE_LIVE_TRACKING) {
    window_set_background_color(s_data.window, GColorWhite);
    layer_set_hidden(menu_layer_get_layer(s_data.page_live_tracking), false);
  }
#endif
  if (s_data.page_number == PAGE_MAP) {
    window_set_background_color(s_data.window, BG_COLOR_MAP);
    layer_set_hidden(s_data.page_map, false);
    layer_mark_dirty(s_data.page_map); // TODO: really needed?
    screen_map_update_map(true); // TODO: really needed?
    //vibes_short_pulse();
  }
}

void set_layer_attr_full(TextLayer *textlayer, const char *text, GFont font, GTextAlignment text_alignment, GColor color, GColor bg_color, Layer *ParentLayer)
{
  text_layer_set_text(textlayer, text);
  text_layer_set_text_alignment(textlayer, text_alignment);
  text_layer_set_text_color(textlayer, color);
  text_layer_set_background_color(textlayer, bg_color);
  text_layer_set_font(textlayer, font);
  text_layer_set_overflow_mode(textlayer, GTextOverflowModeFill);
  if (ParentLayer != NULL) {
      layer_add_child(ParentLayer, text_layer_get_layer(textlayer));
  }
}

//void field_layer_init(Layer* parent, FieldLayer* field_layer, int16_t x, int16_t y, int16_t w, int16_t h, char* title_text, char* data_text, char* unit_text) {
//  field_layer->main_layer = layer_create(GRect(x, y, w, h));
//  layer_add_child(parent, field_layer->main_layer);
//
//  // unit
//  field_layer->unit_layer = text_layer_create(GRect(1, h - 22, w - 2, 24));
//  set_layer_attr_full(field_layer->unit_layer, unit_text, fonts_get_system_font(FONT_KEY_GOTHIC_18), GTextAlignmentCenter, COLOR_UNITS, BG_COLOR_DATA, field_layer->main_layer);
//
//  // data
//  field_layer->data_layer = text_layer_create(GRect(1, h / 2 - 18, w - 2, 32));
//
//  // title
//  field_layer->title_layer = text_layer_create(GRect(1, -1, w - 2, 24));
//  set_layer_attr_full(field_layer->title_layer, title_text, fonts_get_system_font(FONT_KEY_GOTHIC_18), GTextAlignmentCenter, COLOR_TITLE, BG_COLOR_TITLE, field_layer->main_layer);
//
//  set_layer_attr_full(field_layer->data_layer, data_text, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GTextAlignmentCenter, COLOR_DATA, BG_COLOR_DATA, field_layer->main_layer);
//}
void field_layer_deinit(FieldLayer* field_layer) {
//  if (field_layer->title_layer != NULL) {
//    text_layer_destroy(field_layer->title_layer);
//  }
  text_layer_destroy(field_layer->data_layer);
  text_layer_destroy(field_layer->unit_layer);
  //layer_destroy(field_layer->main_layer);
}
void topbar_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;
  graphics_context_set_stroke_color(ctx, BG_COLOR_TOP_BAR);
  graphics_fill_rect(ctx, GRect(0, 0, SCREEN_W, TOPBAR_HEIGHT), 0, GCornerNone);
}
void topbar_layer_init(Window* window) {
//  int16_t w = SCREEN_W - MENU_WIDTH;

  s_data.topbar_layer.layer = layer_create(GRect(0, 0, SCREEN_W, TOPBAR_HEIGHT));
  layer_set_update_proc(s_data.topbar_layer.layer , topbar_layer_update_callback);
  layer_add_child(window_get_root_layer(window), s_data.topbar_layer.layer);

  // time (centered in top bar)
  s_data.topbar_layer.field_center_layer.data_layer = text_layer_create(GRect(PAGE_OFFSET_X, PBL_IF_ROUND_ELSE(6,-8), PAGE_W, PBL_IF_ROUND_ELSE(19,25)));
  text_layer_set_background_color(s_data.topbar_layer.field_center_layer.data_layer, COLOR_TOP_BAR);
  set_layer_attr_full(s_data.topbar_layer.field_center_layer.data_layer, s_data.time, PBL_IF_ROUND_ELSE(font_roboto_bold_16, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)), GTextAlignmentCenter, COLOR_TOP_BAR, BG_COLOR_TOP_BAR, window_get_root_layer(window));

  // bluetooth icon
  s_data.topbar_layer.bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
  GRect frame = gbitmap_get_bounds(s_data.topbar_layer.bluetooth_image);
#ifdef PBL_ROUND
  frame.origin.x += 52;
  frame.origin.y += 8;
#endif
  s_data.topbar_layer.bluetooth_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(s_data.topbar_layer.bluetooth_layer, s_data.topbar_layer.bluetooth_image);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer), !bluetooth_connection_service_peek());

  // accuracy (1/3, right)
  s_data.topbar_layer.accuracy_layer = text_layer_create(GRect(PAGE_W - PBL_IF_ROUND_ELSE(18, 20) - PBL_IF_ROUND_ELSE(15, 0), PBL_IF_ROUND_ELSE(6,-8), PBL_IF_ROUND_ELSE(18, 20), PBL_IF_ROUND_ELSE(19,25)));
  set_layer_attr_full(s_data.topbar_layer.accuracy_layer, s_data.accuracy, PBL_IF_ROUND_ELSE(font_roboto_bold_16, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)), GTextAlignmentRight, COLOR_TOP_BAR, BG_COLOR_TOP_BAR, window_get_root_layer(window));
  layer_set_hidden(text_layer_get_layer(s_data.topbar_layer.accuracy_layer), s_data.state == STATE_STOP);
}

static void disconnect_timer_callback(void *data) {
  disconnect_timer = NULL;
  vibes_short_pulse();
}
void topbar_toggle_bluetooth_icon(bool connected) {
  layer_set_hidden(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer), !connected);
  if (connected) {
    //vibes_short_pulse();
    if (disconnect_timer) {
      app_timer_cancel(disconnect_timer);
    }
  } else {
    if (s_data.debug) {
      vibes_short_pulse();
    } else {
      // schedule a timer to viber in X milliseconds
      disconnect_timer = app_timer_register(5000, disconnect_timer_callback, NULL);
    }
  }
}

void topbar_layer_deinit() {
  field_layer_deinit(&s_data.topbar_layer.field_center_layer);
  text_layer_destroy(s_data.topbar_layer.accuracy_layer);
  if (disconnect_timer) {
    app_timer_cancel(disconnect_timer);
  }
  layer_remove_from_parent(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer));
  bitmap_layer_destroy(s_data.topbar_layer.bluetooth_layer);
  gbitmap_destroy(s_data.topbar_layer.bluetooth_image);
  layer_destroy(s_data.topbar_layer.layer);
}  
void action_bar_init(Window* window) {
  // Initialize the action bar:
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, start_button);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, next_button);
  //action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, reset_buttonp);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, menu_button);
  action_bar_layer_set_background_color(action_bar, COLOR_ACTION_BAR);
}
void action_bar_deinit() {
 action_bar_layer_destroy(action_bar);
}


void screen_reset_instant_data() {
  s_gpsdata.speed100 = 0;
  if (s_gpsdata.heartrate != 255) {
    s_gpsdata.heartrate = 0;
  }
  if (s_gpsdata.cadence != 255) {
    s_gpsdata.cadence = 0;
  }
  s_gpsdata.ascentrate = 0;
  s_gpsdata.speed100 = 0;
  copy_speed(s_data.speed, sizeof(s_data.speed), s_gpsdata.speed100);
  strcpy(s_data.ascentrate, "0");
  strcpy(s_data.accuracy, "-");

  if (s_data.data_subpage != SUBPAGE_UNDEF) {
    layer_mark_dirty(s_data.page_speed);
  }
}

void copy_speed(char *speed, int8_t size, int32_t speed100) {
  if (s_gpsdata.units == UNITS_RUNNING_IMPERIAL || s_gpsdata.units == UNITS_RUNNING_METRIC) {
    // pace: min per mile_or_km
    snprintf(speed, size, "%ld:%.2ld", speed100 / 100, (speed100 % 100) * 3 / 5); // /100*60=/5*3
  } else {
    // + 5: round instead of trunc
    snprintf(speed, size, "%ld.%ld", (speed100 + 5) / 100, ((speed100 + 5) % 100) / 10);
  }
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "cs:%ld => %s", speed100, speed);
}
