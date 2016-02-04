#include <pebble.h>
#include "shapes.h"

static Window *window;
static Layer *triangle;
static Layer *arrowhead;
static Layer *battery_ind;
static GPath *arrowhead_path;
static GPath *triangle_path;
static GPath *battery_ind_path;
static int battery_level;
static bool is_charging;
static uint8_t time_hour;
static uint8_t time_minute;

static void update_triangle(Layer *triangle_layer, GContext *ctx) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "Hours: %d, Angle: %d", time_hour, time_hour * 30);
  int32_t angle = (TRIG_MAX_ANGLE / 360) * (time_hour * 30);
  GRect bounds = layer_get_bounds(triangle_layer);
  gpath_rotate_to(triangle_path, angle);
  gpath_move_to(triangle_path, GPoint((bounds.size.w / 2) + (50 * sin_lookup(angle) / TRIG_MAX_RATIO), (bounds.size.h / 2) + (50 * -cos_lookup(angle) / TRIG_MAX_RATIO)));
  graphics_context_set_fill_color(ctx, GColorDarkCandyAppleRed);
  gpath_draw_filled(ctx, triangle_path);
}

static void update_arrowhead(Layer *arrowhead_layer, GContext *ctx) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "Minutes: %d, Angle: %d", time_minute, time_minute * 6);
  int32_t angle = (TRIG_MAX_ANGLE / 360) * (time_minute * 6);
  GRect bounds = layer_get_bounds(arrowhead_layer);
  gpath_rotate_to(arrowhead_path, angle);
  gpath_move_to(arrowhead_path, GPoint((bounds.size.w / 2) + (50 * sin_lookup(angle) / TRIG_MAX_RATIO), (bounds.size.h / 2) + (50 * -cos_lookup(angle) / TRIG_MAX_RATIO)));
  graphics_context_set_fill_color(ctx, GColorCadetBlue);
  gpath_draw_filled(ctx, arrowhead_path);
}

static void update_battery_ind(Layer *triangle_layer, GContext *ctx) {
  GColor color = GColorWhite;
  
  if(battery_level <= 100 && battery_level >= 50) {
    color = GColorCyan;
  }
  else if (battery_level < 50 && battery_level > 20) {
    color = GColorChromeYellow;
  }
  else if (battery_level <= 20) {
    color = GColorRed;
  }
  graphics_context_set_fill_color(ctx, color);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 3);
  gpath_draw_filled(ctx, battery_ind_path);
  gpath_draw_outline(ctx, battery_ind_path);
}

static void update_battery(BatteryChargeState state) {
  battery_level = state.charge_percent;
  is_charging = state.is_charging;
  layer_mark_dirty(battery_ind);
}

static void update_time(struct tm *time) {
  time_hour = time->tm_hour;
  time_minute = time->tm_min;
  layer_mark_dirty(triangle);
  layer_mark_dirty(arrowhead);
}

static void arw_minute_tick(struct tm *time, TimeUnits units_changed) {
  update_time(time);
}

static void load_window(Window *win) {
  Layer *layer = window_get_root_layer(window);
  GRect rect = layer_get_bounds(layer);
  triangle = layer_create(rect);
  arrowhead = layer_create(rect);
  battery_ind = layer_create(GRect(rect.size.w / 2 - 13, rect.size.h / 2 - 18, 26, 36));
  
  layer_set_update_proc(triangle, update_triangle);
  layer_set_update_proc(arrowhead, update_arrowhead);
  layer_set_update_proc(battery_ind, update_battery_ind);
  
  layer_add_child(layer, triangle);
  layer_add_child(layer, arrowhead);
  layer_add_child(layer, battery_ind);
}

static void unload_window(Window *win) {
  layer_destroy(triangle);
  layer_destroy(arrowhead);
  layer_destroy(battery_ind);
  
  gpath_destroy(triangle_path);
  gpath_destroy(arrowhead_path);
  gpath_destroy(battery_ind_path);
}

static void setup_arrowhead_minute() {
  triangle_path = gpath_create(&TRIANGLE_PATH);
  gpath_move_to(triangle_path, GPoint(20,20));
}

static void setup_arrowhead_hour() {
  arrowhead_path = gpath_create(&ARROWHEAD_PATH);
}

static void setup_battery_ind() {
  battery_ind_path = gpath_create(&BATTERY_IND);
  gpath_move_to(battery_ind_path, GPoint(13, 18));
}

static void arw_init() {
  window = window_create();
  
  window_set_window_handlers(window, (WindowHandlers) {
    .load = load_window,
    .unload = unload_window
  });
  
  window_set_background_color(window, GColorWhite);
  
  window_stack_push(window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, arw_minute_tick);
  battery_state_service_subscribe(update_battery);
  
  setup_arrowhead_minute();
  setup_arrowhead_hour();
  setup_battery_ind();
  
  time_t start = time(NULL);
  update_time(localtime(&start));
  update_battery(battery_state_service_peek());
}

static void arw_deinit() {
  window_destroy(window);
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
}

int main(void) {
  arw_init();
  app_event_loop();
  arw_deinit();
}
