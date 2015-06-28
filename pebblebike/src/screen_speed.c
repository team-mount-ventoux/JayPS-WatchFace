#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screen_speed.h"
#include "screens.h"
#include "screen_config.h"

#define NUMBER_OF_IMAGES 12
#define TOTAL_IMAGE_SLOTS 4
#define NOT_USED -1

const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_IMAGE_NUM_0, RESOURCE_ID_IMAGE_NUM_1, RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3, RESOURCE_ID_IMAGE_NUM_4, RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6, RESOURCE_ID_IMAGE_NUM_7, RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9,RESOURCE_ID_IMAGE_NUM_DOT,RESOURCE_ID_IMAGE_NUM_COLON
};

GBitmap *images[TOTAL_IMAGE_SLOTS];
BitmapLayer *image_layers[TOTAL_IMAGE_SLOTS];
int image_slots[TOTAL_IMAGE_SLOTS] = {NOT_USED,NOT_USED,NOT_USED,NOT_USED};

//TextLayer *distance_layer;
//TextLayer *avg_layer;
Layer *line_layer;
Layer *bottom_layer;

enum {
  ROTATION_DISABLED,
  ROTATION_MIN,
  ROTATION_SPEED = ROTATION_MIN,
  ROTATION_HEARTRATE,
  ROTATION_ALTITUDE,
  ROTATION_MAX,
};
int rotation = ROTATION_DISABLED;
static AppTimer *rotation_timer;

