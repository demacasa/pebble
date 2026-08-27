#include "ui.h"

#include "fmt.h"

#define GRAY COLOR_FALLBACK(GColorLightGray, GColorWhite)
#define DIM_GRAY COLOR_FALLBACK(GColorDarkGray, GColorWhite)

static Window *s_window;
static Layer *s_status_layer; // clock + BT + GPS glyphs (custom draw)
static TextLayer *s_pace_value;
static TextLayer *s_pace_unit; // "/MI"
static TextLayer *s_band;      // one strip: PAUSED banner / idle prompt / GPS lost / exit hint
static Layer *s_divider;
static TextLayer *s_dist_value;
static TextLayer *s_dist_unit;
static TextLayer *s_duration;
static GFont s_font_pace;
static GFont s_font_dist;

static UiModel s_model;
static char s_clock_buf[12];
static char s_pace_buf[8];
static char s_dist_buf[8];
static char s_dur_buf[12];

static void prv_status_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);

  // Clock, left.
  clock_copy_time_string(s_clock_buf, sizeof s_clock_buf);
  graphics_context_set_text_color(ctx, GRAY);
  graphics_draw_text(ctx, s_clock_buf, fonts_get_system_font(FONT_KEY_GOTHIC_18),
                     GRect(0, -4, b.size.w - 44, 22), GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentLeft, NULL);

  // Bluetooth rune, right of center.
  GColor bt = s_model.bt_connected ? GRAY : COLOR_FALLBACK(GColorRed, GColorWhite);
  graphics_context_set_stroke_color(ctx, bt);
  graphics_context_set_stroke_width(ctx, 2);
  int bx = b.size.w - 38;
  GPoint p1 = GPoint(bx, 3), p2 = GPoint(bx + 8, 13), p3 = GPoint(bx + 4, 16),
         p4 = GPoint(bx + 4, 0), p5 = GPoint(bx + 8, 3), p6 = GPoint(bx, 13);
  graphics_draw_line(ctx, p1, p2);
  graphics_draw_line(ctx, p2, p3);
  graphics_draw_line(ctx, p3, p4);
  graphics_draw_line(ctx, p4, p5);
  graphics_draw_line(ctx, p5, p6);

  // GPS arrow, far right. Green = good, yellow = acquiring/lost.
  GColor arrow = (s_model.gps == GPS_GOOD) ? COLOR_FALLBACK(GColorGreen, GColorWhite)
                                           : COLOR_FALLBACK(GColorYellow, GColorLightGray);
  int ax = b.size.w - 18;
  GPathInfo info = {
      .num_points = 4,
      .points = (GPoint[]){{0, 8}, {16, 0}, {9, 16}, {7, 10}},
  };
  GPath *path = gpath_create(&info);
  gpath_move_to(path, GPoint(ax, 0));
  graphics_context_set_fill_color(ctx, arrow);
  gpath_draw_filled(ctx, path);
  gpath_destroy(path);
}

static void prv_divider_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, DIM_GRAY);
  graphics_fill_rect(ctx, GRect(0, 0, b.size.w, 1), 0, GCornerNone);
}

static TextLayer *prv_make_text(Layer *root, GRect frame, GFont font, GColor color,
                                GTextAlignment align) {
  TextLayer *tl = text_layer_create(frame);
  text_layer_set_background_color(tl, GColorClear);
  text_layer_set_text_color(tl, color);
  text_layer_set_font(tl, font);
  text_layer_set_text_alignment(tl, align);
  layer_add_child(root, text_layer_get_layer(tl));
  return tl;
}

