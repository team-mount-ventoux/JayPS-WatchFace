#ifndef COLORS_H
#define COLORS_H

#ifdef PBL_COLOR
  #define BG_COLOR1 GColorOxfordBlue
  #define BG_COLOR2 GColorBlue
  #define COLOR1 GColorWhite
  #define COLOR2 GColorWhite
  #define COLOR3 GColorWhite
#else
  #define BG_COLOR1 GColorBlack
  #define BG_COLOR2 GColorWhite
  #define COLOR1 GColorWhite
  #define COLOR2 GColorBlack
  #define COLOR3 GColorLightGray
#endif
#define BG_COLOR_WINDOW BG_COLOR2

#define COLOR_ACTION_BAR GColorBlack

#define COLOR_TOP_BAR COLOR1
#define BG_COLOR_TOP_BAR GColorBlack

#define BG_COLOR_SPEED_MAIN BG_COLOR1
#define COLOR_SPEED_UNITS COLOR1
#define COLOR_SPEED_DATA COLOR1
#define BG_COLOR_SPEED_UNITS GColorClear
#define BG_COLOR_SPEED_DATA GColorClear
#define BG_COLOR_SPEED GColorBlack

#define BG_COLOR_TITLE GColorClear
#define BG_COLOR_DATA GColorClear
#define BG_COLOR_UNITS GColorClear
#define COLOR_TITLE COLOR2
#define COLOR_DATA COLOR2
#define COLOR_UNITS COLOR2
#define COLOR_LINES COLOR2
#define COLOR_LINES_SPEED_MAIN COLOR3

#define BG_COLOR_MAP GColorWhite
#define COLOR_MAP GColorBlack

#ifdef PBL_COLOR
  #ifdef ENABLE_DEBUG_COLOR
    #undef BG_COLOR_TITLE
    #define BG_COLOR_TITLE GColorRed
    #undef BG_COLOR_DATA
    #define BG_COLOR_DATA GColorRed
    #undef BG_COLOR_UNITS
    #define BG_COLOR_UNITS GColorOrange
    #undef BG_COLOR_WINDOW
    #define BG_COLOR_WINDOW GColorYellow
    #undef BG_COLOR_SPEED
    #define BG_COLOR_SPEED GColorBulgarianRose
  #endif
#endif

#endif // COLORS_H
