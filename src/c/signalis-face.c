#include <pebble.h>
#include <pebble-effect-layer/pebble-effect-layer.h>
#include "ui.h"

/* ---------- Static Vars ---------- */
static Window *s_window;
static TextLayer *s_layer_time;
static GFont s_font_time;
static BitmapLayer *s_layer_background;
static GBitmap *s_bmp_eye;
static GBitmap *s_bmp_dead;
static GBitmapSequence *s_sequence_blink;
static GBitmap *s_bmp_animation;
#if defined(PBL_COLOR)
static EffectOffset s_shadow;
static EffectLayer *s_layer_shadow;
#endif
static GBitmap *s_hud_bluetooth;
static GBitmap *s_hud_battery;
static GBitmap *s_hud_quiet;
static BitmapLayer *s_layer_hud_bluetooth;
static BitmapLayer *s_layer_hud_battery;
static BitmapLayer *s_layer_hud_quiet;

/* ---------- Convenience ----------*/
static bool prv_is_batt_low() {
  return battery_state_service_peek().charge_percent <= LOW_BATT_THRESHOLD;
}

/* ---------- UI ---------- */
static void prv_update_battery();
static void prv_update_background();

static void timer_handler(void *context) {
  uint32_t next_delay;

  // Advance to the next APNG frame, and get the delay for this frame
  if(gbitmap_sequence_update_bitmap_next_frame(s_sequence_blink, s_bmp_animation, &next_delay)) {
    // Set the new frame into the BitmapLayer
    APP_LOG(APP_LOG_LEVEL_INFO, "Rendering sequence frame");
    bitmap_layer_set_bitmap(s_layer_background, s_bmp_animation);
    layer_mark_dirty(bitmap_layer_get_layer(s_layer_background));

    // Timer for that frame's delay
    app_timer_register(next_delay, timer_handler, NULL);
  } else {
    // Destroy animation bits
    APP_LOG(APP_LOG_LEVEL_INFO, "Sequence final frame");
    if (s_bmp_animation) {
      gbitmap_destroy(s_bmp_animation);
      s_bmp_animation = NULL;
    }
    // Set static background
    prv_update_background();
  }
}

static void prv_play_blink_animation() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Playing blink");
  if (prv_is_batt_low()) { return; } //Skip playing animation if battery is low
  
  //Free old data
  if (s_bmp_animation) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Destroying gbitmap");
    gbitmap_destroy(s_bmp_animation);
    s_bmp_animation = NULL;
  }
  if(s_sequence_blink) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Destroying gbitmapsequence");
    gbitmap_sequence_destroy(s_sequence_blink);
    s_sequence_blink = NULL;
  }
  
  // Create sequence
  APP_LOG(APP_LOG_LEVEL_INFO, "Creating sequence");
  s_sequence_blink = gbitmap_sequence_create_with_resource(RESOURCE_ID_ANIM_BLINK);
  gbitmap_sequence_set_play_count(s_sequence_blink, 1);
  // Create blank GBitmap using APNG frame size
  APP_LOG(APP_LOG_LEVEL_INFO, "Creating blank gbitmap");
  GSize frame_size = gbitmap_sequence_get_bitmap_size(s_sequence_blink);
  s_bmp_animation = gbitmap_create_blank(frame_size, GBitmapFormat8Bit);

  uint32_t first_delay_ms = 500;

  // Schedule a timer to advance the first frame
  app_timer_register(first_delay_ms, timer_handler, NULL);
}

static void prv_update_background() {
  if (prv_is_batt_low()) {
    if (!s_bmp_dead) {
      s_bmp_dead = gbitmap_create_with_resource(RESOURCE_ID_BG_DEAD);
    }
    bitmap_layer_set_bitmap(s_layer_background, s_bmp_dead);
  } else {
    if (!s_bmp_eye) {
      s_bmp_eye = gbitmap_create_with_resource(RESOURCE_ID_BG_EYE);
    }
    bitmap_layer_set_bitmap(s_layer_background, s_bmp_eye);
  }
  layer_mark_dirty(bitmap_layer_get_layer(s_layer_background));
}

static void prv_update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H.%M0" : "%I.%M0", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_layer_time, s_buffer);
}

static void prv_update_bluetooth() {
  //Peek at current connection state
  layer_set_hidden(bitmap_layer_get_layer(s_layer_hud_bluetooth), connection_service_peek_pebble_app_connection());
}

static void prv_update_battery() {
  //Check if battery is below threshold
  bool is_low = battery_state_service_peek().charge_percent <= LOW_BATT_THRESHOLD;
  layer_set_hidden(bitmap_layer_get_layer(s_layer_hud_battery), !is_low);
  prv_update_background();
}

static void prv_update_quiet() {
  //Check if quiet time is enabled
  layer_set_hidden(bitmap_layer_get_layer(s_layer_hud_quiet), !quiet_time_is_active());
}

static void prv_refresh_display() {
  //Catch-all function to redraw anything that may have changed state
  prv_update_time();
  prv_update_bluetooth();
  prv_update_battery();
  prv_update_quiet();
}

