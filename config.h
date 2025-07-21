/*
=============================================================================
config.h - Hardware Configuration & System Constants
=============================================================================
*/

#ifndef CONFIG_H
#define CONFIG_H

// =============================================================================
// HARDWARE CONFIGURATION
// =============================================================================
#define ROTATION 0
#define GFX_BL 23

#define Touch_I2C_SDA 18
#define Touch_I2C_SCL 19
#define Touch_RST     20
#define Touch_INT     21

#define LEDC_FREQ             5000
#define LEDC_TIMER_10_BIT     10

// =============================================================================
// DISPLAY SETTINGS
// =============================================================================
#define SCREEN_WIDTH  172
#define SCREEN_HEIGHT 320

// =============================================================================
// UI CONSTANTS
// =============================================================================
#define ANIMATION_TIME_MS     300
#define SWIPE_THRESHOLD       30
#define DYNAMIC_TEXT_UPDATE_INTERVAL 10000

// =============================================================================
// LOADING SCREEN CONFIGURATION
// =============================================================================
#define LOADING_SCREEN_ENABLED    true      // Set to false to skip loading screen for debugging
#define LOADING_DURATION_MS       7000      // 3 seconds loading time
#define LOADING_SPINNER_SPEED     200       // Animation speed in ms

// =============================================================================
// COLOR PALETTE
// =============================================================================
#define COLOR_BG_MAIN         0xF4F4F4
#define COLOR_BG_TEMPLATE     0xF4F4F4
#define COLOR_BG_DETAIL       0xF4F4F4
#define COLOR_BG_HEADER       0xF4F4F4
#define COLOR_BG_DETAIL_HEADER 0xF4F4F4
#define COLOR_BG_LOADING      0x1E1E1E    // Dark background for loading screen

#define COLOR_BTN_PRIMARY     0x1E1E1E
#define COLOR_BTN_SECONDARY   0xE0E0E0
#define COLOR_BTN_DANGER      0xBF616A
#define COLOR_BTN_SUCCESS     0xA3BE8C
#define COLOR_BTN_WARNING     0xD08770

#define COLOR_TEXT_PRIMARY    0x1E1E1E
#define COLOR_TEXT_SECONDARY  0xF4F4F4
#define COLOR_TEXT_DARK       0x2E3440
#define COLOR_TEXT_LOADING    0xF4F4F4   // Light text for loading screen

#define COLOR_DOT_ACTIVE      0x1E1E1E
#define COLOR_DOT_INACTIVE    0xE0E0E0

#define COLOR_POPUP_OVERLAY   0xF4F4F4
#define COLOR_POPUP_BORDER    0x5E81AC

#define COLOR_BATTERY_LOAD    0x1E1E1E

#define COLOR_LOADING_SPINNER 0x5E81AC   // Blue spinner color
#define COLOR_LOADING_ACCENT  0xD08770   // Orange accent color

// =============================================================================
// DEBUG SETTINGS
// =============================================================================
#define SERIAL_BAUD_RATE      115200
#define DEBUG_ENABLED         true

#if DEBUG_ENABLED
  #define DEBUG_PRINT(x)      Serial.print(x)
  #define DEBUG_PRINTLN(x)    Serial.println(x)
  #define DEBUG_PRINTF(...)   Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H