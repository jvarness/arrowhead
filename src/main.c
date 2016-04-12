#include <pebble.h>
#include "shapes.h"

#define KEY_BG_COLOR 0
#define KEY_HOUR_COLOR 1
#define KEY_MIN_COLOR 2

static Window *window;
static Layer *triangle;
static Layer *arrowhead;
static Layer *battery_ind;
static TextLayer *dials;
static TextLayer *steps_text;
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
  graphics_context_set_fill_color(ctx, GColorFromHEX(persist_read_int(KEY_HOUR_COLOR)));
  gpath_draw_filled(ctx, triangle_path);
}

static void update_arrowhead(Layer *arrowhead_layer, GContext *ctx) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "Minutes: %d, Angle: %d", time_minute, time_minute * 6);
  int32_t angle = (TRIG_MAX_ANGLE / 360) * (time_minute * 6);
  GRect bounds = layer_get_bounds(arrowhead_layer);
  gpath_rotate_to(arrowhead_path, angle);
  gpath_move_to(arrowhead_path, GPoint((bounds.size.w / 2) + (50 * sin_lookup(angle) / TRIG_MAX_RATIO), (bounds.size.h / 2) + (50 * -cos_lookup(angle) / TRIG_MAX_RATIO)));
  graphics_context_set_fill_color(ctx, GColorFromHEX(persist_read_int(KEY_MIN_COLOR)));
  gpath_draw_filled(ctx, arrowhead_path);
}

static void update_battery_ind(Layer *triangle_layer, GContext *ctx) {
  GColor color = GColorWhite;
  
  if(battery_level <= 100 && battery_level > 50) {
    color = GColorCyan;
  }
  else if (battery_level <= 50 && battery_level > 20) {
    color = GColorYellow;
  }
  else if (battery_level <= 20) {
    color = GColorRed;
  }
  graphics_context_set_fill_color(ctx, color);
  graphics_context_set_stroke_color(ctx, GColorFromHEX(persist_read_int(KEY_MIN_COLOR)));
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

static void update_health() {
  HealthValue step_metric = health_service_sum_today(HealthMetricStepCount);
  int step_count = step_metric;
    
  static char buffer[] = "0000000";
  snprintf(buffer, 7, "%d", step_count);
  
  text_layer_set_text(steps_text, buffer);
}

static void health_trigger(HealthEventType eventType, void * context) {
  if(eventType == HealthEventMovementUpdate || eventType == HealthEventMovementUpdate) {
    update_health();
  }
}

static void arw_minute_tick(struct tm *time, TimeUnits units_changed) {
  update_time(time);
}

static void arw_inbox_handler(DictionaryIterator *iterator, void *context) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "In handler");
  Tuple *background_color_t = dict_find(iterator, KEY_BG_COLOR);
  Tuple *hour_color_t = dict_find(iterator, KEY_HOUR_COLOR);
  Tuple *minute_color_t = dict_find(iterator, KEY_MIN_COLOR);
  
  if (background_color_t) {
    int color = background_color_t->value->int32;
    
    persist_write_int(KEY_BG_COLOR, color);
    
    window_set_background_color(window, GColorFromHEX(persist_read_int(KEY_BG_COLOR)));
  }
  
  if (hour_color_t) {
    int color = hour_color_t->value->int32;
    
    persist_write_int(KEY_HOUR_COLOR, color);
    
    layer_mark_dirty(triangle);
    text_layer_set_text_color(dials, GColorFromHEX(persist_read_int(KEY_HOUR_COLOR)));
  }
  
  if (minute_color_t) {
    int color = minute_color_t->value->int32;
    
    persist_write_int(KEY_MIN_COLOR, color);
    
    layer_mark_dirty(arrowhead);
    layer_mark_dirty(battery_ind);
    text_layer_set_text_color(steps_text, GColorFromHEX(persist_read_int(KEY_MIN_COLOR)));
  }
}

static void load_window(Window *win) {  
  Layer *layer = window_get_root_layer(window);
  GRect rect = layer_get_bounds(layer);
  triangle = layer_create(rect);
  arrowhead = layer_create(rect);
  battery_ind = layer_create(GRect(rect.size.w / 2 - 13, rect.size.h / 2 - 18, 26, 36));
  
  dials = text_layer_create(GRect((rect.size.w / 2) - 10, 0, 25, 25));
  text_layer_set_background_color(dials, GColorClear);
  text_layer_set_text_color(dials, GColorFromHEX(persist_read_int(KEY_HOUR_COLOR)));
  text_layer_set_font(dials, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  text_layer_set_text_alignment(dials, GTextAlignmentCenter);
  text_layer_set_text(dials, "12");
  
  steps_text = text_layer_create(GRect((rect.size.w / 2) - 50, rect.size.h - 25, 100, 25));
  text_layer_set_background_color(steps_text, GColorClear);
  text_layer_set_text_color(steps_text, GColorFromHEX(persist_read_int(KEY_MIN_COLOR)));
  text_layer_set_font(steps_text, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  text_layer_set_text_alignment(steps_text, GTextAlignmentCenter);
  
  layer_set_update_proc(triangle, update_triangle);
  layer_set_update_proc(arrowhead, update_arrowhead);
  layer_set_update_proc(battery_ind, update_battery_ind);
  
  layer_add_child(layer, text_layer_get_layer(dials));
  layer_add_child(layer, text_layer_get_layer(steps_text));
  layer_add_child(layer, triangle);
  layer_add_child(layer, arrowhead);
  layer_add_child(layer, battery_ind);
  
  update_health();
}

static void unload_window(Window *win) {
  layer_destroy(triangle);
  layer_destroy(arrowhead);
  layer_destroy(battery_ind);
  
  text_layer_destroy(dials);
  text_layer_destroy(steps_text);
  
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

static void check_defaults() {
  if(!persist_exists(KEY_BG_COLOR)){
    persist_write_int(KEY_BG_COLOR, 0xFFFFFF);
  }
  if(!persist_exists(KEY_MIN_COLOR)){
    persist_write_int(KEY_MIN_COLOR, 0x0000FF);
  }
  if(!persist_exists(KEY_HOUR_COLOR)){
    persist_write_int(KEY_HOUR_COLOR, 0xAA0000);
  }
}

static void arw_init() {  
  check_defaults();
  
  window = window_create();
  
  window_set_window_handlers(window, (WindowHandlers) {
    .load = load_window,
    .unload = unload_window
  });
  
  window_set_background_color(window, GColorFromHEX(persist_read_int(KEY_BG_COLOR)));
  
  window_stack_push(window, true);
  
  app_message_register_inbox_received(arw_inbox_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  tick_timer_service_subscribe(MINUTE_UNIT, arw_minute_tick);
  battery_state_service_subscribe(update_battery);
  health_service_events_subscribe(health_trigger, NULL);
  
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
  health_service_events_unsubscribe();
  app_message_deregister_callbacks();
}

int main(void) {
  arw_init();
  app_event_loop();
  arw_deinit();
}
