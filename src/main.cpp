#include <Arduino.h>
#include "io.h"
#include "gps.h"
#include "conf.h"
#include "env.h"

HardwareSerial serialGPS(1);
uint8_t zoom_level = PIXEL_SIZE_DEF; /// @brief zoom_level = 1 corresponds aprox to 1 meter / pixel
Coord gps_coord; /// @brief Holds the current gps data
Point32 gps_pos_wc; /// @brief GPS position in world coordinates.
uint8_t mode = DEVMODE_NAV; /// @brief Current device mode

bool select_btn_pressed = false;
bool up_btn_pressed = false;
bool down_btn_pressed = false;
bool left_btn_pressed = false;
bool right_btn_pressed = false;
bool menu_btn_short_pressed = false;
bool menu_btn_long_pressed = false;

void setup()
{
  #if CORE_DEBUG_LEVEL > 0
    LOGI("Starting..."); 
    Serial.begin(115200);
  #endif
  serialGPS.begin( 9600, SERIAL_8N1, GPS_TX, GPS_RX);
  delay(50);

  init_io(); // initialize GPIO's, display, power saving, etc

  LOGI("Waiting for satellites...");
  // serialGPS.println("$PMTK225,0*2B"); // set 'full on' mode
  // serialGPS.println("$PMTK225,2,300,1000*1F"); // enable Periodic Mode (1-second interval, 300ms on-time)
  serialGPS.println("$PMTK225,8*23"); // set 'Alwayslocate' mode

  // disable extra NMEA sentences. Only enables the $GPGGA sentence (position data)
  serialGPS.println("$PMTK314,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29");
  gps_pos_wc = Point32( INIT_POS);  // TODO: get last position from flash memory
  refresh_display();
}

void loop()
{
  check_buttons();
  if( menu_btn_long_pressed) mode = DEVMODE_LOWPOW;
  switch( mode){
    case DEVMODE_NAV:
      getPosition();
      if( gps_coord.isValid && gps_coord.isUpdated){
        LOGD("XXXX Fix?:%i, Sats:%i, isUpdated:%i", gps_coord.fixAcquired, gps_coord.satellites, gps_coord.isUpdated);
        gps_pos_wc = gps_coord.getPoint32();  // center display in gps coord
        refresh_display();
        gps_coord.isUpdated = false;
      }
      if( select_btn_pressed){ mode = DEVMODE_MOVE; } 
      break;
    
    case DEVMODE_MOVE:
      if( up_btn_pressed){    gps_pos_wc.y += 40*zoom_level; refresh_display(); }
      if( down_btn_pressed){  gps_pos_wc.y -= 40*zoom_level; refresh_display(); }
      if( left_btn_pressed){  gps_pos_wc.x -= 40*zoom_level; refresh_display(); }
      if( right_btn_pressed){ gps_pos_wc.x += 40*zoom_level; refresh_display(); }
      if( select_btn_pressed) mode = DEVMODE_ZOOM; 
      break;
    
    case DEVMODE_ZOOM:
      if( up_btn_pressed && zoom_level < MAX_ZOOM){ zoom_level += 1; refresh_display(); }
      if( down_btn_pressed && zoom_level > 1){      zoom_level -= 1; refresh_display(); }
      if( select_btn_pressed){ 
        mode = DEVMODE_NAV;
        if( gps_coord.isValid) gps_pos_wc = gps_coord.getPoint32();
      }
      break;

    case DEVMODE_LOWPOW: // TODO
      ledcWrite(0, 0); // set display off
      // sleep...
      // esp_sleep_enable_timer_wakeup( 20 * 1000000);
      
      serialGPS.println("$PMTK161,0*28"); // enter standby Mode
      // serialGPS.println("$PMTK225,8*23"); // always locate mode
      LOGI("esp_light_sleep_start");
      delay(500); // debounce button
      esp_light_sleep_start();

      // wakeup_reason = esp_sleep_get_wakeup_cause();
      serialGPS.println("$PMTK225,0*2B"); // back to 'full on' mode
      // serialGPS.println("$PMTK225,8*23"); // set 'Alwayslocate' mode
      mode = DEVMODE_NAV;
      ledcWrite(0, 128); // set display 50%
      refresh_display(); // Check if needed
      break;
  }

  if( select_btn_pressed) refresh_display();  // TODO: refresh only header
  up_btn_pressed = false;
  down_btn_pressed = false;
  left_btn_pressed = false;
  right_btn_pressed = false;
  select_btn_pressed = false;
  select_btn_pressed = false;
  menu_btn_short_pressed = false;
  menu_btn_long_pressed = false;
}
