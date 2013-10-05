#include "pebble_os.h"
#include "pebble_app.h"
#include "config.h"
#include "menu.h"
#include "pebblebike.h"
#include "communication.h"

static Window window;
static SimpleMenuLayer menu_layer;
static SimpleMenuSection menu_sections[3]; // Sections
static SimpleMenuItem menu_section0_items[2]; // Section 0 (Actions)
static SimpleMenuItem menu_section1_items[3]; // Section 1 (Buttons)
static SimpleMenuItem menu_section2_items[3]; // Section 2 (About)

/**
 * Private functions
 */

void handle_appear(Window *window)
{
  scroll_layer_set_frame(&menu_layer.menu.scroll_layer, window->layer.bounds);
}


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
 
  window_init(&window, "Menu");
  window_set_window_handlers(&window, (WindowHandlers) {
    .appear = (WindowHandler)handle_appear,
  });
  
  window_set_background_color(&window, GColorWhite);
  int i = 0, s = 0;
    
  // Section "Acions"
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
  // Header
  menu_sections[s++] = (SimpleMenuSection) {
    .title = "Buttons",
    .items = menu_section1_items,
    .num_items = ARRAY_LENGTH(menu_section1_items)
  };  
  
  
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
  // Header
  menu_sections[s++] = (SimpleMenuSection) {
    .title = "About",
    .items = menu_section2_items,
    .num_items = ARRAY_LENGTH(menu_section2_items)
  };
  
  simple_menu_layer_init(&menu_layer, window.layer.frame, &window, menu_sections, ARRAY_LENGTH(menu_sections), NULL);
  layer_add_child(&window.layer, &menu_layer.menu.scroll_layer.layer);
}

/**
 * Public functions
 */

void menu_show()
{
    init_settings_window();
    
    window_stack_push(&window, true);
}