#if defined(PBL_RECT)

#include <pebble.h>
#include "ui.h"

GRect background_layer_bounds(Layer *window_layer) {
  //This is where the fun stuff of moving the background layer during
  //UnobstructedArea changes happens
  GRect unobstructed_bounds = layer_get_unobstructed_bounds(window_layer);
  GRect fullscreen = layer_get_bounds(window_layer);
  //I'd like to shift the center of the background up by half a
  //pixel for every full pixel the screen shrinks by. I do this
  //by adjusting the Y origin of the BitmapLayer, and shrink its
  //height so we're not drawing to obstructed pixels.
  int diff = fullscreen.size.h - unobstructed_bounds.size.h;
  GRect adjusted_bounds = GRect(0, -(diff / 2), fullscreen.size.w, fullscreen.size.h - (diff / 2));
  return adjusted_bounds;
}

GRect time_layer_bounds(Layer *window_layer) {
  GRect bounds = layer_get_unobstructed_bounds(window_layer);
  return GRect(0,  bounds.size.h - TIME_LAYER_HEIGHT - 12, bounds.size.w, TIME_LAYER_HEIGHT);
}

/* ---------- Status Indicators ---------- */
GRect hud_bluetooth_bounds() { return GRect(8, 11, 37, 24); }
GRect hud_battery_bounds() { return GRect(53, 11, 38, 24); }
GRect hud_quiet_bounds() { return GRect(99, 11, 37, 24); }

#endif