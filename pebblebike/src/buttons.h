#ifndef BUTTONS_H
#define BUTTONS_H

extern HeapBitmap start_button;
extern HeapBitmap stop_button;
//extern HeapBitmap reset_button;
extern HeapBitmap menu_button;

void buttons_update(uint8_t state);
void buttons_init();
void buttons_deinit();
void click_config_provider(ClickConfig **config, void *context);

#endif // BUTTONS_H