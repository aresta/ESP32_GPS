#ifndef conf_h_
#define conf_h_

#define CORE_DEBUG_LEVEL 5 

#define SCREEN_WIDTH 240    // tft.width()
#define SCREEN_HEIGHT 320   // tft.height()
#define PIXEL_SIZE_DEF 2    // in meters
#define MAX_ZOOM 7
extern int zoom_level;

// Buttons GPIO nums
#define UP_BUTTON 25
#define DOWN_BUTTON 33
#define LEFT_BUTTON 34
#define RIGHT_BUTTON 35
// #define LEFT_BUTTON 14
// #define RIGHT_BUTTON 12
#define SELECT_BUTTON 22
#define TFT_OFF_BUTTON 32
#define TFT_BLK_PIN 21
#define TFT_CS 15
#define SD_CS 13
#define GPS_CE 14
#define MENU_BUTTON 5

#define DEVMODE_NAV 1
#define DEVMODE_MOVE 2
#define DEVMODE_ZOOM 3
#define DEVMODE_MENU 4


#endif
