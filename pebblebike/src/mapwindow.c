#include "pebble_os.h"
#include "pebble_fonts.h"
#include "pebble_app.h"
#include "mapwindow.h"
#include "common.h"
#include "xprintf.h"
#include <stdint.h>
#include <string.h>

static Window window;
static AppMessageCallbacksNode app_callbacks;

//	static uint8_t album_art_data[512];
static char s_title[24];
static uint8_t map_data[20480];
static GBitmap map_bitmap;

static BitmapLayer map_layer;

TextLayer textLayer;

void show_map_window() {
    window_init(&window, "Google Map");
    window_set_window_handlers(&window, (WindowHandlers){
        .unload = window_unload,
        .load = window_load,
    });
    window_stack_push(&window, true);
}

static void window_load(Window* window) {

	strncpy(s_title,"1.",24);

	window_set_background_color(window, GColorBlack);
  	window_set_fullscreen(window, true);

	text_layer_init(&textLayer, GRect(0, 0, 144, 20));
	text_layer_set_text(&textLayer, s_title);
	text_layer_set_text_color(&textLayer, GColorWhite);
	text_layer_set_background_color(&textLayer, GColorClear);
	text_layer_set_text_alignment(&textLayer, GTextAlignmentCenter);
  	layer_add_child(&window->layer, &textLayer.layer);

	window_stack_push(window, true /* Animated */);

  	map_bitmap = (GBitmap) {
        .addr = map_data,
        .bounds = GRect(0, 0, 128, 128),
        .info_flags = 1,
        .row_size_bytes = 16,
    };

    bitmap_layer_init(&map_layer, GRect(10, 20, 128, 128));
    bitmap_layer_set_bitmap(&map_layer, &map_bitmap);
    layer_add_child(window_get_root_layer(window), &map_layer.layer);

	app_callbacks = (AppMessageCallbacksNode){
        .callbacks = {
            .in_received = app_in_received,
        }
    };
    app_message_register_callbacks(&app_callbacks);

}

static void window_unload(Window* window) {

}

static void app_in_received(DictionaryIterator *received, void* context) {
	
	Tuple* tuple = dict_find(received, PEBBLEBIKE_DATA_KEY);
    if(tuple) {
    	
  		if(tuple->length > 1 && tuple->value->data[0] != 127) {
  			size_t offset = tuple->value->data[0] * 80;
          
        xsprintf(s_title, "TIX: %d - %d",offset,tuple->length - 1);  

        memcpy(map_data + offset, tuple->value->data + 1, tuple->length - 1);
  			send_cmd(PEBBLEBIKE_DATA_ACK);
  		} else {
  			//data complete
  			layer_mark_dirty(&map_layer.layer);
  		}
  		layer_mark_dirty(&textLayer.layer);

    }

}

static void send_cmd(uint8_t cmd) {

  Tuplet value = TupletInteger(STATE_CHANGED, cmd);
  
  DictionaryIterator *iter;
  app_message_out_get(&iter);
  
  if (iter == NULL)
    return;
  
  dict_write_tuplet(iter, &value);
  dict_write_end(iter);
  
  app_message_out_send();
  app_message_out_release();

}