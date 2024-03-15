#include <pebble.h>

#define BLUR_RADIUS 1
#define TIME_LAYER_HEIGHT 48

#define CLOCK_COLOR PBL_IF_COLOR_ELSE(GColorRed, GColorWhite)
#define SHADOW_COLOR GColorWhite

#define LOW_BATT_THRESHOLD 10

GRect background_layer_bounds(Layer *window_layer);
GRect animation_layer_bounds(Layer *window_layer);
GRect time_layer_bounds(Layer *window_layer);

GRect hud_bluetooth_bounds();
GRect hud_battery_bounds();
GRect hud_quiet_bounds();