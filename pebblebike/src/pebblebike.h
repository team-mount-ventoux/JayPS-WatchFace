#ifndef PEBBLEBIKE_H
#define PEBBLEBIKE_H

enum {
    ALTITUDE_DATA = 0x13, // TUPLE_BYTE_ARRAY
    STATE_CHANGED = 0x14,
    MSG_VERSION_PEBBLE = 0x15,
    MSG_VERSION_ANDROID = 0x16,
    MSG_LIVE_SHORT = 0x17,
    //MSG_LIVE_ASK_NAMES = 0x18,
    MSG_LIVE_NAME0 = 0x19,
    MSG_LIVE_NAME1 = 0x20,
    MSG_LIVE_NAME2 = 0x21,
    MSG_LIVE_NAME3 = 0x22,
    MSG_LIVE_NAME4 = 0x23,
    MSG_BATTERY_LEVEL = 0x24,
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

    ORUXMAPS_START_RECORD_CONTINUE_PRESS = 0x5,
    ORUXMAPS_STOP_RECORD_PRESS = 0x6,
    ORUXMAPS_NEW_WAYPOINT_PRESS = 0x7,
};

enum {
    UNITS_IMPERIAL = 0x0,
    UNITS_METRIC = 0x1,
};

enum {
    PAGE_SPEED = 0,
    PAGE_HEARTRATE = 1,
    PAGE_ALTITUDE = 2,
    PAGE_LIVE_TRACKING = 3,
    PAGE_MAP = 4,
    PAGE_DEBUG1 = 5,
    PAGE_DEBUG2 = 6,
};
#define PAGE_FIRST PAGE_SPEED
enum {
    PERSIST_UNITS_KEY = 0x0,
};

#if DEBUG
#define NUMBER_OF_PAGES 7
#endif

#if !DEBUG
#define NUMBER_OF_PAGES 5
#endif

#define CHAR_WIDTH 35
#define DOT_WIDTH 15
#define CHAR_HEIGHT 51

#define CANVAS_WIDTH 144
#define MENU_WIDTH 22
#define TOPBAR_HEIGHT 18

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
#define HEART_RATE_UNIT "bpm"

typedef struct TopBarLayer {
    Layer *layer;
    TextLayer *time_layer;
    TextLayer *accuracy_layer;
    GBitmap *bluetooth_image;
    BitmapLayer *bluetooth_layer;
} TopBarLayer;

typedef struct SpeedLayer {
    Layer *layer;
    char* text;
} SpeedLayer;

typedef struct FieldLayer {
    Layer *main_layer;
    TextLayer *title_layer;
    TextLayer *data_layer;
    TextLayer *unit_layer;
    char units[8];
} FieldLayer;

typedef struct AppData {
    Window *window;

    Layer *page_speed;
    Layer *page_altitude;
    MenuLayer *page_live_tracking;
    Layer *page_map;

//#if DEBUG
    Layer *page_debug1;
    Layer *page_debug2;
//#endif

    TopBarLayer topbar_layer;

    SpeedLayer speed_layer;
    TextLayer *distance_layer;
    TextLayer *avgspeed_layer;
    TextLayer *mph_layer;
    TextLayer *avgmph_layer;
    TextLayer *miles_layer;

    FieldLayer altitude_layer;
    FieldLayer altitude_ascent;
    FieldLayer altitude_ascent_rate;
    FieldLayer altitude_slope;
    FieldLayer altitude_accuracy;

    TextLayer *live_tracking_layer;
//#if DEBUG
    TextLayer *debug1_layer;
    TextLayer *debug2_layer;
//#endif

    char time[6]; // xx:xx, \0 terminated
    char speed[16];
    char distance[6];
    char avgspeed[6];
    char altitude[6];
    char ascent[8];
    char ascentrate[8];
    char slope[8];
    char accuracy[5];
//#if DEBUG
    char debug1[200];
    char debug2[200];
//#endif
    char unitsSpeedOrHeartRate[8];
    char unitsSpeed[8];
    char unitsDistance[8];
    uint8_t state;
    uint8_t live;
    uint8_t debug;
    uint8_t refresh_code;
    int page_number;

    int32_t android_version;
    int32_t phone_battery_level;
} AppData;

typedef struct GPSData {
    int nb_received;
    uint8_t units;
    uint16_t time;
    int32_t speed100;
    int32_t distance100;
    int32_t avgspeed100;
    int16_t altitude;
    int16_t ascent;
    int16_t ascentrate;

    int8_t slope;
    uint8_t accuracy;
    int16_t xpos;
    int16_t ypos;
    uint16_t bearing;
    uint8_t heartrate;
} GPSData;


//////////////
// Live Data
//////////////
#define NUM_LIVE_FRIENDS 5
typedef struct LiveFriendData {
    char name[10];
    char subtitle[20];
    int16_t xpos;
    int16_t ypos;
    int32_t distance;
    uint16_t bearing;
    int16_t lastviewed;
    TextLayer *name_layer;
    GRect name_frame;
} LiveFriendData;
typedef struct LiveData {
    uint8_t nb;
    LiveFriendData friends[NUM_LIVE_FRIENDS];
    LiveFriendData *sorted_friends[NUM_LIVE_FRIENDS];
} LiveData;

extern GFont font_12, font_18, font_24;
extern AppData s_data;
extern GPSData s_gpsdata;
extern LiveData s_live;

extern char tmp[255];

void change_units(uint8_t units, bool first_time);
void change_state(uint8_t state);

#endif // PEBBLEBIKE_H