static void prv_initialize_layers(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  //Set up BitmapLayer
  s_layer_background = bitmap_layer_create(background_layer_bounds(window_layer));
  //Create and set appropriate background for battery level
  prv_update_background();
  layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_background));

  //Set up status indicator hud
  s_hud_bluetooth = gbitmap_create_with_resource(RESOURCE_ID_HUD_BLUETOOTH);
  s_hud_battery = gbitmap_create_with_resource(RESOURCE_ID_HUD_BATTERY);
  s_hud_quiet = gbitmap_create_with_resource(RESOURCE_ID_HUD_QUIET);
  s_layer_hud_bluetooth = bitmap_layer_create(hud_bluetooth_bounds());
  s_layer_hud_battery = bitmap_layer_create(hud_battery_bounds());
  s_layer_hud_quiet = bitmap_layer_create(hud_quiet_bounds());
  bitmap_layer_set_bitmap(s_layer_hud_bluetooth, s_hud_bluetooth);
  bitmap_layer_set_bitmap(s_layer_hud_battery, s_hud_battery);
  bitmap_layer_set_bitmap(s_layer_hud_quiet, s_hud_quiet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_hud_bluetooth));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_hud_battery));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_hud_quiet));

  // Set up TextLayer
  s_font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DIN_MITTELSCHRIFT_48));
  s_layer_time = text_layer_create(time_layer_bounds(window_layer));
  text_layer_set_font(s_layer_time, s_font_time);
  text_layer_set_text_color(s_layer_time, CLOCK_COLOR);
  text_layer_set_background_color(s_layer_time, GColorClear);
  text_layer_set_text_alignment(s_layer_time, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_layer_time));
  
  #if defined(PBL_COLOR) //Set up EffectLayer for text shadow on color watches
  s_shadow = (EffectOffset){
    .orig_color = CLOCK_COLOR,
    .offset_color = SHADOW_COLOR,
    .offset_y = BLUR_RADIUS,
    .offset_x = 0,
  };
  s_layer_shadow = effect_layer_create(time_layer_bounds(window_layer));
  effect_layer_add_effect(s_layer_shadow, effect_shadow, &s_shadow);
  layer_add_child(window_layer, effect_layer_get_layer(s_layer_shadow));
  #endif
  // Text legibility is handled differently on BW, so we only draw the shadow on color
  
  //Initialize time and status displays
  prv_refresh_display();
}

/* ---------- Event Service Handlers ---------- */
static void prv_time_tick_handler(struct tm *tick_time, TimeUnits changed) {
  prv_update_time();
}

static void prv_battery_callback(BatteryChargeState state) {
  prv_update_battery();
}

static void prv_bluetooth_callback(bool connected) {
  prv_update_bluetooth();
  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

void prv_did_focus_callback(bool did_focus) {
  APP_LOG(APP_LOG_LEVEL_INFO, "%s focus", did_focus ? "Gained" : "Lost");
  if (did_focus) {
    prv_play_blink_animation();
  }
}

#if defined(PBL_RECT) //Handle timeline peek animation on non-round watches
static void prv_update_unobstructed_area(AnimationProgress progress, void *context) {
  layer_set_frame(bitmap_layer_get_layer(s_layer_background), background_layer_bounds(window_get_root_layer(s_window)));
  layer_set_frame(text_layer_get_layer(s_layer_time),time_layer_bounds(window_get_root_layer(s_window)));
  #if defined(PBL_COLOR)
  layer_set_frame(effect_layer_get_layer(s_layer_shadow), time_layer_bounds(window_get_root_layer(s_window)));
  #endif
}
#endif

/* ---------- App Life Cycle ---------- */
static void prv_window_load(Window *window) {
  prv_initialize_layers(window);
  
  #if defined(PBL_RECT) //Handle timeline peek animation on non-round watches
  UnobstructedAreaHandlers handlers = {
    .change = prv_update_unobstructed_area
  };
  unobstructed_area_service_subscribe(handlers, NULL);
  #endif
  //Set up other event handlers
  tick_timer_service_subscribe(MINUTE_UNIT, prv_time_tick_handler);
  battery_state_service_subscribe(prv_battery_callback);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = prv_bluetooth_callback
  });
  app_focus_service_subscribe_handlers((AppFocusHandlers) {
    .did_focus = prv_did_focus_callback
  });
}

static void prv_window_unload(Window *window) {
  //Destroy layers
  bitmap_layer_destroy(s_layer_background);
  gbitmap_destroy(s_bmp_eye);
  gbitmap_destroy(s_bmp_dead);
  gbitmap_destroy(s_bmp_animation);
  gbitmap_sequence_destroy(s_sequence_blink);
  bitmap_layer_destroy(s_layer_hud_bluetooth);
  bitmap_layer_destroy(s_layer_hud_battery);
  bitmap_layer_destroy(s_layer_hud_quiet);
  gbitmap_destroy(s_hud_bluetooth);
  gbitmap_destroy(s_hud_battery);
  gbitmap_destroy(s_hud_quiet);
  text_layer_destroy(s_layer_time);
  fonts_unload_custom_font(s_font_time);
  #if defined(PBL_COLOR)
  effect_layer_destroy(s_layer_shadow);
  #endif
  
  //Unsubscribe from event services
  #if defined(PBL_RECT)
  unobstructed_area_service_unsubscribe();
  #endif
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  app_focus_service_unsubscribe();
}

static void prv_init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
