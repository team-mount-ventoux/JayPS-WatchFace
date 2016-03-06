var rocky = Rocky.bindCanvas(document.getElementById("pebble"));
rocky.export_global_c_symbols();

/* colors */

var GColorClear=GColorRed;

var BG_COLOR1=GColorOxfordBlue;
var BG_COLOR2=GColorBlue;
var COLOR1=GColorWhite;
var COLOR2=GColorWhite;
var COLOR3=GColorWhite;

var COLOR_LINES=COLOR2;
var BG_COLOR_WINDOW=BG_COLOR2;

var COLOR_ACTION_BAR=GColorBlack;

var COLOR_TOP_BAR=COLOR1;
var BG_COLOR_TOP_BAR=GColorBlack;

var BG_COLOR_SPEED_MAIN=BG_COLOR1;
var COLOR_SPEED_UNITS=COLOR1;
var COLOR_SPEED_DATA=COLOR1;
var BG_COLOR_SPEED_UNITS=GColorClear;
var BG_COLOR_SPEED_DATA=GColorClear;
var BG_COLOR_SPEED=GColorBlack;

var BG_COLOR_TITLE=GColorClear;
var BG_COLOR_DATA=GColorClear;
var BG_COLOR_UNITS=GColorClear;
var COLOR_TITLE=COLOR2;
var COLOR_DATA=COLOR2;
var COLOR_UNITS=COLOR2;
var COLOR_LINES=COLOR2;
var COLOR_LINES_SPEED_MAIN=COLOR3;

var BG_COLOR_MAP=GColorWhite;
var COLOR_MAP=GColorBlack;

/* sizes */

var TOPBAR_HEIGHT=18;
var SCREEN_W=144;
var SCREEN_H=168;
var PAGE_OFFSET_X=0;
var PAGE_OFFSET_Y=TOPBAR_HEIGHT;
//var PAGE_OFFSET_Y=0;
var MENU_WIDTH=0;

var PAGE_W=(144-MENU_WIDTH);
//var PAGE_H=(SCREEN_H-TOPBAR_HEIGHT);
var PAGE_H=(SCREEN_H);

var PAGE_SPEED_TOP_H=SCREEN_H / 2 - TOPBAR_HEIGHT + 20;
var PAGE_SPEED_TOP_OFFSET_Y=TOPBAR_HEIGHT;
var PAGE_SPEED_MAIN_H=76;

var PAGE_SCREEN_CENTER_H=(SCREEN_H/2 - PAGE_SPEED_TOP_OFFSET_Y);

var PAGE_SPEED_MIDDLE_DATA_H=PAGE_SCREEN_CENTER_H + 8 + TOPBAR_HEIGHT;
var PAGE_SPEED_TOP_DATA_H=PAGE_SPEED_MIDDLE_DATA_H - PAGE_SPEED_MAIN_H / 2;
var PAGE_SPEED_BOTTOM_DATA_H=PAGE_SPEED_MIDDLE_DATA_H + PAGE_SPEED_MAIN_H / 2;

var altitudes = [200,220,230,240,210,180,110,250,380,420,310,500,700,800,1200,1100,900,800,700,700,560,650,620,600];
var heartrates = [135,145,150,148,150,155,162,170,180,185,182,175,170,160,155,163,165,155,162,164,168,172,175,160,145,135];


function PBL_IF_ROUND_ELSE(x,y) { return y; }

function graph(ctx, bounds, data, options) {
  min=10000;
  max=-10000;
  for (i=0; i < data.length; i++) {
    max = data[i] > max ? data[i] : max;
    min = data[i] < min ? data[i] : min;
  }
  coeff = max != min ? bounds.h / (max-min) : 0;
  size = 6;
  //console.log({min, max, coeff});
  
  for (i=0; i < data.length; i++) {
    height = (data[i] - min) * coeff;
    for (j = 0; j <= Math.floor(height / size); j++) {
      color = GColorWhite;
      if (options.colors) {
        for (k = 0; k < options.colors.length; k++) {
          //if (data[i] >= options.colors[k].min) {
          if ((j * size / coeff) + min >= options.colors[k].min) {
            color = options.colors[k].color;
          }
        }
      }
      graphics_context_set_fill_color(ctx, color);

      height2 = j < Math.floor(height / size) ? size : height % size;
      graphics_fill_rect(ctx, GRect(bounds.x + i * size, bounds.y + bounds.h - j * size, size - 1, -(height2-1)), 0, GCornerNone);
    }
  }
}
                     