void speed_layer_update_proc(Layer *layer, GContext* ctx) {
  SpeedLayer *speed_layer = &s_data.screenA_layer.speed_layer;

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_frame(speed_layer->layer), 0, GCornerNone);

  if (speed_layer->text && strlen(speed_layer->text) > 0) {
    
    // TODO: check if it's the same text?
    
    int len = strlen(speed_layer->text);

    if (len > 4) {
      len = 4;
    }

    // number of dots
    int dots = 0;
    for (int c=0; c < len; c++) {
      if (speed_layer->text[c] == ',') {
        // convert commas to dots (older app sent localized numbers...)
        speed_layer->text[c] = '.';
      }
      if (speed_layer->text[c] == '.' || speed_layer->text[c] == ':') {
        // dot or colon, same width
        dots++;
      }
    }

    // get the size
    int size = (len - dots) * CHAR_WIDTH + dots * DOT_WIDTH;

    int leftpos = (CANVAS_WIDTH - MENU_WIDTH - size) / 2;

    // clean up old layers
    for(int n=0; n < TOTAL_IMAGE_SLOTS; n++) {
      if(image_slots[n] != NOT_USED) {
        layer_remove_from_parent(bitmap_layer_get_layer(image_layers[n]));
        bitmap_layer_destroy(image_layers[n]);
        gbitmap_destroy(images[n]);
        image_slots[n] = NOT_USED;
      }
    }

    for(int c=0; c < len; c++) {

      int digit_value = -1;
      if (speed_layer->text[c] == '.') {
        digit_value = 10;
      } else if (speed_layer->text[c] == ':') {
        digit_value = 11;
      } else {
        digit_value = speed_layer->text[c] - '0';
      }

      if (digit_value >= 0 && digit_value < 12) {
        images[c] = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[digit_value]);
        image_layers[c] = bitmap_layer_create(gbitmap_get_bounds(images[c]));
        bitmap_layer_set_bitmap(image_layers[c], images[c]);
        
        GRect frame = layer_get_frame(bitmap_layer_get_layer(image_layers[c]));
        frame.origin.x = leftpos;
        frame.origin.y = 10;
        layer_set_frame(bitmap_layer_get_layer(image_layers[c]), frame);

        layer_add_child(s_data.screenA_layer.speed_layer.layer, bitmap_layer_get_layer(image_layers[c]));
        image_slots[c] = digit_value;
      }

      if (digit_value == 10 || digit_value == 11) {
        // dot or colon, same width
        leftpos += DOT_WIDTH;
      } else {
        leftpos += CHAR_WIDTH;
      }
    }
  }
}
void speed_layer_init(SpeedLayer *speed_layer, GRect frame) {
  speed_layer->layer = layer_create(frame);
  layer_set_update_proc(speed_layer->layer, speed_layer_update_proc);
}
void speed_layer_set_text(SpeedLayer *speed_layer,char* textdata) {
  speed_layer->text = textdata;
}
void page_speed_update_proc(Layer *page_speed, GContext* ctx) {
  //vibes_short_pulse();
}
void line_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, GPoint(72 - MENU_WIDTH / 2, 90), GPoint(72 - MENU_WIDTH / 2, 160));
}
void screen_speed_layer_init(Window* window) {
  s_data.screenA_layer.field_top.type = config.screenA_top_type;
  s_data.screenA_layer.field_bottom_left.type = config.screenA_bottom_left_type;
  s_data.screenA_layer.field_bottom_right.type = config.screenA_bottom_right_type;

  s_data.page_speed = layer_create(GRect(0,TOPBAR_HEIGHT,CANVAS_WIDTH-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
  layer_set_update_proc(s_data.page_speed, page_speed_update_proc);
  layer_add_child(window_get_root_layer(window), s_data.page_speed);

  strcpy(s_data.speed, "0.0");
  strcpy(s_data.distance, "-");
  strcpy(s_data.avgspeed, "-");

  speed_layer_init(&s_data.screenA_layer.speed_layer,GRect(0,0,CANVAS_WIDTH-MENU_WIDTH,84));
  speed_layer_set_text(&s_data.screenA_layer.speed_layer, s_data.speed);
  layer_add_child(s_data.page_speed, s_data.screenA_layer.speed_layer.layer);

  Layer *window_layer = window_get_root_layer(window);
  line_layer = layer_create(layer_get_frame(window_layer));
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(s_data.page_speed, line_layer);

  // BEGIN top "speed"
  // s_data.screenA_layer.field_top.title_layer NOT used

  s_data.screenA_layer.field_top.unit_layer = text_layer_create(GRect(0, 58, CANVAS_WIDTH - MENU_WIDTH, 22));
  set_layer_attr_full(s_data.screenA_layer.field_top.unit_layer, s_data.unitsSpeedOrHeartRate, font_18, GTextAlignmentCenter, GColorWhite, GColorBlack, s_data.page_speed);

  // s_data.screenA_layer.field_top.data_layer NOT used
  // END top

  // BEGIN bottom left "distance"
  s_data.screenA_layer.field_bottom_left.title_layer = text_layer_create(GRect(2, 91, 66 - MENU_WIDTH / 2, 14));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_left.title_layer, "distance", font_12, GTextAlignmentCenter, GColorBlack, GColorWhite, s_data.page_speed);

  s_data.screenA_layer.field_bottom_left.unit_layer = text_layer_create(GRect(2, 136, 66 - MENU_WIDTH / 2, 14));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_left.unit_layer, s_data.unitsDistance, font_12, GTextAlignmentCenter, GColorBlack, GColorWhite, s_data.page_speed);

  s_data.screenA_layer.field_bottom_left.data_layer = text_layer_create(GRect(1, 110, (SCREEN_W - MENU_WIDTH) / 2 - 2, 26));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_left.data_layer, s_data.distance, font_22_24, GTextAlignmentCenter, GColorBlack, GColorWhite, s_data.page_speed);
  // END bottom left

  // BEGIN bottom right "avg"
  s_data.screenA_layer.field_bottom_right.title_layer = text_layer_create(GRect(75 - MENU_WIDTH / 2, 91, 66 - MENU_WIDTH / 2, 14));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_right.title_layer, "avg speed", font_12, GTextAlignmentCenter, GColorBlack, GColorWhite, s_data.page_speed);

  s_data.screenA_layer.field_bottom_right.unit_layer = text_layer_create(GRect(75 - MENU_WIDTH / 2, 136, 66 - MENU_WIDTH / 2, 15));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_right.unit_layer, s_data.unitsSpeed, font_12, GTextAlignmentCenter, GColorBlack, GColorWhite, s_data.page_speed);

  s_data.screenA_layer.field_bottom_right.data_layer = text_layer_create(GRect((SCREEN_W - MENU_WIDTH) / 2 + 1, 110, (SCREEN_W - MENU_WIDTH) / 2 - 2, 26));
  set_layer_attr_full(s_data.screenA_layer.field_bottom_right.data_layer, s_data.avgspeed, font_22_24, GTextAlignmentCenter, GColorBlack, GColorWhite, s_data.page_speed);
  // END bottom right

  layer_set_hidden(s_data.page_speed, false);
  //vibes_double_pulse();

  //screen_speed_start_rotation();
}

