#include "pebble.h"
#include "config.h"
#include "menu.h"
#include "pebblebike.h"
#include "communication.h"

#define MENU_HELP_BUTTONS true

static Window *window;
static SimpleMenuLayer *menu_layer;
static SimpleMenuSection menu_sections[4]; // Sections
static SimpleMenuItem menu_section0_items[2]; // Section Actions
#if ORUXMAP
  static SimpleMenuItem menu_section_orux_items[3]; // Section OruxMap
#endif
#if MENU_HELP_BUTTONS
  static SimpleMenuItem menu_section1_items[4]; // Section Buttons
#endif
static SimpleMenuItem menu_section2_items[5]; // Section About

char phone_battery_level[6];
char pebble_battery_level[6];

/**
 * Private functions
 */

void handle_appear(Window *window)
{
  //Layer *window_layer = window_get_root_layer(window);
  //scroll_layer_set_frame(simple_menu_layer_get_layer(menu_layer), layer_get_frame(window_layer));
  // todo sdk2
  //scroll_layer_set_frame(&menu_layer.menu.scroll_layer, window->layer.bounds);
}
#if ORUXMAP
void menu_orux_callback(int index, void *context) {
  switch (index) {
  case 0:
    send_cmd(ORUXMAPS_START_RECORD_CONTINUE_PRESS);
    break;
  case 1:
    send_cmd(ORUXMAPS_STOP_RECORD_PRESS);
    break;
  case 2:
    send_cmd(ORUXMAPS_NEW_WAYPOINT_PRESS);
    break;
  }
}
#endif

void menu_start_stop_data_callback(int index, void *context) {
  if(s_data.state == STATE_STOP)
    send_cmd(PLAY_PRESS);
  else
    send_cmd(STOP_PRESS);

  window_stack_pop(true);
}
void menu_reset_data_callback(int index, void *context)
{
  vibes_short_pulse();
  send_cmd(REFRESH_PRESS);
  window_stack_pop(true);
}

void init_settings_window()
{
  window_set_background_color(window, GColorWhite);
  int i = 0, s = 0;

  // Section "Actions"
  i = 0;
  menu_section0_items[i++] = (SimpleMenuItem) {
    .title = (s_data.state == STATE_STOP) ? "Start GPS" : "Stop GPS",
    .callback = &menu_start_stop_data_callback,
  };
  menu_section0_items[i++] = (SimpleMenuItem) {
    .title = "Reset data",
    .subtitle = "distance, time, ascent...",
    .callback = &menu_reset_data_callback,
  };
  // Header
  menu_sections[s++] = (SimpleMenuSection) {
    .title = "Actions",
    .items = menu_section0_items,
    .num_items = ARRAY_LENGTH(menu_section0_items)
  };

#if ORUXMAP
    // Section "Orux"
    i = 0;
    menu_section_orux_items[i++] = (SimpleMenuItem) {
      .title = "Start OruxMap",
      .subtitle = "Continue previous track",
      .callback = &menu_orux_callback,
    };
    menu_section_orux_items[i++] = (SimpleMenuItem) {
      .title = "Stop OruxMap",
      .callback = &menu_orux_callback,
    };
    menu_section_orux_items[i++] = (SimpleMenuItem) {
      .title = "Add waypoint",
      .callback = &menu_orux_callback,
    };
    // Header
    menu_sections[s++] = (SimpleMenuSection) {
      .title = "OruxMap Integration",
      .items = menu_section_orux_items,
      .num_items = ARRAY_LENGTH(menu_section_orux_items)
    };
#endif

#if MENU_HELP_BUTTONS
    // Section "Buttons"
    i = 0;
    menu_section1_items[i++] = (SimpleMenuItem) {
      .title = "Up",
      .subtitle = "Start/stop GPS",
    };
    menu_section1_items[i++] = (SimpleMenuItem) {
      .title = "Middle",
      .subtitle = "Change screen",
    };
    menu_section1_items[i++] = (SimpleMenuItem) {
      .title = "Down",
      .subtitle = "Menu / Zoom (map)",
    };
    menu_section1_items[i++] = (SimpleMenuItem) {
      .title = "Back",
      .subtitle = "Double click to exit",
    };
    // Header
    menu_sections[s++] = (SimpleMenuSection) {
      .title = "Buttons",
      .items = menu_section1_items,
      .num_items = ARRAY_LENGTH(menu_section1_items)
    };
#endif


  // Section "About"
  i = 0;
  menu_section2_items[i++] = (SimpleMenuItem) {
    .title = "Pebble Bike",
    .subtitle = VERSION_TEXT,
  };
  menu_section2_items[i++] = (SimpleMenuItem) {
    .title = "Authors",
    .subtitle = APP_COMPANY,
  };
  menu_section2_items[i++] = (SimpleMenuItem) {
    .title = "More info",
    .subtitle = "http://pebblebike.com",
  };

  BatteryChargeState charge_state = battery_state_service_peek();
  snprintf(pebble_battery_level, sizeof(pebble_battery_level), "%d %%", charge_state.charge_percent);
  menu_section2_items[i++] = (SimpleMenuItem) {
    .title = "Pebble battery",
    .subtitle = pebble_battery_level,
  };
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "phone_battery_level:%ld", s_data.phone_battery_level);
  if (s_data.phone_battery_level > 0) {
    snprintf(phone_battery_level, sizeof(phone_battery_level), "%ld %%", s_data.phone_battery_level);
  } else {
    snprintf(phone_battery_level, sizeof(phone_battery_level), "-");
  }
  menu_section2_items[i++] = (SimpleMenuItem) {
    .title = "Phone battery",
    .subtitle = phone_battery_level,
  };
  // Header
  menu_sections[s++] = (SimpleMenuSection) {
    .title = "About",
    .items = menu_section2_items,
    .num_items = ARRAY_LENGTH(menu_section2_items)
  };

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Initialize the simple menu layer
  menu_layer = simple_menu_layer_create(bounds, window, menu_sections, ARRAY_LENGTH(menu_sections), NULL);

  // Add it to the window for display
  layer_add_child(window_layer, simple_menu_layer_get_layer(menu_layer));  
}


/**
 * Public functions
 */

void menu_show()
{
  init_settings_window();

  window_stack_push(window, true);
}

void window_disappear(Window *window) {
  simple_menu_layer_destroy(menu_layer);
}

void menu_init() {
  window = window_create();
  window_set_fullscreen(window, true);

  window_set_window_handlers(window, (WindowHandlers) {
    .appear = (WindowHandler)handle_appear,
    .disappear = window_disappear,
  });
  
}
void menu_deinit() {
  window_destroy(window);
}
