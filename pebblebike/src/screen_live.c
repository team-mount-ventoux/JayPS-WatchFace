#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "screen_live.h"

#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 2


// A callback is used to specify the amount of sections of menu items
// With this, you can dynamically add and remove sections
uint16_t menu_get_num_sections_callback(MenuLayer *me, void *data) {
    return NUM_MENU_SECTIONS;
}


// Each section has a number of items; we use a callback to specify this
// You can also dynamically add and remove items using this
uint16_t menu_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data) {
    switch (section_index) {
    case 0:
        return s_live.nb > 0 ? s_live.nb : NUM_FIRST_MENU_ITEMS;

    default:
        return 0;
    }
}


// A callback is used to specify the height of the section header
int16_t menu_get_header_height_callback(MenuLayer *me, uint16_t section_index, void *data) {
    // This is a define provided in pebble_os.h that you may use for the default height
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

// Here we draw what each header is
void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
    // Determine which section we're working with
    switch (section_index) {
    case 0:
        // Draw title text in the section header
        menu_cell_basic_header_draw(ctx, cell_layer, "Live Tracking");
        break;
    }
}


// This is the menu item draw callback where you specify what each item should look like
void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    char subtitle[20];
    char tmp[10];

    // Determine which section we're going to draw in
    switch (cell_index->section) {
    case 0:
        if (s_live.nb > 0) {
            if (cell_index->row < s_live.nb) {
                if (s_live.sorted_friends[cell_index->row]->distance < 1000) {
                    snprintf(subtitle, sizeof(subtitle),
                             "%dm",
                             (int) s_live.sorted_friends[cell_index->row]->distance
                            );
                } else {
                    snprintf(subtitle, sizeof(subtitle),
                             "%ld.%ldkm",
                             s_live.sorted_friends[cell_index->row]->distance / 1000,
			     s_live.sorted_friends[cell_index->row]->distance % 1000 / 100
                            );
                }
                snprintf(subtitle, sizeof(subtitle),
                         "%s %u°",
                         subtitle, s_live.sorted_friends[cell_index->row]->bearing
                        );
                if (s_live.sorted_friends[cell_index->row]->lastviewed < 60) {
                    snprintf(subtitle, sizeof(subtitle),
                             "%s %d\"",
                             subtitle, s_live.sorted_friends[cell_index->row]->lastviewed
                            );
                } else {
                    snprintf(subtitle, sizeof(subtitle),
                             "%s %d'%d\"",
                             subtitle, s_live.sorted_friends[cell_index->row]->lastviewed/60, s_live.sorted_friends[cell_index->row]->lastviewed%60
                            );
                }
                snprintf(s_live.sorted_friends[cell_index->row]->subtitle, sizeof(s_live.sorted_friends[cell_index->row]->subtitle),
                         "%s",
                         subtitle
                        );

                menu_cell_basic_draw(ctx, cell_layer, s_live.sorted_friends[cell_index->row]->name, s_live.sorted_friends[cell_index->row]->subtitle, NULL);
            } else {
                //menu_cell_basic_draw(ctx, cell_layer, "2", "subtitle", NULL);
            }
        } else {
            switch (cell_index->row) {
            case 0:
                menu_cell_basic_draw(ctx, cell_layer, "Setup account", "in android app", NULL);
                break;
            case 1:
                menu_cell_basic_draw(ctx, cell_layer, "Or join the beta", "pebblebike.com/live", NULL);
                break;
            }
        }
    }
}


void screen_live_menu(bool up) {
    //vibes_short_pulse();
    menu_layer_set_selected_next(s_data.page_live_tracking, up, MenuRowAlignTop, true);
}

void screen_live_menu_update() {
    // sort sorted_friends by distance
    // the algo is not optimised (O(n²)) but it should be a problem because we only got few friends and there are almost sorted
    LiveFriendData *tmp;
    for(int i = 0; i < s_live.nb; i++) {
        for(int j = i + 1; j < s_live.nb; j++) {
            if (s_live.sorted_friends[i]->distance > s_live.sorted_friends[j]->distance) {
                tmp = s_live.sorted_friends[i];
                s_live.sorted_friends[i] = s_live.sorted_friends[j];
                s_live.sorted_friends[j] = tmp;
            }
        }
    }
    menu_layer_reload_data(s_data.page_live_tracking);
}
void screen_live_layer_init(Window* window) {
    GRect bounds = GRect(0,TOPBAR_HEIGHT,SCREEN_W-MENU_WIDTH,SCREEN_H-TOPBAR_HEIGHT);

    // Initialize the menu layer
    s_data.page_live_tracking = menu_layer_create(bounds);

    // Set all the callbacks for the menu layer
    menu_layer_set_callbacks(s_data.page_live_tracking, NULL, (MenuLayerCallbacks) {
        .get_num_sections = menu_get_num_sections_callback,
         .get_num_rows = menu_get_num_rows_callback,
          .get_header_height = menu_get_header_height_callback,
           .draw_header = menu_draw_header_callback,
            .draw_row = menu_draw_row_callback,
    });


    // Add it to the window for display
    layer_add_child(window_get_root_layer(window), menu_layer_get_layer(s_data.page_live_tracking));

    layer_set_hidden(menu_layer_get_layer(s_data.page_live_tracking), true);


    s_live.nb = 0;
    for(int i = 0; i < NUM_LIVE_FRIENDS; i++) {
        //snprintf(s_live.friends[i].name, sizeof(s_live.friends[i].name), "fr%d", i);
        strcpy(s_live.friends[i].name, "");
        s_live.friends[i].distance = 0;
        s_live.sorted_friends[i] = &s_live.friends[i];
    }
}
void screen_live_layer_deinit() {
  menu_layer_destroy(s_data.page_live_tracking);
}
