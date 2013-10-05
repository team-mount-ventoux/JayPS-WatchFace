#include "pebble_os.h"
#include "pebble_app.h"
#include "config.h"
#include "pebblebike.h"
#include "buttons.h"
#include "communication.h"
#include "screens.h"
#include "screen_map.h"
#include "menu.h"

HeapBitmap start_button;
HeapBitmap stop_button;
//HeapBitmap reset_button;
HeapBitmap menu_button;

void handle_topbutton_longclick(ClickRecognizerRef recognizer, void *context) {
  vibes_short_pulse();
  send_cmd(REFRESH_PRESS);
}
void handle_topbutton_click(ClickRecognizerRef recognizer, void *context) {
  if(s_data.state == STATE_STOP)
    send_cmd(PLAY_PRESS);
  else
    send_cmd(STOP_PRESS);
}
void handle_selectbutton_click(ClickRecognizerRef recognizer, void *context) {
  s_data.page_number++;
  if (s_data.page_number >= NUMBER_OF_PAGES) {
    s_data.page_number = 0;
  }
  update_screens();
}
void handle_bottombutton_click(ClickRecognizerRef recognizer, void *context) {
  if (s_data.page_number == PAGE_MAP) {
    screen_map_zoom_out(2);
  } else {
    menu_show();
  }
}
void handle_bottombutton_longclick(ClickRecognizerRef recognizer, void *context) {
  screen_map_zoom_in(2);  
}


void click_config_provider(ClickConfig **config, void *context) {
  config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) handle_bottombutton_click;
  config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) handle_selectbutton_click;
  config[BUTTON_ID_UP]->click.handler = (ClickHandler) handle_topbutton_click;
  
  // long click config:
  config[BUTTON_ID_DOWN]->long_click.handler = (ClickHandler) handle_bottombutton_longclick;  
  config[BUTTON_ID_UP]->long_click.handler = (ClickHandler) handle_topbutton_longclick;  
}

void buttons_update(uint8_t state) {
  if(state == STATE_STOP) {
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &start_button.bmp);
  } else if(state == STATE_START) {
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &stop_button.bmp);
  } else {
    // bug?
    //vibes_short_pulse();
  }
}

void buttons_init() {
  heap_bitmap_init(&start_button,RESOURCE_ID_IMAGE_START_BUTTON);
  heap_bitmap_init(&stop_button,RESOURCE_ID_IMAGE_STOP_BUTTON);
  //heap_bitmap_init(&reset_button,RESOURCE_ID_IMAGE_RESET_BUTTON);
  heap_bitmap_init(&menu_button,RESOURCE_ID_IMAGE_MENU_BUTTON);
}
void buttons_deinit() {
  heap_bitmap_deinit(&start_button);
  heap_bitmap_deinit(&stop_button);
  //heap_bitmap_deinit(&reset_button);
  heap_bitmap_deinit(&menu_button);
}