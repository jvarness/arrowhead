#pragma once
#include <pebble.h>

static const GPathInfo TRIANGLE_PATH = {
  .num_points = 3,
  .points = (GPoint[]) {{0,-10},{10,10},{-10,10}}
};

static const GPathInfo ARROWHEAD_PATH = {
  .num_points = 4,
  .points = (GPoint[]) {{0,-10},{10,10},{0,5},{-10,10}}
};

static const GPathInfo BATTERY_IND = {
  .num_points = 4,
  .points = (GPoint[]) {{0,-15}, {10,0}, {0,15}, {-10,0}}
};
