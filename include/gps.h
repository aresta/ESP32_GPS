
#pragma once

#include <TinyGPS++.h>
#include <math.h>
#include <stdint.h>
#include <HardwareSerial.h>
#include "graphics.h"

struct Point32; // forward declaration

/// @brief Point in geografic (lat,lon) coordinates and other gps data
struct Coord {
  Point32 getPoint32();
  double lat = 0;
  double lng = 0;
  double prev_lat = 0;
  double prev_lng = 0;
  int16_t altitude = 0;
  int16_t prev_altitude = 0;
  int16_t direction = 0;
  int16_t satellites = 0;
  int16_t prev_satellites = 0;
  // uint8_t hour = 0;
  // uint8_t minute = 0;
  // uint8_t second = 0;
  bool isValid = false;
  bool isUpdated = false;
  bool fixAcquired = false;
};

void getPosition();

