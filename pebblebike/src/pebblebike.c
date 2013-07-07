#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include <stdint.h>
#include <string.h>

// 5DD35873-3BB6-44D6-8255-0E61BC3B97F5
#define MY_UUID { 0x5D, 0xD3, 0x58, 0x73, 0x3B, 0xB6, 0x44, 0xD6, 0x82, 0x55, 0x0E, 0x61, 0xBC, 0x3B, 0x97, 0xF5 }
PBL_APP_INFO(MY_UUID,
             "Pebble Bike 1.2.0-beta2", "N Jackson",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_STANDARD_APP);

enum {
  STATE_CHANGED =0x0,
  SPEED_TEXT = 0x1,     // TUPLE_CSTR
  DISTANCE_TEXT = 0x2,  // TUPLE_CSTR
  AVGSPEED_TEXT = 0x3,  // TUPLE_CSTR
  MEASUREMENT_UNITS = 0x4, // TUPLE_INT
  ALTITUDE_TEXT = 0x5,   // TUPLE_CSTR
  ASCENT_TEXT = 0x6,     // TUPLE_CSTR
  ASCENTRATE_TEXT = 0x7, // TUPLE_CSTR
  SLOPE_TEXT = 0x8,      // TUPLE_CSTR
  ACCURACY_TEXT = 0x9,      // TUPLE_CSTR
};

enum {
  STATE_START = 1,
  STATE_STOP = 2,
};

enum {
  PLAY_PRESS =0x0,
  STOP_PRESS = 0x1,
  REFRESH_PRESS = 0x2,
};

enum {
  UNITS_IMPERIAL = 0x0,
  UNITS_METRIC = 0x1,
};

enum {
  PAGE_SPEED = 0,
  PAGE_ALTITUDE = 1,
};
#define NUMBER_OF_PAGES 2

#define NUMBER_OF_IMAGES 11
#define TOTAL_IMAGE_SLOTS 4
#define NOT_USED -1

#define CHAR_WIDTH 35
#define DOT_WIDTH 15
#define CHAR_HEIGHT 51

#define CANVAS_WIDTH 144
#define MENU_WIDTH 22
#define TOPBAR_HEIGHT 15

#define SCREEN_W 144
#define SCREEN_H 168

#define SPEED_UNIT_METRIC "km/h"
#define SPEED_UNIT_IMPERIAL "mph"
#define DISTANCE_UNIT_METRIC "km"
#define DISTANCE_UNIT_IMPERIAL "miles"
#define ALTITUDE_UNIT_METRIC "m"
#define ALTITUDE_UNIT_IMPERIAL "ft"
#define ASCENT_RATE_UNIT_METRIC "m/h"
#define ASCENT_RATE_UNIT_IMPERIAL "ft/h"

const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_IMAGE_NUM_0, RESOURCE_ID_IMAGE_NUM_1, RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3, RESOURCE_ID_IMAGE_NUM_4, RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6, RESOURCE_ID_IMAGE_NUM_7, RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9,RESOURCE_ID_IMAGE_NUM_DOT
};

BmpContainer image_containers[TOTAL_IMAGE_SLOTS];
int image_slots[TOTAL_IMAGE_SLOTS] = {NOT_USED,NOT_USED,NOT_USED,NOT_USED};

HeapBitmap start_button;
HeapBitmap stop_button;
HeapBitmap reset_button;

ActionBarLayer action_bar;

// page_speed
TextLayer distance_layer;
TextLayer avg_layer;


Layer line_layer;
Layer bottom_layer;

GFont font_12, font_18, font_24;

typedef struct TopBarLayer {
  Layer layer;
  TextLayer time_layer;
  TextLayer accuracy_layer;
} TopBarLayer;

