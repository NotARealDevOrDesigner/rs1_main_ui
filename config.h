/*
=============================================================================
config.h - Hardware Configuration & System Constants
=============================================================================
*/

#ifndef CONFIG_H
#define CONFIG_H

// TIMER SYSTEM CONFIGURATION
// =============================================================================
// Servo Configuration
#define SERVO_PIN 9
#define SERVO_START_POSITION 0      // Adjustable start position (0-180°)
#define SERVO_END_POSITION 90       // Adjustable end position (0-180°) 
#define SERVO_ACTIVATION_TIME 0.6   // Adjustable activation time in seconds
#define SERVO_ABSOLUTE_MAX_POSITION 90    // The true 100% position (never changes)
extern int servoAbsoluteMaxPosition;      // Runtime variable for absolute max

// Timer Colors (add to existing color definitions)
#define COLOR_TIMER_PRIMARY   0x007BFF    // Blue for main countdown
#define COLOR_TIMER_SECONDARY 0x808080    // Gray for release phase
#define COLOR_TIMER_OVERLAY   0x1A1A1A    // Dark overlay background

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
// ROTARY ENCODER CONFIGURATION
// =============================================================================
#define ENCODER_PIN_A         8      // Rotary encoder CLK pin (pin A)
#define ENCODER_PIN_B         7      // Rotary encoder DT pin (pin B)  
#define ENCODER_PIN_BUTTON    6      // Rotary encoder push button (optional)
#define ENCODER_DEBOUNCE_MS   50     // Debounce time in milliseconds
#define ENCODER_STEPS_PER_CLICK 1    // Steps per physical click (depends on encoder)

// =============================================================================
// VALUE EDITING CONFIGURATION
// =============================================================================
#define VALUE_INCREMENT_SMALL   1    // Small increment (1 second)
#define VALUE_INCREMENT_LARGE   10   // Large increment (10 seconds)
#define VALUE_MIN_SECONDS       0    // Minimum value (00:00)
#define VALUE_MAX_SECONDS       3599 // Maximum value (59:59)

// Adaptive encoder configuration - VEREINFACHT
#define ENCODER_SPEED_FAST_MS     40   // Fast rotation threshold (time between clicks)
#define ENCODER_SPEED_MEDIUM_MS   120  // Medium rotation threshold
// Alles >= MEDIUM_MS = Slow (1er Schritte)

// Schritt-Größen - NUR NOCH 3 STUFEN
#define ENCODER_STEP_FAST         30   // 30 Sekunden bei schneller Drehung
#define ENCODER_STEP_MEDIUM       10   // 10 Sekunden bei mittlerer Drehung  
#define ENCODER_STEP_SLOW         1    // 1 Sekunde bei langsamer Drehung

// Hysterese für stabiles Verhalten
#define ENCODER_HYSTERESIS_ENABLED true
#define ENCODER_HYSTERESIS_FACTOR  1.5f  // 30% Hysterese

// Glättung/Filterung
#define ENCODER_SMOOTHING_ENABLED true
#define ENCODER_SMOOTHING_SAMPLES 5     // Durchschnitt über 3 Messungen
#define ENCODER_MIN_CHANGE_TIME   20    // Mindestens 15ms zwischen Änderungen

// Value display format
#define VALUE_FORMAT_MM_SS      0    // MM:SS format (minutes:seconds)
#define VALUE_FORMAT_SS         1    // SS format (seconds only)
#define VALUE_FORMAT_COUNT      2    // Simple counter format

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
#define LOADING_DURATION_MS       3000      // 3 seconds loading time
#define LOADING_SPINNER_SPEED     200       // Animation speed in ms

// =============================================================================
// COLOR PALETTE
// =============================================================================
#define COLOR_BLE_INDICATOR   0x007BFF    // Blauer Punkt für BT Status

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

// Debug-Ausgaben für Encoder
#define ENCODER_DEBUG_ENABLED true

#if ENCODER_DEBUG_ENABLED && DEBUG_ENABLED
  #define ENCODER_DEBUG_PRINT(x)      Serial.print("[ENC] "); Serial.print(x)
  #define ENCODER_DEBUG_PRINTLN(x)    Serial.print("[ENC] "); Serial.println(x)
  #define ENCODER_DEBUG_PRINTF(...)   Serial.print("[ENC] "); Serial.printf(__VA_ARGS__)
#else
  #define ENCODER_DEBUG_PRINT(x)
  #define ENCODER_DEBUG_PRINTLN(x)
  #define ENCODER_DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H