rocky.update_proc = function (ctx, bounds) {
  /* window */
  graphics_context_set_fill_color(ctx, BG_COLOR_WINDOW);
  graphics_fill_rect(ctx, GRect(0, 0, SCREEN_W, SCREEN_H), 0, GCornerNone);
  /* topbar */
  graphics_context_set_fill_color(ctx, BG_COLOR_TOP_BAR);
  graphics_fill_rect(ctx, GRect(0, 0, SCREEN_W, TOPBAR_HEIGHT), 0, GCornerNone);

  /* vertical line */
  graphics_context_set_stroke_color(ctx, COLOR_LINES);
  graphics_draw_line(ctx, GPoint(PAGE_OFFSET_X + PAGE_W / 2, PAGE_SPEED_TOP_H + 2), GPoint(PAGE_OFFSET_X + PAGE_W / 2, PAGE_H - 2));
  
  graphics_context_set_fill_color(ctx, BG_COLOR_SPEED_MAIN);
  graphics_fill_rect(ctx, GRect(0, PAGE_SPEED_TOP_DATA_H, SCREEN_W, PAGE_SPEED_MAIN_H), 0, GCornerNone);

  graphics_context_set_fill_color(ctx, COLOR_LINES_SPEED_MAIN);
  graphics_fill_rect(ctx, GRect(0, PAGE_SPEED_TOP_DATA_H, SCREEN_W, 2), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(0, PAGE_SPEED_BOTTOM_DATA_H, SCREEN_W, 2), 0, GCornerNone);
  
  options = {
    colors : [
      {min:0, color:GColorGreen},
      {min:150, color:GColorOrange},
      {min:170, color:GColorRed}
    ]
  };
  
  //graph(ctx, GRect(0, PAGE_SPEED_TOP_DATA_H, SCREEN_W, PAGE_SPEED_MAIN_H), altitudes, {});
  //graph(ctx, GRect(PBL_IF_ROUND_ELSE(19, 1), PAGE_SPEED_TOP_DATA_H - 35, SCREEN_W - 2*PBL_IF_ROUND_ELSE(19, 1), 34), altitudes, {});
  graph(ctx, GRect(PBL_IF_ROUND_ELSE(19, 1), PAGE_SPEED_TOP_DATA_H - 35, SCREEN_W - 2*PBL_IF_ROUND_ELSE(19, 1), 34), heartrates, options);

  graphics_context_set_text_color(ctx, COLOR_SPEED_UNITS);
  
  // center
  graphics_draw_text(ctx, "m", fonts_get_system_font(FONT_KEY_GOTHIC_14),
    GRect(PBL_IF_ROUND_ELSE(10, 3), PAGE_SPEED_MIDDLE_DATA_H + 19, SCREEN_W - 2*PBL_IF_ROUND_ELSE(10, 3), 22),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentRight);

  //font_roboto = fonts_load_custom_font({height: 40, url:"https://github.com/pebble-examples/feature-custom-font/raw/master/resources/fonts/osp-din/OSP-DIN.ttf"}); // ko
  font_roboto = fonts_load_custom_font({height: 50, url:"https://github.com/team-mount-ventoux/PebbleVentoo-WatchFace/raw/master/pebblebike/resources/fonts/Roboto-Bold.ttf"}); // ko (height 62>50)

  // loading custom font fails, use system font instead (size 49 instead of 62)
  font_roboto = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);

  graphics_draw_text(ctx, "1982", font_roboto,
    //GRect(0, PAGE_SPEED_MIDDLE_DATA_H - 42 - PBL_IF_ROUND_ELSE(0,0), SCREEN_W, 80), // roboto 62
    GRect(0, PAGE_SPEED_MIDDLE_DATA_H - 36 - PBL_IF_ROUND_ELSE(0,0), SCREEN_W, 80),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);

  // bottom left
  graphics_draw_text(ctx, "km", fonts_get_system_font(FONT_KEY_GOTHIC_14),
    GRect(PAGE_OFFSET_X + 1, PAGE_SPEED_BOTTOM_DATA_H + 20, PAGE_W / 2 - 4, 18),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentRight);
  graphics_draw_text(ctx, "64.3", fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
    GRect(PAGE_OFFSET_X + 1, PAGE_SPEED_BOTTOM_DATA_H - 5, PAGE_W / 2 - 4, 30),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentRight);

  // bottom right
  graphics_draw_text(ctx, "km/h", fonts_get_system_font(FONT_KEY_GOTHIC_14),
    GRect(PAGE_OFFSET_X + PAGE_W / 2 + PBL_IF_ROUND_ELSE(4, 0), PAGE_SPEED_BOTTOM_DATA_H + 20, PAGE_W / 2 - 2*PBL_IF_ROUND_ELSE(4, 0), 18),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentRight);
  graphics_draw_text(ctx, "20.3", fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
    GRect(PAGE_OFFSET_X + PAGE_W / 2  + PBL_IF_ROUND_ELSE(4, 0), PAGE_SPEED_BOTTOM_DATA_H - 5, PAGE_W / 2 - 2*PBL_IF_ROUND_ELSE(4, 0), 30),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentRight);
  
  graphics_draw_text(ctx, "15:02", fonts_get_system_font(FONT_KEY_GOTHIC_14),
    GRect(PAGE_OFFSET_X, PBL_IF_ROUND_ELSE(6,-1), PAGE_W, 19),
    GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
  

};

//setInterval(function () {rocky.mark_dirty();}, 5000);