typedef struct SpeedLayer {
      Layer layer;
      const char* text;
 } SpeedLayer;

 typedef struct FieldLayer {
    Layer main_layer;
    TextLayer title_layer;
    TextLayer data_layer;
    TextLayer unit_layer;
    char units[8];
  } FieldLayer;

 static struct AppData {
  Window window;

  Layer page_speed;
  Layer page_altitude;

  TopBarLayer topbar_layer;

  SpeedLayer speed_layer;
  TextLayer distance_layer;
  TextLayer avgspeed_layer;
  TextLayer mph_layer;
  TextLayer avgmph_layer;
  TextLayer miles_layer;

  FieldLayer altitude_layer;
  FieldLayer altitude_ascent;
  FieldLayer altitude_ascent_rate;
  FieldLayer altitude_slope;
  FieldLayer altitude_accuracy;

  char time[6]; // xx:xx, \0 terminated
  char speed[16];
  char distance[16];
  char avgspeed[16];
  char altitude[16];
  char ascent[16];
  char ascentrate[16];
  char slope[16];
  char accuracy[16];
  char unitsSpeed[8];
  char unitsDistance[8];
  int state;
  int page_number;
  AppSync sync;
  uint8_t sync_buffer[200];
} s_data;


void speed_layer_update_proc(SpeedLayer *speed_layer, GContext* ctx) {

  graphics_context_set_fill_color(app_get_current_graphics_context(), GColorBlack);
  graphics_fill_rect(app_get_current_graphics_context(), layer_get_frame(&speed_layer->layer), 0, GCornerNone);

  if (speed_layer->text && strlen(speed_layer->text) > 0) {
    
    int len = strlen(speed_layer->text);

    if(len > 4)
      return;

    // get the size
    int size = 0;
    if(len > 1)
      size = (len * CHAR_WIDTH) - 20; // dot is 20 pixels smaller
    else if(len ==1)
      size = CHAR_WIDTH;

    int leftpos = (CANVAS_WIDTH - MENU_WIDTH - size) / 2;


    // clean up old layers
    for(int n=0; n < TOTAL_IMAGE_SLOTS; n++) {
      if(image_slots[n] != NOT_USED) {
        layer_remove_from_parent(&image_containers[n].layer.layer);
        bmp_deinit_container(&image_containers[n]);
        image_slots[n] = NOT_USED;
      }
    }

    for(int c=0; c < len; c++) {

      int digit_value = -1;
      if(speed_layer->text[c] == '.') {
        digit_value = 10;
      } else {
        digit_value = speed_layer->text[c] - '0';
      }
      
      if(digit_value >=0 && digit_value < 11) {
        bmp_init_container(IMAGE_RESOURCE_IDS[digit_value], &image_containers[c]);

        GRect frame = layer_get_frame(&image_containers[c].layer.layer);
        frame.origin.x = leftpos;
        frame.origin.y = 10;
        layer_set_frame(&image_containers[c].layer.layer, frame);

        layer_add_child(&s_data.speed_layer.layer, &image_containers[c].layer.layer);
        image_slots[c] = digit_value;
      }

      if(digit_value == 10)
        leftpos += DOT_WIDTH;
      else
        leftpos += CHAR_WIDTH;
        
    }

  }
}

void bottom_layer_update_proc(SpeedLayer *speed_layer, GContext* ctx) {

}

void speed_layer_init(SpeedLayer *speed_layer, GRect frame) {
  layer_init(&speed_layer->layer, frame);
  speed_layer->layer.update_proc = (LayerUpdateProc)speed_layer_update_proc;
}

void speed_layer_set_text(SpeedLayer *speed_layer,char* textdata) {
  speed_layer->text = textdata;
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
void update_layers() {
  layer_set_hidden(&s_data.page_speed, true);
  layer_set_hidden(&s_data.page_altitude, true);
  if (s_data.page_number == PAGE_SPEED) {
    layer_set_hidden(&s_data.page_speed, false);
//    layer_mark_dirty(&s_data.speed_layer.layer);
//    layer_mark_dirty(&s_data.distance_layer.layer);
//    layer_mark_dirty(&s_data.avgspeed_layer.layer);
  }
  if (s_data.page_number == PAGE_ALTITUDE) {
    layer_set_hidden(&s_data.page_altitude, false);
  }
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
	update_layers();
}
void handle_bottombutton_click(ClickRecognizerRef recognizer, void *context) {
  send_cmd(REFRESH_PRESS);
}


void click_config_provider(ClickConfig **config, void *context) {
  config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) handle_bottombutton_click;
  config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) handle_selectbutton_click;
  config[BUTTON_ID_UP]->click.handler = (ClickHandler) handle_topbutton_click;
}

