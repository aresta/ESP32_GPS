#pragma once

// #define CORE_DEBUG_LEVEL 5 
#define SCREEN_WIDTH 240    /// @brief Display size in pixels
#define SCREEN_HEIGHT 320   /// @brief Display size in pixels
#define SCREEN_BUFFER_WIDTH (SCREEN_WIDTH * 2)      /// @brief Display buffer size in pixels
#define SCREEN_BUFFER_HEIGHT (SCREEN_HEIGHT * 2)    /// @brief Display buffer size in pixels
#define PIXEL_SIZE_DEF 2  /// @brief initial pixel / world meters relation (zoom)
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

/// @brief States of the device
enum DEV_STATES { 
    DEVMODE_NAV,    /// @brief Navigation mode, display is on, joystick disabled
    DEVMODE_MOVE,   /// @brief Map move mode, joystick enabled to navigate the map
    DEVMODE_ZOOM,   /// @brief Map zoom mode, joystick up/down enabled
    DEVMODE_MENU,   /// @brief Change dev mode
    DEVMODE_LOWPOW  /// @brief Low power mode, display is off and GPS in standby
};

// Log disable system based in CORE_DEBUG_LEVEL in platformio.ini
#if CORE_DEBUG_LEVEL > 0
    #define LOGD(...) log_d(__VA_ARGS__)
    #define LOGI(...) log_i(__VA_ARGS__)
    #define LOGW(...) log_w(__VA_ARGS__)
    #define LOGE(...) log_e(__VA_ARGS__)
    #define LOGV(...) log_v(__VA_ARGS__)
#else
    // When debugging is disabled, replace log calls with empty statements
    #define LOGD(...)
    #define LOGI(...)
    #define LOGW(...)
    #define LOGE(...)
    #define LOGV(...)
#endif

