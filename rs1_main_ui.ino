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
// ROTARY ENCODER HANDLING
// =============================================================================
void handle_encoder_input() {
  // Get adaptive encoder movement (speed-based steps)
  int32_t encoder_delta = get_adaptive_encoder_delta();
  
if (encoder_delta != 0) {
    // Handle encoder input based on current state
    if (app_state.current_state == STATE_INTERVAL) {
      // Interval page: always edit the single option (option 0)
      update_option_value(STATE_INTERVAL, 0, encoder_delta);
      // Force UI update for interval page
      update_interval_content(interval_content);
      DEBUG_PRINTF("Adaptive encoder moved: %d, updating Interval timer\n", encoder_delta);
    }


    else if (app_state.current_state == STATE_WIRE_SETTINGS) {
      // Wire settings: adjust percentage (0-100%)
      app_state.servo_wire_percentage += encoder_delta;
      if (app_state.servo_wire_percentage < 0) {
        app_state.servo_wire_percentage = 0;
      } else if (app_state.servo_wire_percentage > 100) {
        app_state.servo_wire_percentage = 100;
      }
  
      update_wire_percentage_display();
  
      // Calculate and move servo...
      int servo_range = servoAbsoluteMaxPosition - servoStartPosition;
      int target_position = servoStartPosition + (servo_range * app_state.servo_wire_percentage / 100);
      servoEndPosition = target_position;
      servo_move_to_position(target_position);
  
      DEBUG_PRINTF("Wire percentage: %d%%, Working stop updated to: %d°\n", 
               app_state.servo_wire_percentage, servoEndPosition);
  
      // GEÄNDERT: Immer speichern, unabhängig von settings_initialized
      static unsigned long last_wire_save = 0;
      if (millis() - last_wire_save > 1000) { // Max 1x pro Sekunde
        preferences.begin(SETTINGS_NAMESPACE, false);
        preferences.putInt(KEY_SERVO_WIRE_PCT, app_state.servo_wire_percentage);
        preferences.end();
        last_wire_save = millis();
        DEBUG_PRINTF("DIRECT SAVE: Wire percentage %d%% saved to flash\n", app_state.servo_wire_percentage);
      }
    }

    else if (is_main_template_state(app_state.current_state) && !app_state.is_animating) {
      // Timer/T-Lapse pages: edit the currently visible card
      // IMPORTANT: Use current_option to determine which card is active
      int target_option = app_state.current_option;
      
      DEBUG_PRINTF("Adaptive encoder input - Current option: %d, Target option: %d\n", 
                   app_state.current_option, target_option);
      
      update_option_value(app_state.current_state, target_option, encoder_delta);
      
      // Force UI update for template pages - but don't change the active card!
      update_template_content(get_current_content());
      
      DEBUG_PRINTF("Adaptive encoder moved: %d, updating %s option %d (visible card)\n", 
                   encoder_delta, 
                   get_current_content().heading.c_str(),
                   target_option + 1);
    }
  }
  
  // Check if encoder button was pressed (optional - for future use)
  if (is_encoder_button_pressed()) {
    DEBUG_PRINTLN("Encoder button pressed");
    // Could be used to toggle between options manually
    if (is_main_template_state(app_state.current_state) && app_state.current_state != STATE_INTERVAL) {
      // Toggle between option 0 and 1 for Timer/T-Lapse
      app_state.current_option = (app_state.current_option == 0) ? 1 : 0;
      animate_to_option(app_state.current_option);
      DEBUG_PRINTF("Encoder button: Switched to option %d\n", app_state.current_option);
    }
  }
}
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
  
  bool charging = (digitalRead(CHARGE_PIN) == LOW);
  bool power_switch_on = (digitalRead(POWER_SWITCH_PIN) == HIGH);
  
  // Skip loading screen wenn im Charging-Modus ohne Schalter
  if (charging && !power_switch_on) {
    Serial.println("Device starting in charging mode - skipping loading screen");
    battery_init();
    change_state(STATE_MAIN); // Oder direkt zum Charging Screen
    ui_init();
  } else {
    // Normaler Startup mit Loading Screen
    if (!LOADING_SCREEN_ENABLED) {
      battery_init();
      Serial.println("Battery system initialized");
    } else {
      Serial.println("Battery system init delayed until after loading");
    }
    ui_init();
  }
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
  
  handle_encoder_input();

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
  
    // Show loading screen configuration
  if (LOADING_SCREEN_ENABLED) {
    DEBUG_PRINTF("Loading screen: ENABLED (%d ms duration)\n", LOADING_DURATION_MS);
  } else {
    DEBUG_PRINTLN("Loading screen: DISABLED (debug mode)");
  }

  app_init();
  
  Serial.println("=== Setup Complete ===");
  Serial.println("Commands: pintest, statetest, bat status, help");
}

void loop() {
  handle_serial_commands();
  app_loop();
}