void ui_create(Window *window) {
  s_window = window;
  window_set_background_color(window, GColorBlack);
  Layer *root = window_get_root_layer(window);
  GRect b = layer_get_bounds(root);
  const int w = b.size.w;
  const int h = b.size.h;

  s_font_pace = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_66));
  s_font_dist = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_50));

  s_status_layer = layer_create(GRect(8, 4, w - 16, 20));
  layer_set_update_proc(s_status_layer, prv_status_update_proc);
  layer_add_child(root, s_status_layer);

  s_pace_value =
      prv_make_text(root, GRect(0, 28, w - 52, 74), s_font_pace, GColorWhite, GTextAlignmentRight);
  s_pace_unit = prv_make_text(root, GRect(w - 46, 68, 44, 22),
                              fonts_get_system_font(FONT_KEY_GOTHIC_18), GRAY, GTextAlignmentLeft);
  text_layer_set_text(s_pace_unit, "/MI");

  s_band = prv_make_text(root, GRect(0, h - 118, w, 24), fonts_get_system_font(FONT_KEY_GOTHIC_14),
                         GColorWhite, GTextAlignmentCenter);

  s_divider = layer_create(GRect(8, h - 90, w - 16, 1));
  layer_set_update_proc(s_divider, prv_divider_update_proc);
  layer_add_child(root, s_divider);

  s_dist_value = prv_make_text(root, GRect(0, h - 86, w - 52, 54), s_font_dist, GColorWhite,
                               GTextAlignmentRight);
  s_dist_unit = prv_make_text(root, GRect(w - 46, h - 58, 40, 24),
                              fonts_get_system_font(FONT_KEY_GOTHIC_18), GRAY, GTextAlignmentLeft);
  text_layer_set_text(s_dist_unit, "MI");

  s_duration =
      prv_make_text(root, GRect(0, h - 30, w, 28), fonts_get_system_font(FONT_KEY_GOTHIC_24),
                    GColorWhite, GTextAlignmentCenter);
}

// The band is one reusable strip: inverted PAUSED banner, or a plain hint line.
static void prv_set_band(const char *text, GColor bg, GColor fg, const char *font_key) {
  text_layer_set_background_color(s_band, bg);
  text_layer_set_text_color(s_band, fg);
  text_layer_set_font(s_band, fonts_get_system_font(font_key));
  text_layer_set_text(s_band, text);
}

void ui_update(const UiModel *model) {
  s_model = *model;

  // Pace only means anything while running; idle and paused render "--:--".
  uint16_t shown_pace = (model->state == RUN_STATE_RUNNING) ? model->pace_spm : 0;
  fmt_pace(shown_pace, s_pace_buf, sizeof s_pace_buf);
  text_layer_set_text(s_pace_value, s_pace_buf);
  bool have_pace = shown_pace != 0;
  text_layer_set_text_color(s_pace_value, have_pace ? GColorWhite : GRAY);

  fmt_tenths_mi(meters_to_tenths_mi(model->distance_m), s_dist_buf, sizeof s_dist_buf);
  text_layer_set_text(s_dist_value, s_dist_buf);
  GColor value_color = (model->state == RUN_STATE_IDLE) ? GRAY : GColorWhite;
  text_layer_set_text_color(s_dist_value, value_color);

  fmt_duration_hms(model->duration_s, s_dur_buf, sizeof s_dur_buf);
  text_layer_set_text(s_duration, s_dur_buf);
  text_layer_set_text_color(s_duration, value_color);

  if (model->show_exit_hint) {
    prv_set_band("PRESS BACK AGAIN TO EXIT", GColorClear, GColorWhite, FONT_KEY_GOTHIC_14);
  } else if (model->state == RUN_STATE_PAUSED) {
    prv_set_band("PAUSED", GColorWhite, GColorBlack, FONT_KEY_GOTHIC_18_BOLD);
  } else if (model->state == RUN_STATE_IDLE) {
    prv_set_band("PRESS SELECT TO START", GColorClear, GColorWhite, FONT_KEY_GOTHIC_14);
  } else if (model->state == RUN_STATE_RUNNING && model->gps == GPS_LOST) {
    prv_set_band("GPS SIGNAL LOST", GColorClear, COLOR_FALLBACK(GColorYellow, GColorWhite),
                 FONT_KEY_GOTHIC_14);
  } else {
    prv_set_band("", GColorClear, GColorWhite, FONT_KEY_GOTHIC_14);
  }

  layer_mark_dirty(s_status_layer);
}

void ui_destroy(void) {
  layer_destroy(s_status_layer);
  layer_destroy(s_divider);
  text_layer_destroy(s_pace_value);
  text_layer_destroy(s_pace_unit);
  text_layer_destroy(s_band);
  text_layer_destroy(s_dist_value);
  text_layer_destroy(s_dist_unit);
  text_layer_destroy(s_duration);
  fonts_unload_custom_font(s_font_pace);
  fonts_unload_custom_font(s_font_dist);
}
