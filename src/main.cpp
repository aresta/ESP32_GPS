#include <Arduino.h>
#include "maps.h"
#include "gps.h"
#include "graphics.h"
#include "conf.h"
#include "env.h"
#include <WiFi.h>
#include "esp_bt.h"
#include "esp_cpu.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
HardwareSerial serialGPS(1);
ViewPort viewPort;
uint8_t zoom_level = PIXEL_SIZE_DEF; // zoom_level = 1 corresponds aprox to 1 meter / pixel
Coord gps_coord;
Point32 display_pos;
uint8_t mode = DEVMODE_NAV;

void printFreeMem()
{
  log_i("FreeHeap: %i", esp_get_free_heap_size());
  log_i("Heap minimum_free_heap_size: %i", esp_get_minimum_free_heap_size());
  log_i("Heap largest_free_block: %i", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
  log_i("Task watermark: %i", uxTaskGetStackHighWaterMark(NULL));
}

void setup()
{
  // Configure GPIO's
  pinMode( UP_BUTTON, INPUT_PULLUP);
  pinMode( DOWN_BUTTON, INPUT_PULLUP);
  pinMode( LEFT_BUTTON, INPUT_PULLUP);
  pinMode( RIGHT_BUTTON, INPUT_PULLUP);
  pinMode( SELECT_BUTTON, INPUT_PULLUP);
  pinMode( MENU_BUTTON, INPUT_PULLUP);
  pinMode( TFT_BLK_PIN, OUTPUT);
  ledcSetup(0, 5000, 8);  // Set up PWM for TFT BLK
  ledcAttachPin( TFT_BLK_PIN, 0); 
  ledcWrite(0, 0);  // switch off display
  
  Serial.begin(115200);
  serialGPS.begin( 9600, SERIAL_8N1, GPS_TX, GPS_RX);
  delay(50);
  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF); // disable wifi & BT
  btStop();

  digitalWrite( SD_CS_PIN, HIGH); // SD card chips select
  digitalWrite( TFT_CS, HIGH); // TFT chip select
  ledcWrite(0, 128); // set display 50%

  tft.init();
  tft.setRotation(0);  // portrait
  tft.invertDisplay( true);
  tft.fillScreen( CYAN);
  tft.setTextColor(TFT_BLACK);
  spr.createSprite( VIEWBUFFER_WIDTH, VIEWBUFFER_HEIGHT);
  spr.fillScreen( CYAN);
  spr.setTextColor(TFT_BLACK);

  tft_msg("Initializing...");
  log_i("Opening SD Card...");
  if(!init_sd_card()) {
    tft_msg("Error: SD Card Mount Failed!");
    while(true);
  }

  log_i("Waiting for satellites...");
  // serialGPS.println("$PMTK225,0*2B"); // set 'full on' mode
  serialGPS.println("$PMTK225,2,300,1000*1F"); // enable Periodic Mode (1-second interval, 300ms on-time)
  // serialGPS.println("$PMTK225,8*23"); // set 'Alwayslocate' mode

  // disable extra NMEA sentences. Only enables the $GPGGA sentence (position data)
  serialGPS.println("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29");

  // stats(viewPort, mmap);
  // printFreeMem();

  gpio_wakeup_enable((gpio_num_t )MENU_BUTTON, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();

  display_pos = Point32( INIT_POS);  // TODO: get last position from flash memory

  refresh_display();
}

bool select_btn_pressed = false;
bool up_btn_pressed = false;
bool down_btn_pressed = false;
bool left_btn_pressed = false;
bool right_btn_pressed = false;
bool menu_btn_pressed = false;

void check_buttons()
{
  static uint32_t lastDebounceTime = 0;
  const uint32_t debounceDelay = 200;

  if( digitalRead( SELECT_BUTTON) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    select_btn_pressed = true;
  }
  else if( digitalRead( UP_BUTTON) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    up_btn_pressed = true;
  }
  else if( digitalRead( DOWN_BUTTON) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    down_btn_pressed = true;
  }
  else if( digitalRead( LEFT_BUTTON) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    left_btn_pressed = true;
  }
  else if( digitalRead( RIGHT_BUTTON) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    right_btn_pressed = true;
  }
  else if( digitalRead( MENU_BUTTON) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    menu_btn_pressed = true;
  }
}

void loop()
{
  check_buttons();
  if( menu_btn_pressed) mode = DEVMODE_LOWPOW;
  switch( mode){
    case DEVMODE_NAV:
      getPosition();
      if( gps_coord.isValid && gps_coord.isUpdated){
        log_d("XXXX Fix?:%i, Sats:%i, isUpdated:%i", gps_coord.fixAcquired, gps_coord.satellites, gps_coord.isUpdated);
        display_pos = gps_coord.getPoint32();  // center display in gps coord
        refresh_display();
        gps_coord.isUpdated = false;
      }
      if( select_btn_pressed){
        mode = DEVMODE_MOVE;
      } else {
        // esp_sleep_enable_timer_wakeup( 100 * 1000); // sleep some ms
        // gpio_wakeup_enable((gpio_num_t )SELECT_BUTTON, GPIO_INTR_LOW_LEVEL);
        // esp_light_sleep_start();
        // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
        // gpio_wakeup_disable((gpio_num_t )SELECT_BUTTON);
      }
      break;
    
    case DEVMODE_MOVE:
      if( up_btn_pressed){    display_pos.y += 40*zoom_level; refresh_display(); }
      if( down_btn_pressed){  display_pos.y -= 40*zoom_level; refresh_display(); }
      if( left_btn_pressed){  display_pos.x -= 40*zoom_level; refresh_display(); }
      if( right_btn_pressed){ display_pos.x += 40*zoom_level; refresh_display(); }
      if( select_btn_pressed) mode = DEVMODE_ZOOM; 
      break;
    
    case DEVMODE_ZOOM:
      if( up_btn_pressed && zoom_level < MAX_ZOOM){ zoom_level += 1; refresh_display(); }
      if( down_btn_pressed && zoom_level > 1){      zoom_level -= 1; refresh_display(); }
      if( select_btn_pressed){ 
        mode = DEVMODE_NAV;
        if( gps_coord.isValid) display_pos = gps_coord.getPoint32();
      }
      break;

    case DEVMODE_LOWPOW: // TODO
      ledcWrite(0, 0); // set display off
      // sleep...
      // esp_sleep_enable_timer_wakeup( 20 * 1000000);
      
      // serialGPS.println("$PMTK161,0*28"); // enter standby Mode
      serialGPS.println("$PMTK225,8*23"); // always locate mode
      log_i("esp_light_sleep_start");
      delay(400); // debounce button
      esp_light_sleep_start();

      // wakeup_reason = esp_sleep_get_wakeup_cause();
      serialGPS.println("$PMTK225,0*2B"); // back to 'full on' mode
      // serialGPS.println("$PMTK225,8*23"); // set 'Alwayslocate' mode
      mode = DEVMODE_NAV;
      ledcWrite(0, 128); // set display 50%
      delay(400); // debounce button
      break;
  }

  if( select_btn_pressed) refresh_display();  // TODO: refresh only header
  up_btn_pressed = false;
  down_btn_pressed = false;
  left_btn_pressed = false;
  right_btn_pressed = false;
  select_btn_pressed = false;
  select_btn_pressed = false;
  menu_btn_pressed = false;
}
