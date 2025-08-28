/*
=============================================================================
settings.h - Persistent Settings Management System
=============================================================================
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Preferences.h>
#include "config.h"
#include "state_machine.h"

// =============================================================================
// SETTINGS CONFIGURATION
// =============================================================================
#define SETTINGS_NAMESPACE "camera_app"
#define SETTINGS_VERSION 1

// Settings keys
#define KEY_SERVO_WIRE_PCT    "servo_wire_pct"
#define KEY_LED_ENABLED       "led_enabled"
#define KEY_BT_ENABLED        "bt_enabled"
#define KEY_TIMER_DELAY       "timer_delay"
#define KEY_TIMER_RELEASE     "timer_release"
#define KEY_TLAPSE_TOTAL      "tlapse_total"
#define KEY_TLAPSE_FRAMES     "tlapse_frames"
#define KEY_INTERVAL_TIME     "interval_time"
#define KEY_SERVO_START_POS   "servo_start_pos"
#define KEY_SERVO_END_POS     "servo_end_pos"
#define KEY_SERVO_MAX_POS     "servo_max_pos"
#define KEY_SERVO_ACT_TIME    "servo_act_time"
#define KEY_SETTINGS_VERSION  "version"

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================
void settings_init();
void save_settings();
void load_settings();
void reset_settings_to_defaults();
bool settings_exist();

// Individual save functions for optimization
void save_app_state();
void save_timer_values();
void save_servo_settings();

// Settings info and debug
void print_settings_info();
void handle_settings_serial_commands(String command);

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================
extern Preferences preferences;
extern bool settings_initialized;

// =============================================================================
// IMPLEMENTATION
// =============================================================================
Preferences preferences;
bool settings_initialized = false;

void settings_init() {
  DEBUG_PRINTLN("Initializing settings system...");
  
  if (settings_exist()) {
    load_settings();
    DEBUG_PRINTLN("Settings loaded from flash");
  } else {
    DEBUG_PRINTLN("No saved settings found - using defaults");
    // Save defaults for first time
    save_settings();
  }
  
  settings_initialized = true;
  print_settings_info();
}

bool settings_exist() {
  preferences.begin(SETTINGS_NAMESPACE, true); // read-only
  bool exists = preferences.isKey(KEY_SETTINGS_VERSION);
  preferences.end();
  return exists;
}

void load_settings() {
  if (!preferences.begin(SETTINGS_NAMESPACE, true)) {
    DEBUG_PRINTLN("ERROR: Failed to open preferences for reading");
    return;
  }
  
  // Check version compatibility
  int saved_version = preferences.getInt(KEY_SETTINGS_VERSION, 0);
  if (saved_version != SETTINGS_VERSION) {
    DEBUG_PRINTF("Settings version mismatch: saved=%d, current=%d\n", saved_version, SETTINGS_VERSION);
    preferences.end();
    reset_settings_to_defaults();
    return;
  }
  
// Load app state
  app_state.servo_wire_percentage = preferences.getInt(KEY_SERVO_WIRE_PCT, 100);
  app_state.led_enabled = preferences.getBool(KEY_LED_ENABLED, true);
  app_state.bluetooth_enabled = preferences.getBool(KEY_BT_ENABLED, false);
  
  // Load timer values AND initialize labels
  timer_values.page_title = "Timer";
  timer_values.option1_label = "Delay";
  timer_values.option2_label = "Release";
  timer_values.option1 = {
    preferences.getUInt(KEY_TIMER_DELAY, 0),
    VALUE_FORMAT_MM_SS, 0, 3599, VALUE_INCREMENT_SMALL
  };
  timer_values.option2 = {
    preferences.getUInt(KEY_TIMER_RELEASE, 0),
    VALUE_FORMAT_MM_SS, 0, 3599, VALUE_INCREMENT_SMALL
  };
  
  // Load T-Lapse values AND initialize labels
  tlapse_values.page_title = "Timelapse";
  tlapse_values.option1_label = "Total";
  tlapse_values.option2_label = "Frames";
  tlapse_values.option1 = {
    preferences.getUInt(KEY_TLAPSE_TOTAL, 0),
    VALUE_FORMAT_MM_SS, 0, 3599, VALUE_INCREMENT_SMALL
  };
  tlapse_values.option2 = {
    preferences.getUInt(KEY_TLAPSE_FRAMES, 0),
    VALUE_FORMAT_COUNT, 0, 0, 1
  };
  
  // Load Interval values AND initialize labels
  interval_values.page_title = "Interval";
  interval_values.option1_label = "Interval";
  interval_values.option2_label = "";  // Empty, not used
  interval_values.option1 = {
    preferences.getUInt(KEY_INTERVAL_TIME, 0),
    VALUE_FORMAT_MM_SS, 0, 3599, VALUE_INCREMENT_SMALL
  };
  interval_values.option2 = {0, VALUE_FORMAT_COUNT, 0, 0, 1}; // Not used
  
  // Load servo settings (if implemented)
  // servoStartPosition = preferences.getInt(KEY_SERVO_START_POS, SERVO_START_POSITION);
  // servoEndPosition = preferences.getInt(KEY_SERVO_END_POS, SERVO_END_POSITION);
  // servoAbsoluteMaxPosition = preferences.getInt(KEY_SERVO_MAX_POS, SERVO_ABSOLUTE_MAX_POSITION);
  // servoActivationTime = preferences.getFloat(KEY_SERVO_ACT_TIME, SERVO_ACTIVATION_TIME);
  
  preferences.end();
  
  // Update page content after loading
  update_page_content_from_values(STATE_TIMER);
  update_page_content_from_values(STATE_TLAPSE);
  update_page_content_from_values(STATE_INTERVAL);
  
  DEBUG_PRINTLN("Settings loaded successfully");
}

void save_settings() {
  if (!settings_initialized && !preferences.begin(SETTINGS_NAMESPACE, false)) {
    DEBUG_PRINTLN("ERROR: Failed to open preferences for writing");
    return;
  }
  
  if (settings_initialized) {
    preferences.begin(SETTINGS_NAMESPACE, false);
  }
  
  // Save version
  preferences.putInt(KEY_SETTINGS_VERSION, SETTINGS_VERSION);
  
  // Save app state
  preferences.putInt(KEY_SERVO_WIRE_PCT, app_state.servo_wire_percentage);
  preferences.putBool(KEY_LED_ENABLED, app_state.led_enabled);
  preferences.putBool(KEY_BT_ENABLED, app_state.bluetooth_enabled);
  
  // Save timer values
  preferences.putUInt(KEY_TIMER_DELAY, timer_values.option1.seconds);
  preferences.putUInt(KEY_TIMER_RELEASE, timer_values.option2.seconds);
  preferences.putUInt(KEY_TLAPSE_TOTAL, tlapse_values.option1.seconds);
  preferences.putUInt(KEY_TLAPSE_FRAMES, tlapse_values.option2.seconds);
  preferences.putUInt(KEY_INTERVAL_TIME, interval_values.option1.seconds);
  
  // Save servo settings (if implemented)
  // preferences.putInt(KEY_SERVO_START_POS, servoStartPosition);
  // preferences.putInt(KEY_SERVO_END_POS, servoEndPosition);
  // preferences.putInt(KEY_SERVO_MAX_POS, servoAbsoluteMaxPosition);
  // preferences.putFloat(KEY_SERVO_ACT_TIME, servoActivationTime);
  
  preferences.end();
  DEBUG_PRINTLN("Settings saved to flash");
}

void save_app_state() {
  preferences.begin(SETTINGS_NAMESPACE, false);
  preferences.putInt(KEY_SERVO_WIRE_PCT, app_state.servo_wire_percentage);
  preferences.putBool(KEY_LED_ENABLED, app_state.led_enabled);
  preferences.putBool(KEY_BT_ENABLED, app_state.bluetooth_enabled);
  preferences.end();
}

void save_timer_values() {
  preferences.begin(SETTINGS_NAMESPACE, false);
  preferences.putUInt(KEY_TIMER_DELAY, timer_values.option1.seconds);
  preferences.putUInt(KEY_TIMER_RELEASE, timer_values.option2.seconds);
  preferences.putUInt(KEY_TLAPSE_TOTAL, tlapse_values.option1.seconds);
  preferences.putUInt(KEY_TLAPSE_FRAMES, tlapse_values.option2.seconds);
  preferences.putUInt(KEY_INTERVAL_TIME, interval_values.option1.seconds);
  preferences.end();
}

void save_servo_settings() {
  // preferences.begin(SETTINGS_NAMESPACE, false);
  // preferences.putInt(KEY_SERVO_START_POS, servoStartPosition);
  // preferences.putInt(KEY_SERVO_END_POS, servoEndPosition);
  // preferences.putInt(KEY_SERVO_MAX_POS, servoAbsoluteMaxPosition);
  // preferences.putFloat(KEY_SERVO_ACT_TIME, servoActivationTime);
  // preferences.end();
  // Uncomment when servo settings are implemented
}

void reset_settings_to_defaults() {
  DEBUG_PRINTLN("Resetting settings to defaults...");
  
  preferences.begin(SETTINGS_NAMESPACE, false);
  preferences.clear(); // Clear all keys
  preferences.end();
  
  // Reset to default values
  app_state.servo_wire_percentage = 100;
  app_state.led_enabled = true;
  app_state.bluetooth_enabled = false;
  
  // Reset timer values through existing function
  values_init();
  
  // Save defaults
  save_settings();
  
  DEBUG_PRINTLN("Settings reset complete");
}

void print_settings_info() {
  DEBUG_PRINTLN("=== Current Settings ===");
  DEBUG_PRINTF("Servo Wire: %d%%\n", app_state.servo_wire_percentage);
  DEBUG_PRINTF("LED: %s\n", app_state.led_enabled ? "ON" : "OFF");
  DEBUG_PRINTF("Bluetooth: %s\n", app_state.bluetooth_enabled ? "ON" : "OFF");
  DEBUG_PRINTF("Timer Delay: %ds\n", timer_values.option1.seconds);
  DEBUG_PRINTF("Timer Release: %ds\n", timer_values.option2.seconds);
  DEBUG_PRINTF("T-Lapse Total: %ds\n", tlapse_values.option1.seconds);
  DEBUG_PRINTF("T-Lapse Frames: %d\n", tlapse_values.option2.seconds);
  DEBUG_PRINTF("Interval: %ds\n", interval_values.option1.seconds);
  
  // Storage info
  preferences.begin(SETTINGS_NAMESPACE, true);
  size_t used_entries = preferences.freeEntries();
  preferences.end();
  
  DEBUG_PRINTF("Storage: %d entries used\n", used_entries);
  DEBUG_PRINTLN("========================");
}

void handle_settings_serial_commands(String command) {
  if (command == "settings save") {
    save_settings();
    DEBUG_PRINTLN("Settings manually saved");
  }
  else if (command == "settings load") {
    load_settings();
    DEBUG_PRINTLN("Settings manually loaded");
  }
  else if (command == "settings reset") {
    reset_settings_to_defaults();
    DEBUG_PRINTLN("Settings reset to defaults");
  }
  else if (command == "settings info") {
    print_settings_info();
  }
  else if (command == "settings exist") {
    DEBUG_PRINTF("Settings exist: %s\n", settings_exist() ? "YES" : "NO");
  }
  else if (command == "settings help") {
    DEBUG_PRINTLN("Settings Commands:");
    DEBUG_PRINTLN("  settings save   - Save current settings");
    DEBUG_PRINTLN("  settings load   - Reload settings from flash");
    DEBUG_PRINTLN("  settings reset  - Reset to defaults");
    DEBUG_PRINTLN("  settings info   - Show current settings");
    DEBUG_PRINTLN("  settings exist  - Check if settings exist");
  }
}

#endif // SETTINGS_H