#pragma once
#include <pebble.h>

static const GPathInfo TRIANGLE_PATH = {
  .num_points = 3,
  .points = (GPoint[]) {{0,0},{10,20},{20,0}}
};

static const GPathInfo ARROWHEAD_PATH = {
  .num_points = 4,
  .points = (GPoint[]) {{0,0},{10,20},{20,0},{10,5}}
};

static const GPathInfo BATTERY_IND = {
  .num_points = 4,
  .points = (GPoint[]) {{10,0}, {20,15}, {10,30}, {0,15}}
};