#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include <stdint.h>
#include <string.h>

// 5DD35873-3BB6-44D6-8255-0E61BC3B97F5
#define MY_UUID { 0x5D, 0xD3, 0x58, 0x73, 0x3B, 0xB6, 0x44, 0xD6, 0x82, 0x55, 0x0E, 0x61, 0xBC, 0x3B, 0x97, 0xF5 }
PBL_APP_INFO(MY_UUID,
             "Pebble Bike 1.3.0-beta3", "N Jackson",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_STANDARD_APP);

#define ROCKSHOT true

#if ROCKSHOT
#include "rockshot.h"
#endif

#define NUMBER_OF_IMAGES 11
#define TOTAL_IMAGE_SLOTS 4
#define NOT_USED -1
             
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
#include "pebblebike.h"


// map layer
Layer path_layer;

#define NUM_POINTS 1100
GPoint pts[NUM_POINTS];
int cur_point = 0;
int nb_points = 0;
// in meters/pixels
#define MAP_SCALE_MIN 500
#define MAP_SCALE_MAX 32000
int map_scale = MAP_SCALE_MIN * 2;
#define MAP_VSIZE_X 4000
#define MAP_VSIZE_Y 4000

#define XINI MAP_VSIZE_X/2
#define YINI MAP_VSIZE_Y/2

int32_t xposprev = 0, yposprev = 0;

GRect pathFrame;

void update_map(bool force_recenter);
void update_location() {
  
  if (s_gpsdata.xpos == xposprev && s_gpsdata.ypos == yposprev) {
      /*snprintf(s_data.debug2, sizeof(s_data.debug2),
        "#11 nbpoints:%u\npos : %ld|%ld\nposprev : %ld|%ld\n",
        nb_points,
        s_gpsdata.xpos, s_gpsdata.ypos,
        xposprev, yposprev
      );*/
      return;
  }
  //vibes_short_pulse();
  xposprev = s_gpsdata.xpos;
  yposprev = s_gpsdata.ypos;
    
  cur_point = nb_points % NUM_POINTS;
  nb_points++;

  pts[cur_point] = GPoint(s_gpsdata.xpos, s_gpsdata.ypos);
  
  
  if (s_data.page_number == PAGE_MAP) {
    // refresh displayed map only if current page is PAGE_MAP
    update_map(false);
  }

}

