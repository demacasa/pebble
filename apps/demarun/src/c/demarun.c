#include <pebble.h>

static Window *s_window;
static TextLayer *s_title_layer;

static void prv_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  window_set_background_color(window, GColorBlack);
  s_title_layer = text_layer_create(GRect(0, bounds.size.h / 2 - 14, bounds.size.w, 28));
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_text_color(s_title_layer, GColorWhite);
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_title_layer, "DemaRun");
  layer_add_child(root, text_layer_get_layer(s_title_layer));
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
}

int main(void) {
  s_window = window_create();
  window_set_window_handlers(
      s_window, (WindowHandlers){.load = prv_window_load, .unload = prv_window_unload});
  window_stack_push(s_window, true);
  app_event_loop();
  window_destroy(s_window);
  return 0;
}
