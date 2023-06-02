
#ifndef gps_h_
#define gps_h_
#include <TinyGPS++.h>
#include <math.h>
#include <stdint.h>
#include <HardwareSerial.h>

struct Coord;
Coord getPosition(HardwareSerial& serialGPS);

#endif