static void app_send_failed(DictionaryIterator* failed, AppMessageResult reason, void* context) {
  // TODO: error handling
}

// TODO: Error handling
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  (void) dict_error;
  (void) app_message_error;
  (void) context;
/*
  vibes_short_pulse();

  switch (dict_error) {
    case DICT_OK:
      //The operation returned successfully.
      strncpy(s_data.ascent, "OK", 16);
      break;
    case DICT_NOT_ENOUGH_STORAGE:
      strncpy(s_data.ascent, "STO", 16);
      //There was not enough backing storage to complete the operation.
      break;
    case DICT_INVALID_ARGS:
      //One or more arguments were invalid or uninitialized.
      strncpy(s_data.ascent, "INV", 16);
      break;
    case DICT_INTERNAL_INCONSISTENCY:
      //The lengths and/or count of the dictionary its tuples are inconsistent.
      strncpy(s_data.ascent, "INC", 16);
      break;
  }
  switch (app_message_error) {
    case APP_MSG_OK:
      // All good, operation was successful.
      strncpy(s_data.ascentrate, "OK", 16);
      break;
    case APP_MSG_SEND_TIMEOUT:
      // The other end did not confirm receiving the sent data with an (n)ack in time.
      strncpy(s_data.ascentrate, "NOC", 16);
      break;
    case APP_MSG_SEND_REJECTED:
      // The other end rejected the sent data, with a "nack" reply.
      strncpy(s_data.ascentrate, "NAC", 16);
      break;
    case APP_MSG_NOT_CONNECTED:
      // The other end was not connected.
      strncpy(s_data.ascentrate, "NCO", 16);
      break;
    case APP_MSG_APP_NOT_RUNNING:
      // The local application was not running.
      strncpy(s_data.ascentrate, "NOR", 16);
      break;
    case APP_MSG_INVALID_ARGS:
      // The function was called with invalid arguments.
      strncpy(s_data.ascentrate, "INV", 16);
      break;
    case APP_MSG_BUSY:
      // There are pending (in or outbound) messages that need to be processed first before new ones can be received or sent.
      strncpy(s_data.ascentrate, "BUS", 16);
      break;
    case APP_MSG_BUFFER_OVERFLOW:
      // The buffer was too small to contain the incoming message.
      strncpy(s_data.ascentrate, "OVE", 16);
      break;
    case APP_MSG_ALREADY_RELEASED:
      // The resource had already been released.
      strncpy(s_data.ascentrate, "ALR", 16);
      break;
    case APP_MSG_CALLBACK_ALREADY_REGISTERED:
      // The callback node was already registered, or its ListNode has not been initialized.
      strncpy(s_data.ascentrate, "AL2", 16);
      break;
    case APP_MSG_CALLBACK_NOT_REGISTERED:
      // The callback could not be deregistered, because it had not been registered before.
      strncpy(s_data.ascentrate, "NOR", 16);
      break;
  }
  */
}
void update_buttons(int state) {

  if(state == STATE_STOP)
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &start_button.bmp);
  else
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &stop_button.bmp);

}
static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  (void) old_tuple;
  
  switch (key) {
  case SPEED_TEXT:
    strncpy(s_data.speed, new_tuple->value->cstring, 16);
    break;
  case DISTANCE_TEXT:
    strncpy(s_data.distance, new_tuple->value->cstring, 16);
    break;
  case AVGSPEED_TEXT:
    strncpy(s_data.avgspeed, new_tuple->value->cstring, 16);
    break;
  case ALTITUDE_TEXT:
    strncpy(s_data.altitude, new_tuple->value->cstring, 16);
    break;
  case ASCENT_TEXT:
    strncpy(s_data.ascent, new_tuple->value->cstring, 16);
    break;
  case ASCENTRATE_TEXT:
    strncpy(s_data.ascentrate, new_tuple->value->cstring, 16);
    break;
  case SLOPE_TEXT:
    strncpy(s_data.slope, new_tuple->value->cstring, 16);
    break;
  case ACCURACY_TEXT:
    strncpy(s_data.accuracy, new_tuple->value->cstring, 16);
    break;
  case STATE_CHANGED:
    s_data.state = new_tuple->value->uint8;
    update_buttons(s_data.state);
    break;
  case MEASUREMENT_UNITS:
    if(new_tuple->value->uint8 == UNITS_METRIC) {
      strncpy(s_data.unitsSpeed, SPEED_UNIT_METRIC, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_METRIC, 8);
      strncpy(s_data.altitude_layer.units, ALTITUDE_UNIT_METRIC, 8);
      strncpy(s_data.altitude_ascent.units, ALTITUDE_UNIT_METRIC, 8);
      strncpy(s_data.altitude_ascent_rate.units, ASCENT_RATE_UNIT_METRIC, 8);
    } else {
      strncpy(s_data.unitsSpeed, SPEED_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_IMPERIAL, 8);
      strncpy(s_data.altitude_layer.units, ALTITUDE_UNIT_IMPERIAL, 8);
      strncpy(s_data.altitude_ascent.units, ALTITUDE_UNIT_IMPERIAL, 8);
      strncpy(s_data.altitude_ascent_rate.units, ASCENT_RATE_UNIT_IMPERIAL, 8);
    }
    layer_mark_dirty(&s_data.miles_layer.layer);
    layer_mark_dirty(&s_data.mph_layer.layer);
    layer_mark_dirty(&s_data.avgmph_layer.layer);
    break;
  default:
    return;
  }

}




