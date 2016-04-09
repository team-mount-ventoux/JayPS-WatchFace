#ifndef SCREEN_CONFIG_H
#define SCREEN_CONFIG_H

#define MIN_VERSION_PEBBLE_SCREEN_A_TOP2 267
#define MIN_VERSION_PEBBLE_SCREEN_A_TOP_BAR 287
enum {
  CONFIG_FIELD_SCREEN__MIN,
  CONFIG_FIELD_SCREEN_TOP = CONFIG_FIELD_SCREEN__MIN,
  CONFIG_FIELD_SCREEN_BOTTOM_LEFT,
  CONFIG_FIELD_SCREEN_BOTTOM_RIGHT,
  CONFIG_FIELD_SCREEN_TOP2,
  CONFIG_FIELD_SCREEN_TOP_BAR,
  CONFIG_FIELD_SCREEN__MAX,
};
enum {
  CONFIG_CHANGE_TYPE_PREVIOUS,
  CONFIG_CHANGE_TYPE_NEXT,
};
enum {
  CONFIG_SCREEN_DISABLED,
  CONFIG_SCREEN_A,
  CONFIG_SCREEN_B,
};
extern uint8_t config_screen;

typedef struct ConfigData {
    /// ! DO NOT CHANGE ORDER
    uint8_t screenA_top_type;
    uint8_t screenA_bottom_left_type;
    uint8_t screenA_bottom_right_type;
    uint8_t screenB_top_type;
    uint8_t screenB_top2_type;
    uint8_t screenB_bottom_left_type;
    uint8_t screenB_bottom_right_type;
    uint8_t screenA_top2_type;
    uint8_t screenA_topbar_center_type;
    uint8_t screenB_topbar_center_type;
    // ALWAYS INSERT NEW FIELD AT THE END FOR COMPATIBILITY REASON (SAVED CONFIGS ON PREVIOUS VERSIONS)
} ConfigData;
extern ConfigData config;

void config_start();
void config_stop();
void config_change_field();
void config_change_type(uint8_t direction);
void config_load();
void config_save();
void config_affect_type(FieldConfig *field, uint8_t type);
void config_field_set_text(FieldLayer field_layer, uint8_t type, GTextAlignment force_alignement);
void screen_speed_update_config(bool change_page);
void screen_altitude_update_config();


#endif // SCREEN_CONFIG_H
