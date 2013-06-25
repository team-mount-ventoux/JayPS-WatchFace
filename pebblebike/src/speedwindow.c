#define NUMBER_OF_IMAGES 11
#define TOTAL_IMAGE_SLOTS 4
#define NOT_USED -1

#define CHAR_WIDTH 35
#define DOT_WIDTH 15
#define CHAR_HEIGHT 51

#define CANVAS_WIDTH 144
#define MENU_WIDTH 22

#define SPEED_UNIT_METRIC "kph"
#define SPEED_UNIT_IMPERIAL "mph"
#define DISTANCE_UNIT_METRIC "km"
#define DISTANCE_UNIT_IMPERIAL "miles"

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

TextLayer distance_layer;
TextLayer avg_layer;

Layer line_layer;
Layer bottom_layer;

typedef struct SpeedLayer {
      Layer layer;
      const char* text;
 } SpeedLayer;

 static struct AppData {
  Window window;
  SpeedLayer speed_layer;
  TextLayer distance_layer;
  TextLayer avgspeed_layer;
  TextLayer mph_layer;
  TextLayer avgmph_layer;
  TextLayer miles_layer;
  char speed[16];
  char distance[16];
  char avgspeed[16];
  char unitsSpeed[8];
  char unitsDistance[8];
  int state;
  AppSync sync;
  uint8_t sync_buffer[96];
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

void handle_topbutton_click(ClickRecognizerRef recognizer, void *context) {
  if(s_data.state == STATE_STOP)
    send_cmd(PLAY_PRESS);
  else
    send_cmd(STOP_PRESS);
}

void handle_bottombutton_click(ClickRecognizerRef recognizer, void *context) {
  send_cmd(REFRESH_PRESS);
}


void click_config_provider(ClickConfig **config, void *context) {
  config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) handle_bottombutton_click;
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
  case STATE_CHANGED:
    s_data.state = new_tuple->value->uint8;
    break;
  case MEASUREMENT_UNITS:
    if(new_tuple->value->uint8 == UNITS_METRIC) {
      strncpy(s_data.unitsSpeed, SPEED_UNIT_METRIC, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_METRIC, 8);
    } else {
      strncpy(s_data.unitsSpeed, SPEED_UNIT_IMPERIAL, 8);
      strncpy(s_data.unitsDistance, DISTANCE_UNIT_IMPERIAL, 8);
    }
    layer_mark_dirty(&s_data.miles_layer.layer);
    layer_mark_dirty(&s_data.mph_layer.layer);
    layer_mark_dirty(&s_data.avgmph_layer.layer);
    break;
  default:
    return;
  }

}

void update_buttons(int state) {

  if(state == STATE_STOP)
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &start_button.bmp);
  else
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &stop_button.bmp);

}

void line_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, GPoint(72 - MENU_WIDTH / 2, 90), GPoint(72 - MENU_WIDTH / 2, 160));
}