void line_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, GPoint(72 - MENU_WIDTH / 2, 90), GPoint(72 - MENU_WIDTH / 2, 160));
}

void page_speed_update_proc(Layer *page_speed, GContext* ctx);
void page_altitude_update_proc(Layer *page_altitude, GContext* ctx);

void page_speed_layer_init(Window* window) {
  layer_init(&s_data.page_speed, GRect(0,TOPBAR_HEIGHT,CANVAS_WIDTH-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
  s_data.page_speed.update_proc = &page_speed_update_proc;
  layer_add_child(&window->layer, &s_data.page_speed);

  speed_layer_init(&s_data.speed_layer,GRect(0,0,CANVAS_WIDTH-MENU_WIDTH,84));
  speed_layer_set_text(&s_data.speed_layer, s_data.speed);
  layer_add_child(&s_data.page_speed, &s_data.speed_layer.layer);


  text_layer_init(&s_data.mph_layer, GRect(0, 58, CANVAS_WIDTH - MENU_WIDTH, 21));
  text_layer_set_text(&s_data.mph_layer, s_data.unitsSpeed);
  text_layer_set_text_color(&s_data.mph_layer, GColorWhite);
  text_layer_set_background_color(&s_data.mph_layer, GColorClear);
  text_layer_set_font(&s_data.mph_layer, font_18);
  text_layer_set_text_alignment(&s_data.mph_layer, GTextAlignmentCenter);
  layer_add_child(&s_data.page_speed, &s_data.mph_layer.layer);


  text_layer_init(&distance_layer, GRect(2, 91, 66 - MENU_WIDTH / 2, 14));
  text_layer_set_text(&distance_layer, "distance");
  text_layer_set_text_color(&distance_layer, GColorBlack);
  text_layer_set_background_color(&distance_layer, GColorClear);
  text_layer_set_font(&distance_layer, font_12);
  text_layer_set_text_alignment(&distance_layer, GTextAlignmentCenter);
  layer_add_child(&s_data.page_speed, &distance_layer.layer);

  
  text_layer_init(&s_data.miles_layer, GRect(2, 136, 66 - MENU_WIDTH / 2, 14));
  text_layer_set_text(&s_data.miles_layer, s_data.unitsDistance);
  text_layer_set_text_color(&s_data.miles_layer, GColorBlack);
  text_layer_set_background_color(&s_data.miles_layer, GColorClear);
  text_layer_set_font(&s_data.miles_layer, font_12);
  text_layer_set_text_alignment(&s_data.miles_layer, GTextAlignmentCenter);
  layer_add_child(&s_data.page_speed, &s_data.miles_layer.layer);


  text_layer_init(&avg_layer, GRect(75 - MENU_WIDTH / 2, 84, 66 - MENU_WIDTH / 2, 28));
  text_layer_set_text(&avg_layer, "average speed");
  text_layer_set_text_color(&avg_layer, GColorBlack);
  text_layer_set_background_color(&avg_layer, GColorClear);
  text_layer_set_font(&avg_layer, font_12);
  text_layer_set_text_alignment(&avg_layer, GTextAlignmentCenter);
  layer_add_child(&s_data.page_speed, &avg_layer.layer);


  text_layer_init(&s_data.avgmph_layer, GRect(75 - MENU_WIDTH / 2, 136, 66 - MENU_WIDTH / 2, 14));
  text_layer_set_text(&s_data.avgmph_layer, s_data.unitsSpeed);
  text_layer_set_text_color(&s_data.avgmph_layer, GColorBlack);
  text_layer_set_background_color(&s_data.avgmph_layer, GColorClear);
  text_layer_set_font(&s_data.avgmph_layer, font_12);
  text_layer_set_text_alignment(&s_data.avgmph_layer, GTextAlignmentCenter);
  layer_add_child(&s_data.page_speed, &s_data.avgmph_layer.layer);


  layer_init(&line_layer, window->layer.frame);
  line_layer.update_proc = &line_layer_update_callback;
  layer_add_child(&s_data.page_speed, &line_layer);
  

  text_layer_init(&s_data.distance_layer, GRect(1, 106, (SCREEN_W - MENU_WIDTH) / 2 - 2, 32));
  text_layer_set_text_color(&s_data.distance_layer, GColorBlack);
  text_layer_set_background_color(&s_data.distance_layer, GColorClear);
  text_layer_set_font(&s_data.distance_layer, font_24);
  text_layer_set_text_alignment(&s_data.distance_layer, GTextAlignmentCenter);
  text_layer_set_text(&s_data.distance_layer, s_data.distance);
  layer_add_child(&s_data.page_speed, &s_data.distance_layer.layer);


  text_layer_init(&s_data.avgspeed_layer, GRect((SCREEN_W - MENU_WIDTH) / 2 + 1, 106, (SCREEN_W - MENU_WIDTH) / 2 - 2, 32));
  text_layer_set_text_color(&s_data.avgspeed_layer, GColorBlack);
  text_layer_set_background_color(&s_data.avgspeed_layer, GColorClear);
  text_layer_set_font(&s_data.avgspeed_layer, font_24);
  text_layer_set_text_alignment(&s_data.avgspeed_layer, GTextAlignmentCenter);
  text_layer_set_text(&s_data.avgspeed_layer, s_data.avgspeed);
  layer_add_child(&s_data.page_speed, &s_data.avgspeed_layer.layer);

  layer_set_hidden(&s_data.page_speed, false);
  //vibes_double_pulse();
}
void page_speed_update_proc(Layer *page_speed, GContext* ctx) {
  //vibes_short_pulse();
}

void field_layer_init(Layer* parent, FieldLayer* field_layer, int16_t x, int16_t y, int16_t w, int16_t h, char* title_text, char* data_text, char* unit_text) {
  layer_init(&field_layer->main_layer, GRect(x, y, w, h));
  layer_add_child(parent, &field_layer->main_layer);


  // title
  text_layer_init(&field_layer->title_layer, GRect(1, 2, w - 2, 14));
  text_layer_set_text(&field_layer->title_layer, title_text);
  text_layer_set_text_color(&field_layer->title_layer, GColorBlack);
  text_layer_set_background_color(&field_layer->title_layer, GColorClear);
  text_layer_set_font(&field_layer->title_layer, font_12);
  text_layer_set_text_alignment(&field_layer->title_layer, GTextAlignmentCenter);
  layer_add_child(&field_layer->main_layer, &field_layer->title_layer.layer);

  // data
  text_layer_init(&field_layer->data_layer, GRect(1, 21, w - 2, 32));
  text_layer_set_text_color(&field_layer->data_layer, GColorBlack);
  text_layer_set_background_color(&field_layer->data_layer, GColorClear);
  text_layer_set_font(&field_layer->data_layer, font_24);
  text_layer_set_text_alignment(&field_layer->data_layer, GTextAlignmentCenter);
  text_layer_set_text(&field_layer->data_layer, data_text);
  layer_add_child(&field_layer->main_layer, &field_layer->data_layer.layer);

  // unit
  text_layer_init(&field_layer->unit_layer, GRect(1, h - 14, w - 2, 14));
  text_layer_set_text(&field_layer->unit_layer, unit_text);
  text_layer_set_text_color(&field_layer->unit_layer, GColorBlack);
  text_layer_set_background_color(&field_layer->unit_layer, GColorClear);
  text_layer_set_font(&field_layer->unit_layer, font_12);
  text_layer_set_text_alignment(&field_layer->unit_layer, GTextAlignmentCenter);
  layer_add_child(&field_layer->main_layer, &field_layer->unit_layer.layer);

}


void page_altitude_layer_init(Window* window) {
  layer_init(&s_data.page_altitude, GRect(0,0,SCREEN_W-MENU_WIDTH,SCREEN_H));
  s_data.page_altitude.update_proc = &page_altitude_update_proc;
  layer_add_child(&window->layer, &s_data.page_altitude);

  int16_t w = (SCREEN_W - MENU_WIDTH) / 2; //61
  int16_t h = (SCREEN_H - TOPBAR_HEIGHT) / 2 - 1; // 75

  field_layer_init(&s_data.page_altitude, &s_data.altitude_layer,       0,     TOPBAR_HEIGHT + 0,     w, h, "Altitude", s_data.altitude, s_data.altitude_layer.units);
  field_layer_init(&s_data.page_altitude, &s_data.altitude_ascent,      w + 1, TOPBAR_HEIGHT + 0,     w, h, "Ascent", s_data.ascent, s_data.altitude_ascent.units);
  field_layer_init(&s_data.page_altitude, &s_data.altitude_ascent_rate, 0,     TOPBAR_HEIGHT + h + 1, w, h, "Ascent rate", s_data.ascentrate, s_data.altitude_ascent_rate.units);
  field_layer_init(&s_data.page_altitude, &s_data.altitude_slope,       w + 1, TOPBAR_HEIGHT + h + 1, w, h, "Slope", s_data.slope, "%");
  //field_layer_init(&s_data.page_altitude, &s_data.altitude_accuracy,    w + 1, TOPBAR_HEIGHT + h + 1, w, h, "Accuracy", s_data.accuracy, "m");


  layer_set_hidden(&s_data.page_altitude, true);
}

void topbar_layer_init(Window* window) {
  int16_t w = SCREEN_W - MENU_WIDTH;

  layer_init(&s_data.topbar_layer.layer, GRect(0,0,w,SCREEN_H));
  layer_add_child(&window->layer, &s_data.topbar_layer.layer);

  // time (centered in top bar)
  text_layer_init(&s_data.topbar_layer.time_layer, GRect(0,0,w,TOPBAR_HEIGHT));
  text_layer_set_text(&s_data.topbar_layer.time_layer, s_data.time);
  text_layer_set_text_color(&s_data.topbar_layer.time_layer, GColorClear);
  text_layer_set_background_color(&s_data.topbar_layer.time_layer, GColorBlack);
  text_layer_set_font(&s_data.topbar_layer.time_layer, font_12);
  text_layer_set_text_alignment(&s_data.topbar_layer.time_layer, GTextAlignmentCenter);
  layer_add_child(&window->layer, &s_data.topbar_layer.time_layer.layer);

  // accuracy (1/3, right)
  text_layer_init(&s_data.topbar_layer.accuracy_layer, GRect(w*2/3,0,w/3,TOPBAR_HEIGHT));
  text_layer_set_text(&s_data.topbar_layer.accuracy_layer, s_data.accuracy);
  text_layer_set_text_color(&s_data.topbar_layer.accuracy_layer, GColorClear);
  text_layer_set_background_color(&s_data.topbar_layer.accuracy_layer, GColorBlack);
  text_layer_set_font(&s_data.topbar_layer.accuracy_layer, font_12);
  text_layer_set_text_alignment(&s_data.topbar_layer.accuracy_layer, GTextAlignmentRight);
  layer_add_child(&window->layer, &s_data.topbar_layer.accuracy_layer.layer);

}

void page_altitude_update_proc(Layer *page_altitude, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, GColorBlack);

  // vertical line
  graphics_draw_line(ctx, GPoint((SCREEN_W - MENU_WIDTH) / 2, TOPBAR_HEIGHT), GPoint((SCREEN_W - MENU_WIDTH) / 2, SCREEN_H));

  // horizontal line
  graphics_draw_line(ctx, GPoint(0, TOPBAR_HEIGHT + (SCREEN_H - TOPBAR_HEIGHT) / 2), GPoint(SCREEN_W - MENU_WIDTH, TOPBAR_HEIGHT + (SCREEN_H - TOPBAR_HEIGHT) / 2));
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  resource_init_current_app(&APP_RESOURCES);

  font_12 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_12));
  font_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_18));
  font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_24));

  // set default unit of measure
  strncpy(s_data.unitsSpeed, SPEED_UNIT_IMPERIAL, 8);
  strncpy(s_data.unitsDistance, DISTANCE_UNIT_IMPERIAL, 8);
  strncpy(s_data.altitude_layer.units, ALTITUDE_UNIT_IMPERIAL, 8);
  strncpy(s_data.altitude_ascent.units, ALTITUDE_UNIT_IMPERIAL, 8);
  strncpy(s_data.altitude_ascent_rate.units, ASCENT_RATE_UNIT_IMPERIAL, 8);


  heap_bitmap_init(&start_button,RESOURCE_ID_IMAGE_START_BUTTON);
  heap_bitmap_init(&stop_button,RESOURCE_ID_IMAGE_STOP_BUTTON);
  heap_bitmap_init(&reset_button,RESOURCE_ID_IMAGE_RESET_BUTTON);


  Window* window = &s_data.window;
  window_init(window, "Pebble Bike");
  window_set_background_color(&s_data.window, GColorWhite);
  window_set_fullscreen(&s_data.window, true);

  topbar_layer_init(window);

  page_speed_layer_init(window);

  page_altitude_layer_init(window);



  // Initialize the action bar:
  action_bar_layer_init(&action_bar);
  action_bar_layer_add_to_window(&action_bar, window);
  action_bar_layer_set_click_config_provider(&action_bar,
                                             click_config_provider);

  action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &start_button.bmp);
  action_bar_layer_set_icon(&action_bar, BUTTON_ID_DOWN, &reset_button.bmp);

  Tuplet initial_values[] = {
    TupletCString(SPEED_TEXT, "0.0"),
    TupletCString(DISTANCE_TEXT, "0.0"),
    TupletCString(AVGSPEED_TEXT, "0.0"),
    TupletInteger(STATE_CHANGED,STATE_STOP), //stopped
    TupletInteger(MEASUREMENT_UNITS,UNITS_IMPERIAL), //stopped
    TupletCString(ALTITUDE_TEXT, "-"),
    TupletCString(ASCENT_TEXT, "-"),
    TupletCString(ASCENTRATE_TEXT, "-"),
    TupletCString(SLOPE_TEXT, "-"),
    TupletCString(ACCURACY_TEXT, "-"),
  };

  app_sync_init(&s_data.sync, s_data.sync_buffer, 200, initial_values, ARRAY_LENGTH(initial_values),
                sync_tuple_changed_callback, sync_error_callback, NULL);

  window_stack_push(window, true /* Animated */);

}
static void handle_deinit(AppContextRef c) {
   app_sync_deinit(&s_data.sync);
   for(int n=0; n < TOTAL_IMAGE_SLOTS; n++) {
      bmp_deinit_container(&image_containers[n]);
   }

  heap_bitmap_deinit(&start_button);
  heap_bitmap_deinit(&stop_button);
  heap_bitmap_deinit(&reset_button);
}

void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)t;
  (void)ctx;
  //update_buttons(s_data.state);
  //update_layers();

  char *time_format;
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  string_format_time(s_data.time, sizeof(s_data.time), time_format, t->tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (s_data.time[0] == '0')) {
    memmove(s_data.time, &s_data.time[1], sizeof(s_data.time) - 1);
  }
  layer_mark_dirty(&s_data.topbar_layer.layer);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = MINUTE_UNIT
    },
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 200,
        .outbound = 16,
      }
    }
  };
  app_event_loop(params, &handlers);
}
