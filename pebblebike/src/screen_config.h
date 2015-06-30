#ifndef SCREEN_CONFIG_H
#define SCREEN_CONFIG_H

enum {
  CONFIG_FIELD_DISABLED,
  CONFIG_FIELD__MIN,
  CONFIG_FIELD_TOP = CONFIG_FIELD__MIN,
  CONFIG_FIELD_BOTTOM_LEFT,
  CONFIG_FIELD_BOTTOM_RIGHT,
  CONFIG_FIELD__MAX,
};
enum {
  CONFIG_CHANGE_TYPE_PREVIOUS,
  CONFIG_CHANGE_TYPE_NEXT,
};
extern uint8_t config_field;

typedef struct ConfigData {
    uint8_t screenA_top_type;
    uint8_t screenA_bottom_left_type;
    uint8_t screenA_bottom_right_type;
} ConfigData;
extern ConfigData config;

void config_start();
void config_stop();
void config_change_field();
void config_change_type(uint8_t direction);
void config_load();
void config_save();
void screen_speed_update_config();


#endif // SCREEN_CONFIG_H