#pragma once

// #define CORE_DEBUG_LEVEL 5 
#define SCREEN_WIDTH 240  
#define SCREEN_HEIGHT 320   
#define SCREEN_BUFFER_WIDTH (SCREEN_WIDTH * 1)
#define SCREEN_BUFFER_HEIGHT (SCREEN_HEIGHT * 1)
#define PIXEL_SIZE_DEF 2  // in meters
#define MAX_ZOOM 7


// Buttons GPIO nums
#define UP_BUTTON 16
#define DOWN_BUTTON 15
#define LEFT_BUTTON 03
#define RIGHT_BUTTON 04
#define SELECT_BUTTON 07
#define MENU_BUTTON 5
#define TFT_BLK_PIN 21
#define TFT_CS 10

#define SD_MISO_PIN 13
#define SD_CS_PIN 14

#define GPS_RX 17   // to ESP32 TX
#define GPS_TX 18   // to ESP32 RX

enum DEV_STATES { DEVMODE_NAV, DEVMODE_MOVE, DEVMODE_ZOOM, DEVMODE_MENU, DEVMODE_LOWPOW };

