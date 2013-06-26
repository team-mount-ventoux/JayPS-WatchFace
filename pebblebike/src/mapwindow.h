#ifndef mapwindow_h
#define mapwindow_h

#include "pebble_os.h"

static void window_unload(Window* window);
static void window_load(Window* window);
static void app_in_received(DictionaryIterator *received, void* context);
static void send_cmd(uint8_t cmd);

void show_map_window();

#endif