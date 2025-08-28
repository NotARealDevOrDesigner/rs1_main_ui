/*
=============================================================================
ESP32-C6 Camera Control App - Main Application (Fixed without Detail)
=============================================================================
*/

#include "config.h"
#include "state_machine.h"
#include "hardware.h"
#include "ui.h"
#include "battery.h"
#include "timer_system.h" 

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

    /*
    else if (app_state.current_state == STATE_WIRE_SETTINGS) {
      // Wire settings: adjust percentage (0-100%)
      app_state.servo_wire_percentage += encoder_delta;
      if (app_state.servo_wire_percentage < 0) {
        app_state.servo_wire_percentage = 0;
      } else if (app_state.servo_wire_percentage > 100) {
        app_state.servo_wire_percentage = 100;
      }
      
      update_wire_percentage_display();
      
      // LIVE SERVO MOVEMENT - Calculate actual position
      int servo_range = servoEndPosition - servoStartPosition;
      int target_position = servoStartPosition + (servo_range * app_state.servo_wire_percentage / 100);
      servo_move_to_position(target_position);
      
      DEBUG_PRINTF("Wire percentage: %d%%, Servo position: %d°\n", 
                   app_state.servo_wire_percentage, target_position);
    }
    */
    else if (app_state.current_state == STATE_WIRE_SETTINGS) {
      // Wire settings: adjust percentage (0-100%)
      app_state.servo_wire_percentage += encoder_delta;
      if (app_state.servo_wire_percentage < 0) {
        app_state.servo_wire_percentage = 0;
      } else if (app_state.servo_wire_percentage > 100) {
        app_state.servo_wire_percentage = 100;
      }
    update_wire_percentage_display();
  
    // UPDATED: Calculate position from absolute maximum, then update working stop
    int servo_range = servoAbsoluteMaxPosition - servoStartPosition;
    int target_position = servoStartPosition + (servo_range * app_state.servo_wire_percentage / 100);
  
    // Update the working stop position
    servoEndPosition = target_position;
  
    // LIVE SERVO MOVEMENT - move to new working position
    servo_move_to_position(target_position);
  
    DEBUG_PRINTF("Wire percentage: %d%%, Working stop updated to: %d° (absolute max: %d°)\n", 
                app_state.servo_wire_percentage, servoEndPosition, servoAbsoluteMaxPosition);
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
  DEBUG_PRINTLN("=== ESP32-C6 Camera Control App ===");
  DEBUG_PRINTLN("Initializing application...");
  
  // Initialize subsystems in order
  state_machine_init();
  hardware_init();  // This now calls encoder_init_extended() internally
  
  // Battery system initialization (delayed if loading screen is active)
  if (!LOADING_SCREEN_ENABLED) {
    battery_init();
    DEBUG_PRINTLN("Battery system initialized");
  } else {
    DEBUG_PRINTLN("Battery system init delayed until after loading");
  }
  
  ui_init();
  
  DEBUG_PRINTLN("=== Application Ready ===");
  DEBUG_PRINTLN("Available Functions:");
  DEBUG_PRINTLN("- Loading screen with animations");
  DEBUG_PRINTLN("- Smart navigation system (Main → Sub, no Detail)");
  DEBUG_PRINTLN("- Main page with 4 buttons & battery display");
  DEBUG_PRINTLN("- Template pages with swipe navigation & battery");
  DEBUG_PRINTLN("- Simplified navigation (no detail subsites)");
  DEBUG_PRINTLN("- State machine navigation");
  DEBUG_PRINTLN("- Battery management system");
  DEBUG_PRINTLN("- Rotary encoder value editing (3 SPEEDS)");
  DEBUG_PRINTF("- Loading screen: %s\n", LOADING_SCREEN_ENABLED ? "ENABLED" : "DISABLED");
  DEBUG_PRINTLN("========================");
  DEBUG_PRINTLN("Navigation Hierarchy (SIMPLIFIED):");
  DEBUG_PRINTLN("  MAIN");
  DEBUG_PRINTLN("  ├── TIMER (cards: Delay, Release)");
  DEBUG_PRINTLN("  ├── TLAPSE (cards: Total, Frames)");
  DEBUG_PRINTLN("  ├── INTERVAL (single card: Interval)");
  DEBUG_PRINTLN("  └── SETTINGS");
  DEBUG_PRINTLN("========================");
  DEBUG_PRINTLN("Encoder Functions (3 SPEEDS):");
  DEBUG_PRINTLN("- Turn encoder to change visible card values");
  DEBUG_PRINTLN("- Swipe cards to change which value is being edited");
  DEBUG_PRINTLN("- Values are saved globally and persist");
  DEBUG_PRINTLN("- 3 speeds: 1s, 10s, 30s steps");
  DEBUG_PRINTLN("- Cards are no longer clickable (no detail pages)");
  DEBUG_PRINTLN("========================");
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
      DEBUG_PRINTLN("Battery system initialized after loading screen");
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
  
  // Timer system
  timer_system_update();

  // Handle rotary encoder input
  handle_encoder_input();
  
  // REMOVED: Dynamic text update for detail page (no longer exists)
  
  delay(5);
}

