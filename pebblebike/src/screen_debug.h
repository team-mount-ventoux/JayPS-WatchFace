#ifndef SCREEN_DEBUG_H
#define SCREEN_DEBUG_H

#if DEBUG
void screen_debug1_layer_init(Window* window);
void screen_debug1_layer_deinit();
void screen_debug2_layer_init(Window* window);
void screen_debug2_layer_deinit();
#endif

#endif // SCREEN_DEBUG_H