void update_map(bool force_recenter) {
  int x, y;
  int debug = 0, debug2 = 0;
  
  x = (XINI + (s_gpsdata.xpos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
  y = (YINI - (s_gpsdata.ypos * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;

  bool need_recenter = false;
  if (x + pathFrame.origin.x < SCREEN_W/4) {
    need_recenter = true;
    debug += 1;
  }
  if (3*SCREEN_W/4 < x + pathFrame.origin.x) {
    need_recenter = true;
    debug += 1;
  }
  if (y + pathFrame.origin.y < SCREEN_H/4) {
    need_recenter = true;
    debug2 += 1;
  }
  if (3*SCREEN_H/4 < y + pathFrame.origin.y) {
    need_recenter = true;
    debug2 += 1;
  }

  if (need_recenter || force_recenter) {
    //vibes_short_pulse();
    pathFrame.origin.x = -x + SCREEN_W/2;
    pathFrame.origin.y = -y + SCREEN_H/2;
    layer_set_frame(&path_layer, pathFrame);  
  }

  snprintf(s_data.debug2, sizeof(s_data.debug2),
    "#12 nbpts:%u\npos : %d|%d\nx|y:%d|%d\ndebug:%u|%u\nscale:%d\nvsize:%d|%d",
    nb_points,
    s_gpsdata.xpos, s_gpsdata.ypos,
    x, y,
    debug, debug2,
    map_scale,
    MAP_VSIZE_X, MAP_VSIZE_Y
  );

  // Update the layer
  layer_mark_dirty(&path_layer);    
}

void path_layer_update_callback(Layer *me, GContext *ctx) {
  (void)me;

  graphics_context_set_stroke_color(ctx, GColorBlack);

  GPoint p0, p1;
  
  if (nb_points < 2) {
      return;
  }

  for (int i = 0; i < ((nb_points > NUM_POINTS ? NUM_POINTS : nb_points) - 1); i++) {
    p0 = pts[(cur_point-i) % NUM_POINTS];
    p1 = pts[(cur_point-i-1) % NUM_POINTS];

    p0.x = (XINI + (p0.x * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
    p0.y = (YINI - (p0.y * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y;
    p1.x = (XINI + (p1.x * SCREEN_W / (map_scale/10))) % MAP_VSIZE_X;
    p1.y = (YINI - (p1.y * SCREEN_W / (map_scale/10))) % MAP_VSIZE_Y; 

    graphics_draw_line(
        ctx,
        p0,
        p1
    );
  }

}

void page_map_layer_init(Window* window) {

  for (int i = 0; i < NUM_POINTS; i++) {
    pts[i] = GPoint(0, 0);
  }
    
  layer_init(&s_data.page_map, GRect(0,0,SCREEN_W,SCREEN_H));
  layer_add_child(&window->layer, &s_data.page_map);

  pathFrame = GRect(0, 0, MAP_VSIZE_X, MAP_VSIZE_Y);
  layer_init(&path_layer, pathFrame);
  pathFrame.origin.x = -XINI + SCREEN_W/2;
  pathFrame.origin.y = -YINI + SCREEN_H/2;
  layer_set_frame(&path_layer, pathFrame);
  path_layer.update_proc = path_layer_update_callback;
  layer_add_child(&s_data.page_map, &path_layer);
    
  layer_set_hidden(&s_data.page_map, true);
  
}




void speed_layer_update_proc(SpeedLayer *speed_layer, GContext* ctx) {

  graphics_context_set_fill_color(app_get_current_graphics_context(), GColorBlack);
  graphics_fill_rect(app_get_current_graphics_context(), layer_get_frame(&speed_layer->layer), 0, GCornerNone);

  if (speed_layer->text && strlen(speed_layer->text) > 0) {
    
    int len = strlen(speed_layer->text);

    if(len > 4)
      len=3;

    // number of dots
    int dots = 0;
    for (int c=0; c < len; c++) {
      if (speed_layer->text[c] == ',') {
        // convert commas to dots (older app sent localized numbers...)
        speed_layer->text[c] = '.';
      }
      if (speed_layer->text[c] == '.') {
        dots++;
      }
    }
    
    // get the size
    int size = (len - dots) * CHAR_WIDTH + dots * DOT_WIDTH;

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
  Tuplet value = TupletInteger(CMD_BUTTON_PRESS, cmd);
  
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
  layer_set_hidden(&s_data.page_live_tracking, true);
  layer_set_hidden(&s_data.page_map, true);
  layer_set_hidden(&s_data.page_debug1, true);
  layer_set_hidden(&s_data.page_debug2, true);
  if (s_data.page_number == PAGE_SPEED) {
    layer_set_hidden(&s_data.page_speed, false);
//    layer_mark_dirty(&s_data.speed_layer.layer);
//    layer_mark_dirty(&s_data.distance_layer.layer);
//    layer_mark_dirty(&s_data.avgspeed_layer.layer);
  }
  if (s_data.page_number == PAGE_ALTITUDE) {
    layer_set_hidden(&s_data.page_altitude, false);
  }
  if (s_data.page_number == PAGE_LIVE_TRACKING) {
	  layer_set_hidden(&s_data.page_live_tracking, false);
  }
  if (s_data.page_number == PAGE_MAP) {
	  layer_set_hidden(&s_data.page_map, false);
      //vibes_short_pulse();
  }
  if (s_data.page_number == PAGE_DEBUG1) {
	  layer_set_hidden(&s_data.page_debug1, false);
  }
  if (s_data.page_number == PAGE_DEBUG2) {
	  layer_set_hidden(&s_data.page_debug2, false);
  }

}

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
	update_layers();
}
void handle_bottombutton_click(ClickRecognizerRef recognizer, void *context) {
  map_scale = map_scale * 2;
  if (map_scale > MAP_SCALE_MAX) {
      map_scale = MAP_SCALE_MIN;
  }
  update_map(true);
}
void handle_bottombutton_longclick(ClickRecognizerRef recognizer, void *context) {
  map_scale = map_scale / 2;
  if (map_scale < MAP_SCALE_MIN) {
      map_scale = MAP_SCALE_MAX;
  }
  update_map(true);
}


void click_config_provider(ClickConfig **config, void *context) {
  config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) handle_bottombutton_click;
  config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) handle_selectbutton_click;
  config[BUTTON_ID_UP]->click.handler = (ClickHandler) handle_topbutton_click;
  
  // long click config:
  config[BUTTON_ID_DOWN]->long_click.handler = (ClickHandler) handle_bottombutton_longclick;  
  config[BUTTON_ID_UP]->long_click.handler = (ClickHandler) handle_topbutton_longclick;  
}
/*
static void app_send_failed(DictionaryIterator* failed, AppMessageResult reason, void* context) {
  // TODO: error handling
}
*/
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
void update_buttons(uint8_t state) {

  if(state == STATE_STOP) {
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &start_button.bmp);
  } else if(state == STATE_START) {
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &stop_button.bmp);
  } else {
    // bug?
    //vibes_short_pulse();
  }
}
void change_units(uint8_t units, bool force) {
  if ((units == s_gpsdata.units) && !force) {
    return;
  }
  s_gpsdata.units = units;
  if (s_gpsdata.units == UNITS_METRIC) {
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
}
int nbchange_state=0;
void change_state(uint8_t state, int force) {
    //uint8_t state0 = s_data.state;
  /*
  if (state == s_data.state) {
  //if ((state == s_data.state) && !force) {
    return;
  }
  */
  //vibes_short_pulse();
  s_data.state = state;

/*
    snprintf(s_data.debug2, sizeof(s_data.debug2),
      "change_state #%d\ns:%d f:%d\ns_data.state0:%d\ns_data.state:%d",
      nbchange_state,
      state, force,
      state0,
      s_data.state
      );
*/
  update_buttons(s_data.state);
  
  nbchange_state++;
}
static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  (void) old_tuple;
  
  switch (key) {

  case LIVE_TRACKING_FRIENDS:
    strncpy(s_data.friends, new_tuple->value->cstring, 90-1);
    break;

  case ALTITUDE_DATA:
    change_units((new_tuple->value->data[0] & 0b00000001) >> 0, false);
    //change_state((new_tuple->value->data[0] & 0b00000010) >> 1, 0);
    
    s_gpsdata.accuracy = new_tuple->value->data[1];
    s_gpsdata.distance = (float) (new_tuple->value->data[2] + 256 * new_tuple->value->data[3]) / 100; // in km or miles
    s_gpsdata.time = new_tuple->value->data[4] + 256 * new_tuple->value->data[5];
    if (s_gpsdata.time != 0) {
      s_gpsdata.avgspeed = s_gpsdata.distance / (float) s_gpsdata.time * 3600; // km/h or mph
    } else {
      s_gpsdata.avgspeed = 0;
    }
    s_gpsdata.speed = ((float) (new_tuple->value->data[17] + 256 * new_tuple->value->data[18])) / 10;
    
    s_gpsdata.altitude = new_tuple->value->data[6] + 256 * new_tuple->value->data[7];
    s_gpsdata.ascent = new_tuple->value->data[8] + 256 * new_tuple->value->data[9];

    //TODO: negative values
    s_gpsdata.ascentrate = new_tuple->value->data[10] + 256 * new_tuple->value->data[11];
    s_gpsdata.slope = new_tuple->value->data[12];
    
    if (new_tuple->value->data[14] >= 128) { 
        s_gpsdata.xpos = -1 * (new_tuple->value->data[13] + 256 * (new_tuple->value->data[14] - 128));
    } else {
        s_gpsdata.xpos = new_tuple->value->data[13] + 256 * new_tuple->value->data[14];
    }
    if (new_tuple->value->data[16] >= 128) { 
        s_gpsdata.ypos = -1 * (new_tuple->value->data[15] + 256 * (new_tuple->value->data[16] - 128));
    } else {
        s_gpsdata.ypos = new_tuple->value->data[15] + 256 * new_tuple->value->data[16];
    }

    snprintf(s_data.accuracy,   sizeof(s_data.accuracy),   "%d",   s_gpsdata.accuracy);
    snprintf(s_data.distance,   sizeof(s_data.distance),   "%.1f", s_gpsdata.distance);
    snprintf(s_data.avgspeed,   sizeof(s_data.avgspeed),   "%.1f", s_gpsdata.avgspeed);
    snprintf(s_data.speed,      sizeof(s_data.speed),      "%.1f", s_gpsdata.speed);
    
    snprintf(s_data.altitude,   sizeof(s_data.altitude),   "%u",   s_gpsdata.altitude);
    snprintf(s_data.ascent,     sizeof(s_data.ascent),     "%u",   s_gpsdata.ascent);
    snprintf(s_data.ascentrate, sizeof(s_data.ascentrate), "%u",   s_gpsdata.ascentrate);
    snprintf(s_data.slope,      sizeof(s_data.slope),      "%u",   s_gpsdata.slope);
    
    
    snprintf(s_data.debug1, sizeof(s_data.debug1),
      //"#%d d[0]:%d A:%u\nalt:%u asc:%u\nascr:%u sl:%u\npos:%ld|%ld #%u\nD:%.1f km T:%u\n%.1f avg:%.1f",
      "#%d us:%d|%d A:%u\nalt:%u asc:%u\npos:%d|%d #%u\n%d %d %d %d\nD:%.1f km T:%u\n%.1f avg:%.1f",
      s_gpsdata.nb_received++, s_gpsdata.units, s_data.state, s_gpsdata.accuracy,
      s_gpsdata.altitude, s_gpsdata.ascent,
      //s_gpsdata.ascentrate, s_gpsdata.slope,
      s_gpsdata.xpos, s_gpsdata.ypos, nb_points,
      new_tuple->value->data[13], new_tuple->value->data[14], new_tuple->value->data[15], new_tuple->value->data[16],
      s_gpsdata.distance, s_gpsdata.time,
      s_gpsdata.speed, s_gpsdata.avgspeed
    );
    
    update_location();
    
    //if (s_data.page_number == PAGE_SPEED) {
    //  layer_mark_dirty(&s_data.speed_layer.layer);
    //}
  
    break;
  case STATE_CHANGED:
    if (new_tuple->value->uint8 != old_tuple->value->uint8) {
      //vibes_short_pulse();
      change_state(new_tuple->value->uint8, 1);
    }
    break;
  //case MEASUREMENT_UNITS:
    //change_units(new_tuple->value->uint8, false);
    //break;
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

void page_live_tracking_layer_init(Window* window) {
	  layer_init(&s_data.page_live_tracking, GRect(0,TOPBAR_HEIGHT,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
	  layer_add_child(&window->layer, &s_data.page_live_tracking);

	  text_layer_init(&s_data.live_tracking_layer, GRect(0,0,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
	  text_layer_set_text_color(&s_data.live_tracking_layer, GColorBlack);
	  text_layer_set_background_color(&s_data.live_tracking_layer, GColorClear);
	  text_layer_set_font(&s_data.live_tracking_layer, font_18);
	  text_layer_set_text_alignment(&s_data.live_tracking_layer, GTextAlignmentLeft);
	  text_layer_set_text(&s_data.live_tracking_layer, s_data.friends);
	  layer_add_child(&s_data.page_live_tracking, &s_data.live_tracking_layer.layer);

	  layer_set_hidden(&s_data.page_live_tracking, true);
}

void page_debug1_layer_init(Window* window) {
	  layer_init(&s_data.page_debug1, GRect(0,TOPBAR_HEIGHT,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
	  layer_add_child(&window->layer, &s_data.page_debug1);

	  text_layer_init(&s_data.debug1_layer, GRect(0,0,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
	  text_layer_set_text_color(&s_data.debug1_layer, GColorBlack);
	  text_layer_set_background_color(&s_data.debug1_layer, GColorClear);
	  text_layer_set_font(&s_data.debug1_layer, font_18);
	  text_layer_set_text_alignment(&s_data.debug1_layer, GTextAlignmentLeft);
	  text_layer_set_text(&s_data.debug1_layer, s_data.debug1);
	  layer_add_child(&s_data.page_debug1, &s_data.debug1_layer.layer);

	  layer_set_hidden(&s_data.page_debug1, true);
}
void page_debug2_layer_init(Window* window) {
	  layer_init(&s_data.page_debug2, GRect(0,TOPBAR_HEIGHT,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
	  layer_add_child(&window->layer, &s_data.page_debug2);

	  text_layer_init(&s_data.debug2_layer, GRect(0,0,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
	  text_layer_set_text_color(&s_data.debug2_layer, GColorBlack);
	  text_layer_set_background_color(&s_data.debug2_layer, GColorClear);
	  text_layer_set_font(&s_data.debug2_layer, font_18);
	  text_layer_set_text_alignment(&s_data.debug2_layer, GTextAlignmentLeft);
	  text_layer_set_text(&s_data.debug2_layer, s_data.debug2);
	  layer_add_child(&s_data.page_debug2, &s_data.debug2_layer.layer);

	  layer_set_hidden(&s_data.page_debug2, true);
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
  change_units(UNITS_IMPERIAL, true);
  //strncpy(s_data.unitsSpeed, SPEED_UNIT_IMPERIAL, 8);
  //strncpy(s_data.unitsDistance, DISTANCE_UNIT_IMPERIAL, 8);
  //strncpy(s_data.altitude_layer.units, ALTITUDE_UNIT_IMPERIAL, 8);
  //strncpy(s_data.altitude_ascent.units, ALTITUDE_UNIT_IMPERIAL, 8);
  //strncpy(s_data.altitude_ascent_rate.units, ASCENT_RATE_UNIT_IMPERIAL, 8);


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

  page_live_tracking_layer_init(window);
  
  
  page_map_layer_init(window);
  
  page_debug1_layer_init(window);
  page_debug2_layer_init(window);


s_gpsdata.xpos=0;
s_gpsdata.ypos=0;
s_gpsdata.nb_received=0;

  // Initialize the action bar:
  action_bar_layer_init(&action_bar);
  action_bar_layer_add_to_window(&action_bar, window);
  action_bar_layer_set_click_config_provider(&action_bar,
                                             click_config_provider);

  action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &start_button.bmp);
  //action_bar_layer_set_icon(&action_bar, BUTTON_ID_DOWN, &reset_button.bmp);

  uint8_t data[20];
  for(int i=0; i < 20; i++) {
    data[i] = 0;
  }

  Tuplet initial_values[] = {
    //TupletCString(SPEED_TEXT, "0.0"),
    //TupletCString(DISTANCE_TEXT, "-"),
    //TupletCString(AVGSPEED_TEXT, "-"),
    TupletInteger(STATE_CHANGED,STATE_STOP), //stopped
    //TupletInteger(MEASUREMENT_UNITS,UNITS_IMPERIAL), //stopped
    //TupletCString(ALTITUDE_TEXT, "-"),
    //TupletCString(ASCENT_TEXT, "-"),
    //TupletCString(ASCENTRATE_TEXT, "-"),
    //TupletCString(SLOPE_TEXT, "-"),
    //TupletCString(ACCURACY_TEXT, "-"),
    //TupletCString(LIVE_TRACKING_FRIENDS, "+ Live Tracking +\nSetup your account in the android app\nOr join the beta:\npebblebike.com\n/live"),
    TupletCString(LIVE_TRACKING_FRIENDS, "+ Live Tracking +\nSetup your account\n\nOr join the beta:\npebblebike.com\n/live"),
    //TupletInteger(XPOS, 0),
    //TupletInteger(YPOS, 0),
    TupletBytes(ALTITUDE_DATA, data, 20),
  };

  app_sync_init(&s_data.sync, s_data.sync_buffer, 200, initial_values, ARRAY_LENGTH(initial_values),
                sync_tuple_changed_callback, sync_error_callback, NULL);

  window_stack_push(window, true /* Animated */);
  
  #if ROCKSHOT
    rockshot_init(ctx);
  #endif
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
        .outbound = 256,
      }
    }
  };
  #if ROCKSHOT
    rockshot_main(&handlers);
  #endif
  app_event_loop(params, &handlers);
}