void screen_speed_deinit() {
  for(int n=0; n < TOTAL_IMAGE_SLOTS; n++) {
    if(image_slots[n] != NOT_USED) {
      gbitmap_destroy(images[n]);
      bitmap_layer_destroy(image_layers[n]);
    }
  }
  
  layer_destroy(s_data.screenA_layer.speed_layer.layer);
  layer_destroy(s_data.page_speed);
  text_layer_destroy(s_data.screenA_layer.field_top.unit_layer);
  text_layer_destroy(s_data.screenA_layer.field_bottom_left.title_layer);
  text_layer_destroy(s_data.screenA_layer.field_bottom_left.data_layer);
  text_layer_destroy(s_data.screenA_layer.field_bottom_left.unit_layer);
  text_layer_destroy(s_data.screenA_layer.field_bottom_right.title_layer);
  text_layer_destroy(s_data.screenA_layer.field_bottom_right.data_layer);
  text_layer_destroy(s_data.screenA_layer.field_bottom_right.unit_layer);
  layer_destroy(line_layer);
}

void screen_speed_show_speed(bool force_units) {
  if (s_data.page_number != PAGE_SPEED && s_data.page_number != PAGE_HEARTRATE) {
    // nothing to do here
    return;
  }
  if (s_data.page_number == PAGE_HEARTRATE || rotation == ROTATION_HEARTRATE) {
    snprintf(s_data.speed, sizeof(s_data.speed), "%d", s_gpsdata.heartrate);
    if (force_units) {
      strncpy(s_data.unitsSpeedOrHeartRate, HEART_RATE_UNIT, 8);
    }
  } else if (rotation == ROTATION_ALTITUDE) {
    snprintf(s_data.speed, sizeof(s_data.speed), "%d", s_gpsdata.altitude);
    if (force_units) {
      strncpy(s_data.unitsSpeedOrHeartRate, s_data.altitude_layer.units, 8);
    }
  } else {
    if (s_gpsdata.units == UNITS_RUNNING_IMPERIAL || s_gpsdata.units == UNITS_RUNNING_METRIC) {
      // pace: min per mile_or_km
      snprintf(s_data.speed, sizeof(s_data.speed), "%ld:%.2ld", s_gpsdata.speed100 / 100, (s_gpsdata.speed100 % 100) * 3 / 5); // /100*60=/5*3
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "s_gpsdata.speed100:%ld => %s", s_gpsdata.speed100, s_data.speed);
    } else {
      // + 5: round instead of trunc
      snprintf(s_data.speed, sizeof(s_data.speed), "%ld.%ld", (s_gpsdata.speed100 + 5) / 100, ((s_gpsdata.speed100 + 5) % 100) / 10);
    }
    if (force_units) {
      strncpy(s_data.unitsSpeedOrHeartRate, s_data.unitsSpeed, 8);
    }
  }
}

static void rotation_timer_callback(void *data) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "rotation_timer");
  rotation_timer = app_timer_register(2000, rotation_timer_callback, NULL);
  rotation++;
  if (rotation == ROTATION_HEARTRATE && s_gpsdata.heartrate == 255) {
    rotation++;
  }
  if (rotation == ROTATION_MAX) {
    rotation = ROTATION_MIN;
  }
   if (rotation == ROTATION_MIN) {
    s_data.screenA_layer.field_top.type = FIELD_SPEED;
    s_data.screenA_layer.field_bottom_left.type = FIELD_DISTANCE;
    s_data.screenA_layer.field_bottom_right.type = FIELD_AVGSPEED;
  } else {
    s_data.screenA_layer.field_top.type = FIELD_ALTITUDE;
    s_data.screenA_layer.field_bottom_left.type = FIELD_ASCENT;
    s_data.screenA_layer.field_bottom_right.type = FIELD_DISTANCE;
  }
  screen_speed_show_speed(true);
  screen_speed_update_config();
  update_screens();
}
void screen_speed_start_rotation() {
  if (rotation_timer == NULL) {
    rotation_timer = app_timer_register(500, rotation_timer_callback, NULL);
  } else {
    app_timer_cancel(rotation_timer);
    rotation_timer = NULL;
    rotation = ROTATION_DISABLED;
  }
}