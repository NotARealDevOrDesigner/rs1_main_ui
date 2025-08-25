/*
=============================================================================
ESP32-C6 Camera Control App - Main Application with Rotary Encoder Support
=============================================================================

File Structure:
- config.h         : Hardware configuration & constants (with encoder config)
- state_machine.h  : State management system (with value storage)
- hardware.h       : Hardware abstraction layer (with encoder support)
- ui.h            : User interface system with battery & loading screen
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
    else if (is_main_template_state(app_state.current_state) && !app_state.is_animating) {
      // Timer/T-Lapse pages: edit the currently visible card
      // IMPORTANT: Store current option to prevent unwanted changes
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
    // Could be used to toggle between coarse/fine adjustment
    // or enter/exit editing mode
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
  DEBUG_PRINTLN("- Loading screen with animations (7s)");
  DEBUG_PRINTLN("- Smart navigation system (no back loops)");
  DEBUG_PRINTLN("- Main page with 4 buttons & battery display");
  DEBUG_PRINTLN("- Template pages with swipe navigation & battery");
  DEBUG_PRINTLN("- Detail pages with dynamic content & battery");
  DEBUG_PRINTLN("- Popup modal system");
  DEBUG_PRINTLN("- State machine navigation");
  DEBUG_PRINTLN("- Battery management system");
  DEBUG_PRINTLN("- Rotary encoder value editing (3 SPEEDS)");
  DEBUG_PRINTF("- Loading screen: %s\n", LOADING_SCREEN_ENABLED ? "ENABLED" : "DISABLED");
  DEBUG_PRINTLN("========================");
  DEBUG_PRINTLN("Navigation Hierarchy:");
  DEBUG_PRINTLN("  MAIN");
  DEBUG_PRINTLN("  ├── TIMER → DETAIL");
  DEBUG_PRINTLN("  ├── TLAPSE → DETAIL");
  DEBUG_PRINTLN("  ├── INTERVAL → DETAIL");
  DEBUG_PRINTLN("  └── SETTINGS");
  DEBUG_PRINTLN("========================");
  DEBUG_PRINTLN("Encoder Functions (3 SPEEDS):");
  DEBUG_PRINTLN("- Turn encoder to change visible card values");
  DEBUG_PRINTLN("- Swipe cards to change which value is being edited");
  DEBUG_PRINTLN("- Values are saved globally and persist");
  DEBUG_PRINTLN("- 3 speeds: 1s, 10s, 30s steps");
  DEBUG_PRINTLN("- Hysteresis prevents speed jumping");
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
  
  // Handle rotary encoder input
  handle_encoder_input();
  
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
// SERIAL COMMANDS FOR TESTING
// =============================================================================
void handle_serial_commands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Battery commands an das Battery-System weiterleiten
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
      DEBUG_PRINTF("Interval - Interval: %s, Count: %s\n", 
                   format_time_value(interval_values.option1.seconds, interval_values.option1.format).c_str(),
                   format_time_value(interval_values.option2.seconds, interval_values.option2.format).c_str());
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
    // Navigation test commands
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
    else if (command == "nav detail") {
      if (is_main_template_state(app_state.current_state)) {
        DEBUG_PRINTLN("Navigating to DETAIL");
        app_state.detail_context = get_detail_context();
        change_state(STATE_DETAIL);
        show_current_page();
      } else {
        DEBUG_PRINTLN("Can only go to DETAIL from template pages");
      }
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
    else if (command == "help") {
      Serial.println("Available commands:");
      Serial.println("=== Loading Screen Commands ===");
      Serial.println("  loading skip    - Skip current loading screen");
      Serial.println("  loading status  - Show loading screen status");
      Serial.println("  skip            - Alias for loading skip");
      Serial.println("=== Navigation Commands ===");
      Serial.println("  nav test        - Test navigation system");
      Serial.println("  nav timer       - Go to timer page");
      Serial.println("  nav detail      - Go to detail page (from template)");
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
      Serial.println("=== General Commands ===");
      Serial.println("  help            - Show this help");
      Serial.println("=== Encoder Usage (3 SPEEDS) ===");
      Serial.println("  1. Go to Timer/T-Lapse/Interval page");
      Serial.println("  2. Turn encoder to change visible card value");
      Serial.println("  3. Swipe to other card, encoder controls that card");
      Serial.println("  4. Values are saved globally");
      Serial.println("  5. 3 speeds: Slow (1s), Medium (10s), Fast (30s)");
      Serial.println("  6. Hysteresis prevents jumping between speeds");
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
  
  // Show encoder improvements - CORRECTED FOR 3 SPEEDS
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
    // Initial battery status (only if not loading)
    print_battery_status();
  }
}

void loop() {
  // Handle serial commands for testing
  handle_serial_commands();
  
  // Main application loop
  app_loop();
}