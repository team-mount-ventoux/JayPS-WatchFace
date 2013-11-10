#ifndef BUTTONS_H
#define BUTTONS_H

extern GBitmap *start_button;
extern GBitmap *stop_button;
//extern GBitmap *reset_button;
extern GBitmap *menu_button;
extern GBitmap *zoom_button;
extern GBitmap *next_button;
extern GBitmap *menu_up_button;
extern GBitmap *menu_down_button;

void buttons_update();
void buttons_init();
void buttons_deinit();
void click_config_provider(void *context);

#endif // BUTTONS_H
