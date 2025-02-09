#include <Arduino.h>
#include "maps.h"
#include "gps.h"
#include "graphics.h"
#include "conf.h"
#include "env.h"

TFT_eSPI tft = TFT_eSPI();
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
  // pinMode( GPS_CE, OUTPUT);
  digitalWrite( TFT_BLK_PIN, LOW); // switch off display

  Serial.begin(115200);
  // delay(1000);
  // printFreeMem();
  serialGPS.begin( 9600, SERIAL_8N1, GPS_TX, GPS_RX);
  delay(50);

  digitalWrite( SD_CS_PIN, HIGH); // SD card chips select
  digitalWrite( TFT_CS, HIGH); // TFT chip select
  digitalWrite( TFT_BLK_PIN, HIGH);

  tft.init();
  tft.setRotation(0);  // portrait
  tft.invertDisplay( true);
  tft.fillScreen( CYAN);
  tft.setTextColor(TFT_BLACK);
  tft_msg("Initializing...");
  log_i("Initializing...");
  if(!init_sd_card()) {
    tft_msg("Error: SD Card Mount Failed!");
    while(true);
  }

  log_i("Waiting for satellites...");
  serialGPS.println("$PMTK225,0*2B"); // set 'full on' mode

  // stats(viewPort, mmap);
  // printFreeMem();

  // digitalWrite( GPS_CE, HIGH); // GPS low power mode disabled
  gpio_wakeup_enable( (gpio_num_t )MENU_BUTTON, GPIO_INTR_LOW_LEVEL);
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
  if( !gps_coord.fixAcquired){
    getPosition();
    if( gps_coord.isValid){
      display_pos = gps_coord.getPoint32();
      if( gps_coord.isUpdated){
        refresh_display();
      }
    } else{
      delay(100); // TODO
    }
  }

  check_buttons();
  if( menu_btn_pressed) mode = DEVMODE_LOWPOW;
  switch( mode){
    case DEVMODE_NAV:
      getPosition();
      if( gps_coord.isValid && gps_coord.isUpdated){
        display_pos = gps_coord.getPoint32();  // center display in gps coord
        refresh_display();
      }
      if( select_btn_pressed) mode = DEVMODE_MOVE;
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
      digitalWrite( TFT_BLK_PIN, LOW);
      // sleep...
      // esp_sleep_enable_timer_wakeup( 20 * 1000000);
      serialGPS.println("$PMTK161,0*28"); // enter standby Mode
      // serialGPS.println("$PMTK225,8*23"); // set 'Alwayslocate' mode
      log_i("esp_light_sleep_start");
      delay(400); // debounce button
      esp_light_sleep_start();

      // wakeup_reason = esp_sleep_get_wakeup_cause();
      serialGPS.println("$PMTK225,0*2B"); // back to 'full on' mode
      mode = DEVMODE_NAV;
      digitalWrite( TFT_BLK_PIN, HIGH);
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
