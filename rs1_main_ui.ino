/*
=============================================================================
ESP32-C6 Camera Control App - Cleaned for Battery Debugging
=============================================================================
*/

#include "config.h"
#include "state_machine.h"
#include "hardware.h"
#include "settings.h" 
#include "ui.h"
#include "battery.h"
#include "timer_system.h"
#include "bluetooth.h"

// =============================================================================
// APPLICATION LOGIC
// =============================================================================
void app_init() {
  Serial.println("=== ESP32-C6 Camera Control App ===");
  Serial.println("Initializing application...");
  
  // Initialize subsystems in order
  state_machine_init();
  settings_init();
  hardware_init();
  
  // Battery system initialization
  if (!LOADING_SCREEN_ENABLED) {
    battery_init();
    Serial.println("Battery system initialized");
  } else {
    Serial.println("Battery system init delayed until after loading");
  }

  ui_init();
  bluetooth_init();
  
  Serial.println("=== Application Ready ===");
}

void app_loop() {
  // Handle LVGL tasks
  lv_timer_handler();
  
  // Check if loading screen should timeout
  if (LOADING_SCREEN_ENABLED) {
    check_loading_timeout();
    
    // Initialize battery system after loading screen ends
    static bool battery_initialized_after_loading = false;
    if (app_state.current_state != STATE_LOADING && !battery_initialized_after_loading) {
      battery_init();
      battery_initialized_after_loading = true;
      Serial.println("Battery system initialized after loading screen");
    }
  }
  
  // Battery System Updates (only if initialized)
  static bool battery_system_active = false;
  if (app_state.current_state != STATE_LOADING || !LOADING_SCREEN_ENABLED) {
    if (!battery_system_active) {
      battery_system_active = true;
    }
    battery_system_update();
  }
  
  // Other systems
  bluetooth_update();
  timer_system_update();
  
  delay(5);
}

// =============================================================================
// SERIAL COMMANDS - MINIMAL FOR DEBUGGING
// =============================================================================
void handle_serial_commands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Battery commands - route to battery.h
    if (command.startsWith("bat") || command.indexOf("battery") >= 0) {
      handle_battery_serial_commands(command);
    }
    // Direct pin testing
    else if (command == "pintest") {
      bool charging = digitalRead(3) == LOW;  
      bool switch_on = digitalRead(4) == HIGH; 
      Serial.printf("=== PIN TEST ===\n");
      Serial.printf("GPIO 3 (CHARGE): %s -> charging = %s\n", 
                   digitalRead(3) ? "HIGH" : "LOW", charging ? "true" : "false");
      Serial.printf("GPIO 4 (SWITCH): %s -> switch_on = %s\n", 
                   digitalRead(4) ? "HIGH" : "LOW", switch_on ? "true" : "false");
      Serial.printf("Expected State:\n");
      if (!charging && !switch_on) {
        Serial.println("  -> OFF overlay (State 4)");
      } else if (charging && !switch_on) {
        Serial.println("  -> CHARGING overlay (State 2)");
      } else if (!charging && switch_on) {
        Serial.println("  -> NORMAL operation (State 0)");
      } else if (charging && switch_on) {
        Serial.println("  -> CHARGING animation (State 1)");
      }
      Serial.println("================\n");
    }
    // State debugging
    else if (command == "statetest") {
      Serial.printf("=== STATE TEST ===\n");
      Serial.printf("Current battery system state: %d\n", battery_state.system_state);
      Serial.printf("Battery level: %d%% (real: %d%%)\n", battery_state.level, battery_state.real_level);
      Serial.printf("Charging: %s\n", battery_state.is_charging ? "YES" : "NO");
      Serial.printf("Switch On: %s\n", battery_state.is_power_switch_on ? "YES" : "NO");
      Serial.printf("MAX17048: %s\n", battery_state.max17048_available ? "Available" : "Not available");
      Serial.println("==================\n");
    }
    // Skip loading screen
    else if (command == "skip") {
      if (app_state.current_state == STATE_LOADING) {
        change_state(STATE_MAIN);
        show_current_page();
        Serial.println("Loading screen skipped");
      }
    }
    // Help
    else if (command == "help") {
      Serial.println("=== DEBUG COMMANDS ===");
      Serial.println("pintest   - Test GPIO pins directly");
      Serial.println("statetest - Show battery system state");
      Serial.println("bat status - Full battery status");
      Serial.println("bat pins  - Battery pin readings");
      Serial.println("skip      - Skip loading screen");
      Serial.println("======================");
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
  Serial.println("ESP32-C6 Camera Control App Starting...");
  
  // Initialize GPIO pins early for testing
  pinMode(3, INPUT_PULLUP);  // CHARGE_PIN
  pinMode(4, INPUT_PULLUP);  // POWER_SWITCH_PIN
  
  app_init();
  
  Serial.println("=== Setup Complete ===");
  Serial.println("Commands: pintest, statetest, bat status, help");
}

void loop() {
  handle_serial_commands();
  app_loop();
}