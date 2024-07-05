#ifdef ARDUINO

#include <Arduino.h>
#include <TinyGPS++.h>
#include <math.h>
#include <stdint.h>
#include <graphics.h>

#include "gps.h"
#include "../conf.h"

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

/// @brief Get the current position from the GPS chip
/// @param serialGPS handler
/// @return projected coordinates in meters
void getPosition(HardwareSerial& serialGPS, Coord& coord)
{
    while (serialGPS.available() > 0)
    {
        if (gps.encode(serialGPS.read()))
        {
            coord.lat = gps.location.lat();
            coord.lng = gps.location.lng();

            coord.altitude = static_cast<int16_t>(gps.altitude.meters());
            coord.direction = static_cast<int16_t>(gps.course.deg()); // degrees
            coord.satellites = static_cast<int16_t>(gps.satellites.value());
            
            coord.isValid = gps.location.isValid();
            coord.isUpdated = gps.location.isUpdated();

            coord.hour = gps.time.hour();
            coord.minute = gps.time.minute();
            coord.second = gps.time.second();
        }
    }
}

void gpsInit()
{
    #ifdef ARDUINO_uPesy_WROVER
    #else   
        SerialGPS.begin(9600, SERIAL_8N1, GPS_TX, GPS_RX);
    #endif
}

void gpsGetPosition(Coord& coord)
{
    return getPosition(SerialGPS, coord);
}

// Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
// Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
// Serial.print("ALT=");  Serial.println(gps.altitude.meters());
// Serial.print("day ");  Serial.println(gps.date.day());
// Serial.print("year ");  Serial.println(gps.date.year());
// Serial.print("hour ");  Serial.println(gps.time.hour());
// Serial.print("minute ");  Serial.println(gps.time.minute());
// Serial.print("satellites ");  Serial.println(gps.satellites.value());

#endif