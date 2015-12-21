#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_watch_power_layer;
static TextLayer *s_bt_connection_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static bool s_bt_was_connected = true;

static void update_time(struct tm *tick_time){
  static char buffer[6];
  strftime(buffer,6,"%H:%M",tick_time);
  text_layer_set_text(s_time_layer,buffer);
}
static void update_date(struct tm *tick_time){
  static char buffer[80];
  strftime(buffer,80,"%A %B %e",tick_time);
  text_layer_set_text(s_date_layer,buffer);
}

static void battery_state_change_handler(BatteryChargeState state){
  static char buffer[5];
  snprintf(buffer, 5, "%u%%",state.charge_percent);
  text_layer_set_text(s_watch_power_layer,buffer);
  if (state.charge_percent < 30 && !state.is_charging){
    vibes_long_pulse();
  }
}

static void bt_connection_state_change_handler(bool bt_connected){
  if (s_bt_was_connected != bt_connected){ // So we only care if the state actually changed!
    vibes_double_pulse();
    if (bt_connected){
      text_layer_set_text(s_bt_connection_layer,"Online");
    }
    else {
      text_layer_set_text(s_bt_connection_layer,"Offline");
    }
    s_bt_was_connected = bt_connected;
  }
}

static void main_window_load(Window *window){
  
  // PICTURE LAYER
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TAILS_DARK);
  s_background_layer = bitmap_layer_create(GRect(0,60,144,108));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window),bitmap_layer_get_layer(s_background_layer));
  
  // TIME LAYER
  s_time_layer = text_layer_create(GRect(0, -10, 144, 50));
  text_layer_set_background_color(s_time_layer,GColorBlack);
  text_layer_set_text_color(s_time_layer,GColorWhite);
  text_layer_set_text(s_time_layer,"TIME!");
  text_layer_set_font(s_time_layer,fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  //text_layer_set_font(s_time_layer,fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_time_layer,GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_time_layer));
  
  // DATE LAYER
  s_date_layer = text_layer_create(GRect(0,40,144,20));
  text_layer_set_background_color(s_date_layer,GColorBlack);
  text_layer_set_text_color(s_date_layer,GColorWhite);
  text_layer_set_text(s_date_layer,"DATE!");
  text_layer_set_font(s_date_layer,fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_date_layer,GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_date_layer));
  
  // WATCH POWER LAYER
  s_watch_power_layer = text_layer_create(GRect(44,144,100,24));
  text_layer_set_background_color(s_watch_power_layer,GColorClear);
  text_layer_set_text_color(s_watch_power_layer,GColorWhite);
  text_layer_set_text(s_watch_power_layer,"PWR!");
  text_layer_set_font(s_watch_power_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_watch_power_layer,GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_watch_power_layer));
  battery_state_change_handler(battery_state_service_peek());
  
  // BT Connection layer
  s_bt_connection_layer = text_layer_create(GRect(0,144,80,24));
  text_layer_set_background_color(s_bt_connection_layer,GColorClear);
  text_layer_set_text_color(s_bt_connection_layer,GColorWhite);
  text_layer_set_text(s_bt_connection_layer,"Online");
  text_layer_set_font(s_bt_connection_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_bt_connection_layer,GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_bt_connection_layer));
  bluetooth_connection_service_subscribe(bt_connection_state_change_handler);
  bt_connection_state_change_handler(bluetooth_connection_service_peek());

  // Initialize time and date
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  update_time(tick_time);
  update_date(tick_time);

}
static void main_window_unload(Window *window){
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time(tick_time);
  if (units_changed & DAY_UNIT){ 
    update_date(tick_time);
  }
}

static void init(){
  s_main_window = window_create();
  //window_set_fullscreen(s_main_window,true);
  window_set_window_handlers(s_main_window,(WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  

  // Subscribe to events
  battery_state_service_subscribe(battery_state_change_handler);
  tick_timer_service_subscribe(MINUTE_UNIT,tick_handler);
  //tick_timer_service_subscribe(SECOND_UNIT,tick_handler);
  
  window_stack_push(s_main_window, true);
}

static void deinit(){
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_watch_power_layer);
  text_layer_destroy(s_bt_connection_layer);
  bitmap_layer_destroy(s_background_layer);
  window_destroy(s_main_window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}

