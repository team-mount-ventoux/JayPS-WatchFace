#include "pebble_os.h"
#include "pebble_app.h"
#include "config.h"
#include "pebblebike.h"
#include "screen_speed.h"

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

TextLayer distance_layer;
TextLayer avg_layer;
Layer line_layer;
Layer bottom_layer;


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
void speed_layer_init(SpeedLayer *speed_layer, GRect frame) {
  layer_init(&speed_layer->layer, frame);
  speed_layer->layer.update_proc = (LayerUpdateProc)speed_layer_update_proc;
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
  layer_init(&s_data.page_speed, GRect(0,TOPBAR_HEIGHT,CANVAS_WIDTH-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT));
  s_data.page_speed.update_proc = &page_speed_update_proc;
  layer_add_child(&window->layer, &s_data.page_speed);

  strcpy(s_data.speed, "0.0");
  strcpy(s_data.distance, "-");
  strcpy(s_data.avgspeed, "-");

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

void screen_speed_deinit() {
   for(int n=0; n < TOTAL_IMAGE_SLOTS; n++) {
      bmp_deinit_container(&image_containers[n]);
   }
}