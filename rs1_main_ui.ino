/*
=============================================================================
ESP32-C6 Camera Control App - Main Application (ohne Battery System)
=============================================================================

File Structure:
- config.h         : Hardware configuration & constants
- state_machine.h  : State management system
- hardware.h       : Hardware abstraction layer  
- ui.h            : User interface system with battery
- images.h        : Icons system
- battery.h       : Complete battery management system
- main.ino        : Main application (this file)

=============================================================================
*/

#include "config.h"
#include "state_machine.h"
#include "hardware.h"
#include "ui.h"
#include "battery.h"  // Battery management system

// =============================================================================
// APPLICATION LOGIC
// =============================================================================
void app_init() {
  DEBUG_PRINTLN("=== ESP32-C6 Camera Control App ===");
  DEBUG_PRINTLN("Initializing application...");
  
  // Initialize subsystems in order
  state_machine_init();
  hardware_init();
  
  // Battery system vor UI initialisieren
  battery_init();
  
  ui_init();
  
  DEBUG_PRINTLN("=== Application Ready ===");
  DEBUG_PRINTLN("Available Functions:");
  DEBUG_PRINTLN("- Main page with 4 buttons & battery display");
  DEBUG_PRINTLN("- Template pages with swipe navigation & battery");
  DEBUG_PRINTLN("- Detail pages with dynamic content & battery");
  DEBUG_PRINTLN("- Popup modal system");
  DEBUG_PRINTLN("- State machine navigation");
  DEBUG_PRINTLN("- Battery management system");
  DEBUG_PRINTLN("========================");
}

void app_loop() {
  // Handle LVGL tasks
  lv_timer_handler();
  
  // Battery System Updates
  battery_system_update();
  
  // Example: Update dynamic text every 10 seconds for demo
  static unsigned long lastUpdate = 0;
  static int counter = 0;
  
  if (millis() - lastUpdate > DYNAMIC_TEXT_UPDATE_INTERVAL && app_state.current_state == STATE_DETAIL) {
    lastUpdate = millis();
    counter++;
    String newText = get_detail_heading() + " - Updated " + String(counter) + " times";
    update_dynamic_text(newText);
    lv_label_set_text(detail_content_label, app_state.dynamic_text.c_str());
  }
  
  delay(5);
}

// =============================================================================
// SERIAL COMMANDS FÜR TESTING
// =============================================================================
void handle_serial_commands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Battery commands an das Battery-System weiterleiten
    if (command.startsWith("bat") || command.indexOf("battery") >= 0) {
      handle_battery_serial_commands(command);
    }
    else if (command == "help") {
      Serial.println("Available commands:");
      Serial.println("=== Battery Commands ===");
      Serial.println("  bat <0-100>     - Set battery level");
      Serial.println("  bat demo on/off - Enable/disable demo animation");
      Serial.println("  bat status      - Show battery status");
      Serial.println("  bat charge on/off - Set charging status");
      Serial.println("  bat debug       - Show battery debug info");
      Serial.println("  bat test        - Run battery test animation");
      Serial.println("  bat dims        - Test fill dimensions");
      Serial.println("  bat help        - Show battery commands");
      Serial.println("=== General Commands ===");
      Serial.println("  help            - Show this help");
    }
    else {
      Serial.println("Unknown command. Type 'help' for available commands.");
    }
  }
}

// =============================================================================
// ARDUINO SETUP AND LOOP
// =============================================================================
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  
  DEBUG_PRINTLN("ESP32-C6 Camera Control App Starting...");
  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  DEBUG_PRINTLN("LVGL Version: " + LVGL_Arduino);
  
  // Initialize application
  app_init();
  
  DEBUG_PRINTLN("=== Setup Complete ===");
  DEBUG_PRINTLN("Ready for user interaction!");
  DEBUG_PRINTLN("Serial Commands available - type 'help' for list");
  DEBUG_PRINTLN("Battery demo animation is running...");
  
  // Initial battery status
  print_battery_status();
}

void loop() {
  // Handle serial commands for testing
  handle_serial_commands();
  
  // Main application loop
  app_loop();
}