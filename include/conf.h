#pragma once

// #define CORE_DEBUG_LEVEL 5 
#define SCREEN_WIDTH 240  // tft.width()
#define SCREEN_HEIGHT 320   // tft.height()
#define PIXEL_SIZE_DEF 2  // in meters
#define MAX_ZOOM 7

// Buttons GPIO nums
#define UP_BUTTON 03
#define DOWN_BUTTON 04
#define LEFT_BUTTON 15
#define RIGHT_BUTTON 16
#define SELECT_BUTTON 07
#define MENU_BUTTON 5
#define TFT_BLK_PIN 21
#define TFT_CS 10

#define SD_MISO_PIN 13
#define SD_CS_PIN 14

#define GPS_RX 17   // to ESP32 TX
#define GPS_TX 18   // to ESP32 RX
// #define GPS_CE 14

enum DEV_STATES { DEVMODE_NAV, DEVMODE_MOVE, DEVMODE_ZOOM, DEVMODE_MENU, DEVMODE_LOWPOW };

