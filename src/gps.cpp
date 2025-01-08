#include <Arduino.h>
#include <TinyGPS++.h>
#include <math.h>
#include <stdint.h>
#include "gps.h"
#include "graphics.h"

TinyGPSPlus gps;
extern HardwareSerial serialGPS;
extern Coord gps_coord;

#define DEG2RAD(a) ((a) / (180 / M_PI))
#define RAD2DEG(a) ((a) * (180 / M_PI))
#define EARTH_RADIUS 6378137
double lat2y(double lat) { return log(tan( DEG2RAD(lat) / 2 + M_PI/4 )) * EARTH_RADIUS; }
double lon2x(double lon) { return DEG2RAD(lon) * EARTH_RADIUS; }

/// @brief Get the current position from the GPS chip
/// @param serialGPS handler
/// @return projected coordinates in meters
void getPosition()
{
  while( serialGPS.available() > 0){
    gps.encode( serialGPS.read());
  }
  if( gps.location.isValid()){
    gps_coord.prev_lat = gps_coord.lat;
    gps_coord.prev_lng = gps_coord.lng;
    gps_coord.lat = gps.location.lat();
    gps_coord.lng = gps.location.lng();
    gps_coord.altitude = static_cast<int16_t>(gps.altitude.meters());
    gps_coord.direction = static_cast<int16_t>(gps.course.deg()); // degrees
    gps_coord.isValid = true;
    gps_coord.isUpdated = gps.location.isUpdated() && (gps_coord.prev_lat != gps_coord.lat || gps_coord.prev_lng != gps_coord.lng); //TODO: review
    gps_coord.satellites = static_cast<int16_t>(gps.satellites.value());
    gps_coord.hour = gps.time.hour();
    gps_coord.minute = gps.time.minute();
    gps_coord.second = gps.time.second();
    if( gps_coord.satellites >= 4){
      if( !gps_coord.fixAcquired){
        log_d("Fix Acquired! Sats: %i", gps_coord.satellites);
        log_d("isUpdated:%i", gps_coord.isUpdated);
        log_d("prev_lat:%f, lat:%f, prev_lng:%f, lng:%f", gps_coord.prev_lat, gps_coord.lat, gps_coord.prev_lng, gps_coord.lng);
        gps_coord.fixAcquired = true;
        gps_coord.isUpdated = true;
      }
    } else if( gps_coord.satellites <= 3){
      gps_coord.fixAcquired = false;
      log_d("No fix! Sats: %i", gps_coord.satellites);
    }
  } else {
    gps_coord.isValid = false;
  }
  // log_v("is valid/updates/fix: %i %i %i", gps_coord.isValid, gps_coord.isUpdated, gps_coord.fixAcquired);
  // log_v("sats lat lng: %i %f %f", gps_coord.satellites, gps_coord.lat, gps_coord.lng);
}

Point32 Coord::getPoint32()
{
  return Point32( lon2x( lng), lat2y( lat));
}


// Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
// Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
// Serial.print("ALT=");  Serial.println(gps.altitude.meters());
// Serial.print("day ");  Serial.println(gps.date.day());
// Serial.print("year ");  Serial.println(gps.date.year());
// Serial.print("hour ");  Serial.println(gps.time.hour());
// Serial.print("minute ");  Serial.println(gps.time.minute());
// Serial.print("satellites ");  Serial.println(gps.satellites.value());