// =============================================================================
// SERIAL COMMANDS FOR TESTING
// =============================================================================
void handle_serial_commands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Battery commands
    if (command.startsWith("bat") || command.indexOf("battery") >= 0) {
      handle_battery_serial_commands(command);
    }
    // Encoder test commands
    else if (command.startsWith("enc ")) {
      String param = command.substring(4);
      if (param == "up" || param == "+") {
        if (is_main_template_state(app_state.current_state)) {
          update_option_value(app_state.current_state, app_state.current_option, 1);
          DEBUG_PRINTLN("Simulated encoder up (1 step)");
        } else {
          DEBUG_PRINTLN("Encoder only works on template pages (Timer/T-Lapse/Interval)");
        }
      }
      else if (param == "up10") {
        if (is_main_template_state(app_state.current_state)) {
          update_option_value(app_state.current_state, app_state.current_option, 10);
          DEBUG_PRINTLN("Simulated encoder up (10 steps - medium speed)");
        } else {
          DEBUG_PRINTLN("Encoder only works on template pages (Timer/T-Lapse/Interval)");
        }
      }
      else if (param == "up30") {
        if (is_main_template_state(app_state.current_state)) {
          update_option_value(app_state.current_state, app_state.current_option, 30);
          DEBUG_PRINTLN("Simulated encoder up (30 steps - fast speed)");
        } else {
          DEBUG_PRINTLN("Encoder only works on template pages (Timer/T-Lapse/Interval)");
        }
      }
      else if (param == "down" || param == "-") {
        if (is_main_template_state(app_state.current_state)) {
          update_option_value(app_state.current_state, app_state.current_option, -1);
          DEBUG_PRINTLN("Simulated encoder down");
        } else {
          DEBUG_PRINTLN("Encoder only works on template pages (Timer/T-Lapse/Interval)");
        }
      }
      else if (param == "test") {
        if (is_main_template_state(app_state.current_state)) {
          PageValues* values = get_current_page_values();
          DEBUG_PRINTF("Current page: %s\n", values->page_title.c_str());
          DEBUG_PRINTF("Option %d (%s): %s\n", 
                       app_state.current_option + 1,
                       (app_state.current_option == 0) ? values->option1_label.c_str() : values->option2_label.c_str(),
                       format_time_value(get_option_value(app_state.current_state, app_state.current_option), 
                                       (app_state.current_option == 0) ? values->option1.format : values->option2.format).c_str());
        } else {
          DEBUG_PRINTLN("Not on a template page");
        }
      }

      else if (command == "nav settings") {
        DEBUG_PRINTLN("Navigating to SETTINGS");
        change_state(STATE_SETTINGS);
        show_current_page();
      }
      else if (command == "nav wire") {
        DEBUG_PRINTLN("Navigating to WIRE SETTINGS");
        change_state(STATE_WIRE_SETTINGS);
        show_current_page();
      }
      else if (command.startsWith("wire ")) {
        String param = command.substring(5);
        if (param.toInt() >= 0 && param.toInt() <= 100) {
          app_state.servo_wire_percentage = param.toInt();
          update_wire_percentage_display();
          DEBUG_PRINTF("Wire percentage set to: %d%%\n", app_state.servo_wire_percentage);
        }
      }

      else if (param == "debug") {
        DEBUG_PRINTLN("=== Encoder Debug Info ===");
        ENCODER_DEBUG_PRINTF("Current speed level: %d\n", current_speed_level);
        ENCODER_DEBUG_PRINTF("Last step size: %d\n", encoder_speed_step_size);
        ENCODER_DEBUG_PRINTF("Hysteresis: %s\n", ENCODER_HYSTERESIS_ENABLED ? "ON" : "OFF");
        ENCODER_DEBUG_PRINTF("Smoothing: %s\n", ENCODER_SMOOTHING_ENABLED ? "ON" : "OFF");
        DEBUG_PRINTLN("Speed thresholds (3 levels):");
        DEBUG_PRINTF("  Fast: < %d ms (30s steps)\n", ENCODER_SPEED_FAST_MS);
        DEBUG_PRINTF("  Medium: < %d ms (10s steps)\n", ENCODER_SPEED_MEDIUM_MS);
        DEBUG_PRINTF("  Slow: >= %d ms (1s steps)\n", ENCODER_SPEED_MEDIUM_MS);
        DEBUG_PRINTLN("========================");
      }
    }
    else if (command == "values dump") {
      DEBUG_PRINTLN("=== Current Values ===");
      DEBUG_PRINTF("Timer - Delay: %s, Release: %s\n", 
                   format_time_value(timer_values.option1.seconds, timer_values.option1.format).c_str(),
                   format_time_value(timer_values.option2.seconds, timer_values.option2.format).c_str());
      DEBUG_PRINTF("T-Lapse - Total: %s, Frames: %s\n", 
                   format_time_value(tlapse_values.option1.seconds, tlapse_values.option1.format).c_str(),
                   format_time_value(tlapse_values.option2.seconds, tlapse_values.option2.format).c_str());
      DEBUG_PRINTF("Interval - Interval: %s\n", 
                   format_time_value(interval_values.option1.seconds, interval_values.option1.format).c_str());
      DEBUG_PRINTLN("======================");
    }
    else if (command == "values reset") {
      DEBUG_PRINTLN("Resetting all values to defaults...");
      values_init();
      DEBUG_PRINTLN("Values reset complete");
    }
    // Loading screen commands
    else if (command == "loading skip" || command == "skip") {
      if (app_state.current_state == STATE_LOADING) {
        DEBUG_PRINTLN("Skipping loading screen...");
        change_state(STATE_MAIN);
        show_current_page();
      } else {
        DEBUG_PRINTLN("Loading screen is not active");
      }
    }
    else if (command == "loading status") {
      DEBUG_PRINTF("Loading screen enabled: %s\n", LOADING_SCREEN_ENABLED ? "YES" : "NO");
      DEBUG_PRINTF("Current state: %d\n", app_state.current_state);
      if (app_state.current_state == STATE_LOADING) {
        unsigned long elapsed = millis() - app_state.loading_start_time;
        unsigned long remaining = LOADING_DURATION_MS - elapsed;
        DEBUG_PRINTF("Loading time remaining: %lu ms\n", remaining > 0 ? remaining : 0);
      }
    }
    // Navigation test commands - SIMPLIFIED (removed detail navigation)
    else if (command == "nav test") {
      DEBUG_PRINTLN("Testing navigation system...");
      DEBUG_PRINTF("Current: %d, Parent would be: %d\n", 
                   app_state.current_state, get_parent_state(app_state.current_state));
    }
    else if (command == "nav timer") {
      DEBUG_PRINTLN("Navigating to TIMER");
      change_state(STATE_TIMER);
      show_current_page();
    }
    else if (command == "nav tlapse") {
      DEBUG_PRINTLN("Navigating to T-LAPSE");
      change_state(STATE_TLAPSE);
      show_current_page();
    }
    else if (command == "nav interval") {
      DEBUG_PRINTLN("Navigating to INTERVAL");
      change_state(STATE_INTERVAL);
      show_current_page();
    }
    else if (command == "nav main") {
      DEBUG_PRINTLN("Navigating to MAIN");
      change_state(STATE_MAIN);
      show_current_page();
    }
    else if (command == "nav back") {
      DEBUG_PRINTLN("Testing back navigation");
      go_back();
    }
    // Timer system commands
    else if (command.startsWith("timer ")) {
      String param = command.substring(6);
      if (param == "start") {
        if (app_state.current_state == STATE_TIMER) {
          start_timer_execution();
          DEBUG_PRINTLN("Timer started via serial command");
        } else {
          DEBUG_PRINTLN("Navigate to Timer page first");
        }
      }
      else if (param == "cancel") {
        if (runtime.state != TIMER_IDLE) {
          cancel_timer_execution();
          show_current_page();
          DEBUG_PRINTLN("Timer cancelled via serial command");
        } else {
          DEBUG_PRINTLN("No timer running");
        }
      }
    }
    else if (command.startsWith("tlapse ")) {
      String param = command.substring(7);
      if (param == "start") {
        if (app_state.current_state == STATE_TLAPSE) {
          start_tlapse_execution();
          DEBUG_PRINTLN("T-Lapse started via serial command");
        } else {
          DEBUG_PRINTLN("Navigate to T-Lapse page first");
        }
      }
    }
    else if (command.startsWith("interval ")) {
      String param = command.substring(9);
      if (param == "start") {
        if (app_state.current_state == STATE_INTERVAL) {
          start_interval_execution();
          DEBUG_PRINTLN("Interval started via serial command");
        } else {
          DEBUG_PRINTLN("Navigate to Interval page first");
        }
      }
    }
    else if (command == "help") {
      Serial.println("Available commands:");
      Serial.println("=== Loading Screen Commands ===");
      Serial.println("  loading skip    - Skip current loading screen");
      Serial.println("  loading status  - Show loading screen status");
      Serial.println("  skip            - Alias for loading skip");
      Serial.println("=== Navigation Commands (SIMPLIFIED) ===");
      Serial.println("  nav test        - Test navigation system");
      Serial.println("  nav timer       - Go to timer page");
      Serial.println("  nav tlapse      - Go to t-lapse page");
      Serial.println("  nav interval    - Go to interval page");
      Serial.println("  nav main        - Go to main page");
      Serial.println("  nav back        - Test back navigation");
      Serial.println("=== Encoder Commands (3 SPEEDS) ===");
      Serial.println("  enc up/+        - Simulate encoder up (1s)");
      Serial.println("  enc up10        - Simulate medium speed (10s)");
      Serial.println("  enc up30        - Simulate fast speed (30s)");
      Serial.println("  enc down/-      - Simulate encoder down");  
      Serial.println("  enc test        - Show current option info");
      Serial.println("  enc debug       - Show encoder debug info");
      Serial.println("=== Value Commands ===");
      Serial.println("  values dump     - Show all current values");
      Serial.println("  values reset    - Reset all values to defaults");
      Serial.println("=== Battery Commands ===");
      Serial.println("  bat <0-100>     - Set battery level");
      Serial.println("  bat demo on/off - Enable/disable demo animation");
      Serial.println("  bat status      - Show battery status");
      Serial.println("  bat charge on/off - Set charging status");
      Serial.println("  bat debug       - Show battery debug info");
      Serial.println("  bat test        - Run battery test animation");
      Serial.println("  bat dims        - Test fill dimensions");
      Serial.println("  bat help        - Show battery commands");
      Serial.println("=== Timer System Commands ===");
      Serial.println("  timer start/cancel - Timer control");
      Serial.println("  tlapse start       - T-Lapse control");
      Serial.println("  interval start     - Interval control");
      Serial.println("=== General Commands ===");
      Serial.println("  help            - Show this help");
      Serial.println("=== SIMPLIFIED NAVIGATION ===");
      Serial.println("  Main → Timer/T-Lapse/Interval/Settings");
      Serial.println("  Cards are no longer clickable (no detail pages)");
      Serial.println("  Use encoder to change values directly");
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
  
  // Show loading screen configuration
  if (LOADING_SCREEN_ENABLED) {
    DEBUG_PRINTF("Loading screen: ENABLED (%d ms duration)\n", LOADING_DURATION_MS);
  } else {
    DEBUG_PRINTLN("Loading screen: DISABLED (debug mode)");
  }
  
  // Show encoder improvements
  DEBUG_PRINTLN("=== ENCODER CONFIGURATION (3 SPEEDS) ===");
  DEBUG_PRINTF("Hysteresis: %s\n", ENCODER_HYSTERESIS_ENABLED ? "ENABLED" : "DISABLED");
  DEBUG_PRINTF("Smoothing: %s\n", ENCODER_SMOOTHING_ENABLED ? "ENABLED" : "DISABLED");
  DEBUG_PRINTLN("Speed thresholds:");
  DEBUG_PRINTF("  Fast: < %d ms (30s steps)\n", ENCODER_SPEED_FAST_MS);
  DEBUG_PRINTF("  Medium: < %d ms (10s steps)\n", ENCODER_SPEED_MEDIUM_MS);
  DEBUG_PRINTF("  Slow: >= %d ms (1s steps)\n", ENCODER_SPEED_MEDIUM_MS);
  DEBUG_PRINTLN("====================================");
  
  // Initialize application
  app_init();
  
  DEBUG_PRINTLN("=== Setup Complete ===");
  if (LOADING_SCREEN_ENABLED) {
    DEBUG_PRINTLN("Showing loading screen...");
    DEBUG_PRINTLN("Serial Commands available:");
    DEBUG_PRINTLN("- Type 'skip' to bypass loading screen");
    DEBUG_PRINTLN("- Type 'nav timer' then 'enc up' to test encoder");
    DEBUG_PRINTLN("- Type 'enc debug' to see encoder status");
    DEBUG_PRINTLN("- Type 'help' for full command list");
  } else {
    DEBUG_PRINTLN("Ready for user interaction!");
    DEBUG_PRINTLN("Serial Commands available - type 'help' for list");
    DEBUG_PRINTLN("Encoder test: 'nav timer' then 'enc up'/'enc up10'/'enc up30'");
    DEBUG_PRINTLN("Encoder debug: 'enc debug' to see current settings");
    print_battery_status();
  }
}

void loop() {
  // Handle serial commands for testing
  handle_serial_commands();
  
  // Main application loop
  app_loop();
}