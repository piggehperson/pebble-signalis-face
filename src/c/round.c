#if defined(PBL_ROUND)

#include <pebble.h>
#include "ui.h"

GRect background_layer_bounds(Layer *window_layer) {
  return layer_get_bounds(window_layer);
  //This function exists to handle UnobstructedArea on rectangular
  //devices. Since chalk doesn't use that feature, we don't need to
  // do anything here.
}

GRect time_layer_bounds(Layer *window_layer) {
  GRect bounds = layer_get_bounds(window_layer);
  return GRect(0,  bounds.size.h - TIME_LAYER_HEIGHT - 36, bounds.size.w, TIME_LAYER_HEIGHT);
}

/* ---------- Status Indicators ---------- */
GRect hud_bluetooth_bounds() { return GRect(26, 33, 37, 24); }
GRect hud_battery_bounds() { return GRect(71, 8, 38, 24); }
GRect hud_quiet_bounds() { return GRect(117, 33, 37, 24); }

#endif