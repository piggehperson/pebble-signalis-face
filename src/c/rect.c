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

GRect animation_layer_bounds(Layer *window_layer) {
  //Return a frame that covers specifically only the pixels of Elster's eye that change
  //This gets real fun when it has to move with UnobstructedArea
  GRect unobstructed_bounds = layer_get_unobstructed_bounds(window_layer);
  GRect fullscreen = layer_get_bounds(window_layer);
  //Here we only need to adjust the origin Y by the same amount
  //that the origin Y of the background layer shifts
  int diff = fullscreen.size.h - unobstructed_bounds.size.h;

  GPoint origin_raw = GPoint(0, 18);
  return GRect(origin_raw.x, origin_raw.y-(diff / 2), 144, 94);
  //Dear reader, the meager amount of math happening here is already
  //a lot for me to keep my brain wrapped around, so I'm not gonna
  //worry about what bottom pixels are occluded or not :)
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