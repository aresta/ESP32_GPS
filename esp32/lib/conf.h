#ifndef conf_h_
#define conf_h_

#define CORE_DEBUG_LEVEL 5 

#define SCREEN_WIDTH 240    // tft.width()
#define SCREEN_HEIGHT 320   // tft.height()
#define PIXEL_SIZE_DEF 2    // in meters
extern int pixel_size;

// Buttons GPIO nums
#define UP_BUTTON 25
#define DOWN_BUTTON 33
#define LEFT_BUTTON 14
#define RIGHT_BUTTON 12
#define SELECT_BUTTON 22

#define MODE_NAV 1
#define MODE_MOVE 2
#define MODE_ZOOM 3
#define MODE_MENU 4

#endif
