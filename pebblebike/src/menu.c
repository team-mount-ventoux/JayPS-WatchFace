#include "pebble.h"
#include "config.h"
#include "menu.h"
#include "pebblebike.h"
#include "communication.h"
#include "screen_config.h"
#ifdef ENABLE_LOCALIZE
  #include "localize.h"
#endif

#ifdef PBL_PLATFORM_APLITE
  ///todo remove me
  #define MENU_HELP_BUTTONS false
#else
  #define MENU_HELP_BUTTONS true
#endif

static Window *window;
static SimpleMenuLayer *menu_layer;
static SimpleMenuSection menu_sections[4]; // Sections
static SimpleMenuItem menu_section0_items[3]; // Section Actions
#ifdef ENABLE_ORUXMAPS
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
#ifdef ENABLE_ORUXMAPS
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
void menu_configure_screen(int index, void *context)
{
  config_start();
  window_stack_pop(true);
}
void init_settings_window()
{
  window_set_background_color(window, GColorWhite);
  int i = 0, s = 0;

  // Section "Actions"
  i = 0;
  menu_section0_items[i++] = (SimpleMenuItem) {
    .title = (s_data.state == STATE_STOP) ? _("Start GPS") : _("Stop GPS"),
    .callback = &menu_start_stop_data_callback,
  };
  menu_section0_items[i++] = (SimpleMenuItem) {
    .title = _("Reset data"),
    .subtitle = _("distance, time, ascent..."),
    .callback = &menu_reset_data_callback,
  };
  if (s_data.page_number == PAGE_SPEED || s_data.page_number == PAGE_ALTITUDE) {
    menu_section0_items[i++] = (SimpleMenuItem) {
      .title = _("Configure screen"),
      .subtitle = _("Change fields"),
      .callback = &menu_configure_screen,
    };
  }
  // Header
  menu_sections[s++] = (SimpleMenuSection) {
    .title = _("Actions"),
    .items = menu_section0_items,
    .num_items = i
  };

#ifdef ENABLE_ORUXMAPS
    // Section "Orux"
    i = 0;
    menu_section_orux_items[i++] = (SimpleMenuItem) {
      .title = _("Start OruxMaps"),
      .subtitle = _("Continue previous track"),
      .callback = &menu_orux_callback,
    };
    menu_section_orux_items[i++] = (SimpleMenuItem) {
      .title = _("Stop OruxMaps"),
      .callback = &menu_orux_callback,
    };
    menu_section_orux_items[i++] = (SimpleMenuItem) {
      .title = _("Add waypoint"),
      .callback = &menu_orux_callback,
    };
    // Header
    menu_sections[s++] = (SimpleMenuSection) {
      .title = _("OruxMaps Integration"),
      .items = menu_section_orux_items,
      .num_items = ARRAY_LENGTH(menu_section_orux_items)
    };
#endif

#if MENU_HELP_BUTTONS
    // Section "Buttons"
    i = 0;
    menu_section1_items[i++] = (SimpleMenuItem) {
      .title = _("Up"),
      .subtitle = _("Start/stop GPS"),
    };
    menu_section1_items[i++] = (SimpleMenuItem) {
      .title = _("Middle"),
      .subtitle = _("Change screen"),
    };
    menu_section1_items[i++] = (SimpleMenuItem) {
      .title = _("Down"),
      .subtitle = _("Menu / Zoom map"),
    };
    menu_section1_items[i++] = (SimpleMenuItem) {
      .title = _("Back"),
      .subtitle = _("Double click to exit"),
    };
    // Header
    menu_sections[s++] = (SimpleMenuSection) {
      .title = _("Buttons"),
      .items = menu_section1_items,
      .num_items = ARRAY_LENGTH(menu_section1_items)
    };
#endif


  // Section "About"
  i = 0;
  menu_section2_items[i++] = (SimpleMenuItem) {
    .title = "Ventoo",
    .subtitle = VERSION_TEXT,
  };
  menu_section2_items[i++] = (SimpleMenuItem) {
    .title = _("Authors"),
    .subtitle = APP_COMPANY,
  };
  menu_section2_items[i++] = (SimpleMenuItem) {
    .title = _("More info"),
    .subtitle = "http://pebblebike.com",
  };

  BatteryChargeState charge_state = battery_state_service_peek();
  snprintf(pebble_battery_level, sizeof(pebble_battery_level), "%d %%", charge_state.charge_percent);
  menu_section2_items[i++] = (SimpleMenuItem) {
    .title = _("Pebble battery"),
    .subtitle = pebble_battery_level,
  };
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "phone_battery_level:%ld", s_data.phone_battery_level);
  if (s_data.phone_battery_level > 0) {
    snprintf(phone_battery_level, sizeof(phone_battery_level), "%ld %%", s_data.phone_battery_level);
  } else {
    snprintf(phone_battery_level, sizeof(phone_battery_level), "-");
  }
  menu_section2_items[i++] = (SimpleMenuItem) {
    .title = _("Phone battery"),
    .subtitle = phone_battery_level,
  };
  // Header
  menu_sections[s++] = (SimpleMenuSection) {
    .title = _("About"),
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
#ifdef PBL_SDK_2
  window_set_fullscreen(window, true);
#endif

  window_set_window_handlers(window, (WindowHandlers) {
    .appear = (WindowHandler)handle_appear,
    .disappear = window_disappear,
  });
  
}
void menu_deinit() {
  window_destroy(window);
}
