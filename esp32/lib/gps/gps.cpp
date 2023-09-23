#include <Arduino.h>
#include <TinyGPS++.h>
#include <math.h>
#include <stdint.h>
#include "gps.h"
#include "graphics.h"

TinyGPSPlus gps;

#define DEG2RAD(a)   ((a) / (180 / M_PI))
#define RAD2DEG(a)   ((a) * (180 / M_PI))
#define EARTH_RADIUS 6378137
double lat2y(double lat) { return log(tan( DEG2RAD(lat) / 2 + M_PI/4 )) * EARTH_RADIUS; }
double lon2x(double lon) { return          DEG2RAD(lon)                 * EARTH_RADIUS; }

/// @brief Get the current position from the GPS chip
/// @param serialGPS handler
/// @return projected coordinates in meters
Coord getPosition(HardwareSerial& serialGPS)
{
    Coord coord;
    while( serialGPS.available() > 0){
        if( gps.encode( serialGPS.read())){
            coord.lat = gps.location.lat();
            coord.lng = gps.location.lng();
            coord.altitude = static_cast<int16_t>(gps.altitude.meters());
            coord.direction = static_cast<int16_t>(gps.course.deg()); // degrees
            coord.isValid = gps.location.isValid();
            coord.isUpdated = gps.location.isUpdated();
            coord.satellites = static_cast<int16_t>(gps.satellites.value());
            coord.hour = gps.time.hour();
            coord.minute = gps.time.minute();
            coord.second = gps.time.second();
        }
    }    
    return coord;
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