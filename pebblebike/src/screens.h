#ifndef SCREENS_H
#define SCREENS_H

extern ActionBarLayer action_bar;

void update_screens();
void field_layer_init(Layer* parent, FieldLayer* field_layer, int16_t x, int16_t y, int16_t w, int16_t h, char* title_text, char* data_text, char* unit_text);
void topbar_layer_init(Window* window);
void action_bar_init(Window* window);

#endif // SCREENS_H