static void window_load(Window* window) {

  resource_init_current_app(&APP_RESOURCES);

  GFont font_12 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_12));
  GFont font_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_18));
  GFont font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_24));
  
  // set default unit of measure
  strncpy(s_data.unitsSpeed, SPEED_UNIT_IMPERIAL, 8);
  strncpy(s_data.unitsDistance, DISTANCE_UNIT_IMPERIAL, 8);
  
  heap_bitmap_init(&start_button,RESOURCE_ID_IMAGE_START_BUTTON);
  heap_bitmap_init(&stop_button,RESOURCE_ID_IMAGE_STOP_BUTTON);
  heap_bitmap_init(&reset_button,RESOURCE_ID_IMAGE_RESET_BUTTON);


  Window* window = &s_data.window;
  window_init(window, "Pebble Bike");
  window_set_background_color(&s_data.window, GColorWhite);
  window_set_fullscreen(&s_data.window, true); 


  speed_layer_init(&s_data.speed_layer,GRect(0,0,CANVAS_WIDTH,88));
  speed_layer_set_text(&s_data.speed_layer, s_data.speed);
  layer_add_child(&window->layer, &s_data.speed_layer.layer);

  // Initialize the action bar:
  action_bar_layer_init(&action_bar);
  action_bar_layer_add_to_window(&action_bar, window);
  action_bar_layer_set_click_config_provider(&action_bar,
                                             click_config_provider);

  action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &start_button.bmp);
  action_bar_layer_set_icon(&action_bar, BUTTON_ID_DOWN, &reset_button.bmp);

  text_layer_init(&s_data.mph_layer, GRect(0, 60, CANVAS_WIDTH - MENU_WIDTH, 21));
  text_layer_set_text(&s_data.mph_layer, s_data.unitsSpeed);
  text_layer_set_text_color(&s_data.mph_layer, GColorWhite);
  text_layer_set_background_color(&s_data.mph_layer, GColorClear);
  text_layer_set_font(&s_data.mph_layer, font_18);
  text_layer_set_text_alignment(&s_data.mph_layer, GTextAlignmentCenter);
  layer_add_child(&window->layer, &s_data.mph_layer.layer);


  text_layer_init(&distance_layer, GRect(2, 97, 66 - MENU_WIDTH / 2, 14));
  text_layer_set_text(&distance_layer, "distance");
  text_layer_set_text_color(&distance_layer, GColorBlack);
  text_layer_set_background_color(&distance_layer, GColorClear);
  text_layer_set_font(&distance_layer, font_12);
  text_layer_set_text_alignment(&distance_layer, GTextAlignmentCenter);
  layer_add_child(&window->layer, &distance_layer.layer);

  
  text_layer_init(&s_data.miles_layer, GRect(2, 148, 66 - MENU_WIDTH / 2, 14));
  text_layer_set_text(&s_data.miles_layer, s_data.unitsDistance);
  text_layer_set_text_color(&s_data.miles_layer, GColorBlack);
  text_layer_set_background_color(&s_data.miles_layer, GColorClear);
  text_layer_set_font(&s_data.miles_layer, font_12);
  text_layer_set_text_alignment(&s_data.miles_layer, GTextAlignmentCenter);
  layer_add_child(&window->layer, &s_data.miles_layer.layer);


  text_layer_init(&avg_layer, GRect(75 - MENU_WIDTH / 2, 90, 66 - MENU_WIDTH / 2, 28));
  text_layer_set_text(&avg_layer, "average speed");
  text_layer_set_text_color(&avg_layer, GColorBlack);
  text_layer_set_background_color(&avg_layer, GColorClear);
  text_layer_set_font(&avg_layer, font_12);
  text_layer_set_text_alignment(&avg_layer, GTextAlignmentCenter);
  layer_add_child(&window->layer, &avg_layer.layer);


  text_layer_init(&s_data.avgmph_layer, GRect(75 - MENU_WIDTH / 2, 148, 66 - MENU_WIDTH / 2, 14));
  text_layer_set_text(&s_data.avgmph_layer, s_data.unitsSpeed);
  text_layer_set_text_color(&s_data.avgmph_layer, GColorBlack);
  text_layer_set_background_color(&s_data.avgmph_layer, GColorClear);
  text_layer_set_font(&s_data.avgmph_layer, font_12);
  text_layer_set_text_alignment(&s_data.avgmph_layer, GTextAlignmentCenter);
  layer_add_child(&window->layer, &s_data.avgmph_layer.layer);


  layer_init(&line_layer, window->layer.frame);
  line_layer.update_proc = &line_layer_update_callback;
  layer_add_child(&window->layer, &line_layer);
  

  text_layer_init(&s_data.distance_layer, GRect(2, 116, 66 - MENU_WIDTH / 2, 32));
  text_layer_set_text_color(&s_data.distance_layer, GColorBlack);
  text_layer_set_background_color(&s_data.distance_layer, GColorClear);
  text_layer_set_font(&s_data.distance_layer, font_24);
  text_layer_set_text_alignment(&s_data.distance_layer, GTextAlignmentCenter);
  text_layer_set_text(&s_data.distance_layer, s_data.distance);
  layer_add_child(&window->layer, &s_data.distance_layer.layer);


  text_layer_init(&s_data.avgspeed_layer, GRect(74 - MENU_WIDTH / 2, 116, 66  - MENU_WIDTH / 2, 32));
  text_layer_set_text_color(&s_data.avgspeed_layer, GColorBlack);
  text_layer_set_background_color(&s_data.avgspeed_layer, GColorClear);
  text_layer_set_font(&s_data.avgspeed_layer, font_24);
  text_layer_set_text_alignment(&s_data.avgspeed_layer, GTextAlignmentCenter);
  text_layer_set_text(&s_data.avgspeed_layer, s_data.avgspeed);
  layer_add_child(&window->layer, &s_data.avgspeed_layer.layer);
  

  Tuplet initial_values[] = {
    TupletCString(SPEED_TEXT, "0.0"),
    TupletCString(DISTANCE_TEXT, "0.0"),
    TupletCString(AVGSPEED_TEXT, "0.0"),
    TupletInteger(STATE_CHANGED,STATE_STOP), //stopped
    TupletInteger(MEASUREMENT_UNITS,UNITS_IMPERIAL), //stopped
  };

  //app_sync_init(&s_data.sync, s_data.sync_buffer, sizeof(s_data.sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
  //              sync_tuple_changed_callback, sync_error_callback, NULL);
  
  window_stack_push(window, true /* Animated */);

}

static void window_unload(Window* window) {

   //app_sync_deinit(&s_data.sync);
   for(int n=0; n < TOTAL_IMAGE_SLOTS; n++) {
      bmp_deinit_container(&image_containers[n]);
   }

  heap_bitmap_deinit(&start_button);
  heap_bitmap_deinit(&stop_button);
  heap_bitmap_deinit(&reset_button);
}