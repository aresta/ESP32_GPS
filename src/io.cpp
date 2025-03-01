#include <Arduino.h>
#include "conf.h"
#include <WiFi.h>
#include "esp_bt.h"
#include "esp_cpu.h"
#include "colors.h"
#include "graphics.h"
#include "maps.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite *spr[MAX_ZOOM+1]; // zoom goes from 1 to MAX_ZOOM

extern bool select_btn_pressed;
extern bool up_btn_pressed;
extern bool down_btn_pressed;
extern bool left_btn_pressed;
extern bool right_btn_pressed;
extern bool menu_btn_short_pressed;
extern bool menu_btn_long_pressed;

/// @brief Initialize GPIO's, TFT display, sleep mode, power savings
void init_io()
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

  // wake up from low power mode
  gpio_wakeup_enable((gpio_num_t )MENU_BUTTON, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();

  // disable wifi & BT to save power
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();

  digitalWrite( SD_CS_PIN, HIGH); // SD card chips select
  digitalWrite( TFT_CS, HIGH); // TFT chip select
  ledcWrite(0, 128); // set display brigth: 0 to 255(100%)

  // display init
  tft.init();
  tft.setRotation(0);  // portrait
  tft.invertDisplay( true);
  tft.fillScreen( CYAN);
  tft.setTextColor(TFT_BLACK);

  // init sprites for each zoom level
  for(int i = 1; i <= MAX_ZOOM; i++){  // 1st pointer not used
    spr[i] = new TFT_eSprite(&tft);
    spr[i]->createSprite( SCREEN_BUFFER_WIDTH, SCREEN_BUFFER_HEIGHT);
    spr[i]->fillScreen(CYAN);
    spr[i]->setTextColor(TFT_BLACK);
    LOGD("FreeHeap: %i", esp_get_free_heap_size());
  }

  tft_msg("Initializing...");
  LOGI("Opening SD Card...");
  if(!init_sd_card()) {
    LOGE("Error: SD Card Mount Failed!");
    tft_msg("Error: SD Card Mount Failed!");
    while(true);
  }
}

/// @brief read buttons state with debounce and long press support
void check_buttons()
{
  const uint32_t debounceDelay = 150; // ms
  static uint32_t lastDebounceTime = 0;
  static uint32_t menu_button_press_start = 0; // ms 
  static uint16_t menu_button_long_delay = 800; // ms 

  if(( millis() - lastDebounceTime) < debounceDelay) return;
  lastDebounceTime = millis();

  if( digitalRead( SELECT_BUTTON) == LOW) select_btn_pressed = true;
  else if( digitalRead( UP_BUTTON) == LOW) up_btn_pressed = true;
  else if( digitalRead( DOWN_BUTTON) == LOW) down_btn_pressed = true;
  else if( digitalRead( LEFT_BUTTON) == LOW) left_btn_pressed = true;
  else if( digitalRead( RIGHT_BUTTON) == LOW) right_btn_pressed = true;
  else if( digitalRead( MENU_BUTTON) == LOW){
    if( menu_button_press_start == 0){
        menu_button_press_start = millis();
    }
    else if((millis() - menu_button_press_start) > menu_button_long_delay){
        menu_btn_long_pressed = true;
        menu_button_press_start = 0;
    }
  } 
  else if( digitalRead( MENU_BUTTON) == HIGH && menu_button_press_start > 0){ // button released
    menu_btn_short_pressed = true;
    menu_button_press_start = 0;
  }
}

void printFreeMem()
{
  LOGI("FreeHeap: %i", esp_get_free_heap_size());
  LOGI("Heap minimum_free_heap_size: %i", esp_get_minimum_free_heap_size());
  LOGI("Heap largest_free_block: %i", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
  LOGI("Task watermark: %i", uxTaskGetStackHighWaterMark(NULL));
}