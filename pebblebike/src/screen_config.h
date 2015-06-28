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
extern uint8_t config_field;

void config_start();
void config_stop();
void config_change_field();
void config_change_type();
void screen_speed_update_config();

#endif // SCREEN_CONFIG_H