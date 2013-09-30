enum {
  
  //SPEED_TEXT = 0x1,     // TUPLE_CSTR
  //DISTANCE_TEXT = 0x2,  // TUPLE_CSTR
  //AVGSPEED_TEXT = 0x3,  // TUPLE_CSTR
  //MEASUREMENT_UNITS = 0x4, // TUPLE_INT
  //ALTITUDE_TEXT = 0x5,   // TUPLE_CSTR
  //ASCENT_TEXT = 0x6,     // TUPLE_CSTR
  //ASCENTRATE_TEXT = 0x7, // TUPLE_CSTR
  //SLOPE_TEXT = 0x8,      // TUPLE_CSTR
  //ACCURACY_TEXT = 0x9,      // TUPLE_CSTR
  LIVE_TRACKING_FRIENDS = 0x10, // TUPLE_CSTR
  //XPOS = 0x11, // TUPLE_INT
  //YPOS = 0x12, // TUPLE_INT
  ALTITUDE_DATA = 0x13, // TUPLE_BYTE_ARRAY
  STATE_CHANGED = 0x14,
  MSG_VERSION_PEBBLE = 0x15,
  MSG_VERSION_ANDROID = 0x16,
};

enum {
  STATE_STOP = 0,
  STATE_START = 1,
};

enum {
  PLAY_PRESS = 0x0,
  STOP_PRESS = 0x1,
  REFRESH_PRESS = 0x2,
  CMD_BUTTON_PRESS = 0x4,
};

enum {
  UNITS_IMPERIAL = 0x0,
  UNITS_METRIC = 0x1,
};

enum {
  PAGE_SPEED = 0,
  PAGE_ALTITUDE = 1,
  PAGE_LIVE_TRACKING = 2,
  PAGE_MAP = 3,
  PAGE_DEBUG1 = 4,
  PAGE_DEBUG2 = 5,
};

#if DEBUG
  #define NUMBER_OF_PAGES 6
#endif

#if !DEBUG
  #define NUMBER_OF_PAGES 4
#endif



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






typedef struct TopBarLayer {
  Layer layer;
  TextLayer time_layer;
  TextLayer accuracy_layer;
} TopBarLayer;

typedef struct SpeedLayer {
      Layer layer;
      char* text;
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
  Layer page_live_tracking;
  Layer page_map;

  #if DEBUG
  Layer page_debug1;
  Layer page_debug2;
  #endif
  
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

  TextLayer live_tracking_layer;
  #if DEBUG
  TextLayer debug1_layer;
  TextLayer debug2_layer;
  #endif

  char time[6]; // xx:xx, \0 terminated
  char speed[16];
  char distance[6];
  char avgspeed[6];
  char altitude[6];
  char ascent[8];
  char ascentrate[8];
  char slope[8];
  char accuracy[5];
  char friends[90];
  #if DEBUG
  char debug1[200];
  char debug2[200];
  #endif
  char unitsSpeed[8];
  char unitsDistance[8];
  uint8_t state;
  int page_number;
  AppSync sync;
  uint8_t sync_buffer[200];
  
  int32_t android_version;
} s_data;


static struct GPSData {
  int nb_received;
  uint8_t units;
  uint16_t time;
  float speed;
  float distance;
  float avgspeed;
  int16_t altitude;
  int16_t ascent;
  int16_t ascentrate;
  
  int8_t slope;
  uint8_t accuracy;
  int16_t xpos;
  int16_t ypos;
  uint16_t bearing;
} s_gpsdata;
