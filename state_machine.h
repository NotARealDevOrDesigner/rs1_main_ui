/*
=============================================================================
state_machine.h - Simplified State Management System (No Detail States)
=============================================================================
*/

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "config.h"


// Forward declarations für Bluetooth (falls needed)
extern void save_app_state();
// Forward declarations timer saving system
extern bool settings_initialized;
extern void save_timer_values();
// =============================================================================
// STATE DEFINITIONS - SIMPLIFIED
// =============================================================================
enum AppState {
  STATE_LOADING,    
  STATE_MAIN,
  STATE_TIMER,
  STATE_TLAPSE, 
  STATE_INTERVAL,
  STATE_SETTINGS,
  STATE_WIRE_SETTINGS
};

// REMOVED: DetailContext enum - no longer needed

// =============================================================================
// VALUE STORAGE STRUCTURES
// =============================================================================
struct OptionValue {
  uint32_t seconds;         // Store time in seconds
  uint8_t format;           // Display format (MM:SS, SS, COUNT)
  uint32_t min_value;       // Minimum allowed value
  uint32_t max_value;       // Maximum allowed value
  uint16_t increment;       // Increment step size
};

struct PageValues {
  OptionValue option1;
  OptionValue option2;
  String option1_label;
  String option2_label;
  String page_title;
};

// =============================================================================
// DATA STRUCTURES - SIMPLIFIED
// =============================================================================
struct PageContent {
  String heading;
  String option1_text;
  String option2_text;
  String option1_time;
  String option2_time;
};

struct AppStateData {
  AppState current_state;
  AppState previous_state;
  int current_option;           // For encoder editing
  bool is_animating;
  String dynamic_text;
  unsigned long loading_start_time;
  bool encoder_editing_mode;    // Track if we're in editing mode
  // REMOVED: DetailContext detail_context - no longer needed
  bool led_enabled;
  bool bluetooth_enabled;
  int servo_wire_percentage;
};

// =============================================================================
// GLOBAL STATE AND VALUES
// =============================================================================
extern AppStateData app_state;
extern PageContent timer_content;
extern PageContent tlapse_content;
extern PageContent interval_content;

// Value storage for all pages
extern PageValues timer_values;
extern PageValues tlapse_values;
extern PageValues interval_values;

// =============================================================================
// STATE MANAGEMENT FUNCTIONS
// =============================================================================
void state_machine_init();
void change_state(AppState new_state);
void go_back();
void check_loading_timeout();

// Navigation hierarchy functions
AppState get_parent_state(AppState current_state);
bool is_main_template_state(AppState state);

// =============================================================================
// VALUE MANAGEMENT FUNCTIONS
// =============================================================================
void values_init();
String format_time_value(uint32_t centiseconds, uint8_t format);
void update_option_value(AppState page, int option, int32_t delta);
uint32_t get_option_value(AppState page, int option);
PageValues* get_current_page_values();
void update_page_content_from_values(AppState page);

// =============================================================================
// DATA MANAGEMENT FUNCTIONS - SIMPLIFIED
// =============================================================================
void update_dynamic_text(String new_text);
PageContent get_current_content();
// REMOVED: String get_detail_heading();
// REMOVED: DetailContext get_detail_context();

// =============================================================================
// STATE MACHINE IMPLEMENTATION
// =============================================================================

// Global state instance - SIMPLIFIED
AppStateData app_state = {
  LOADING_SCREEN_ENABLED ? STATE_LOADING : STATE_MAIN, 
  STATE_LOADING, 
  0,      // current_option
  false,  // is_animating
  "Loading...", 
  0,      // loading_start_time
  false,   // encoder_editing_mode
  true,   // DIESE 3 ZEILEN HINZUFÜGEN
  false,  
  100     
};

// Value storage - unchanged
PageValues timer_values;
PageValues tlapse_values;
PageValues interval_values;

// Content data - will be updated from values
PageContent timer_content = {"Timer", "Delay", "Release", "00:00", "00:00"};
PageContent tlapse_content = {"Timelapse", "Total", "Frames", "00:00", "0"};
PageContent interval_content = {"Interval", "Interval", "", "00:00", ""}; // No second option

// Forward declaration for UI
void show_current_page();
void update_template_content(PageContent content);

// =============================================================================
// VALUE MANAGEMENT IMPLEMENTATION
// =============================================================================
void values_init() {
  DEBUG_PRINTLN("Initializing value storage system...");
  
  // NOTE: Settings werden von settings_init() geladen
  // Hier nur Fallback-Initialisierung falls keine Settings existieren
  
  // Check if values were loaded from settings, if not initialize defaults
  bool need_timer_init = (timer_values.page_title.isEmpty());
  bool need_tlapse_init = (tlapse_values.page_title.isEmpty());
  bool need_interval_init = (interval_values.page_title.isEmpty());
  
  if (need_timer_init) {
    DEBUG_PRINTLN("Initializing Timer defaults (not loaded from settings)");
    timer_values.page_title = "Timer";
    timer_values.option1_label = "Delay";
    timer_values.option2_label = "Release";
    timer_values.option1 = {0, VALUE_FORMAT_MM_SS, 0, 3599, VALUE_INCREMENT_SMALL};
    timer_values.option2 = {0, VALUE_FORMAT_MM_SS, 0, 3599, VALUE_INCREMENT_SMALL};
  }
  
  if (need_tlapse_init) {
    DEBUG_PRINTLN("Initializing T-Lapse defaults (not loaded from settings)");
    tlapse_values.page_title = "Timelapse";
    tlapse_values.option1_label = "Total";
    tlapse_values.option2_label = "Frames";
    tlapse_values.option1 = {0, VALUE_FORMAT_MM_SS, 0, 3599, VALUE_INCREMENT_SMALL};
    tlapse_values.option2 = {0, VALUE_FORMAT_COUNT, 0, 0, 1};
  }
  
  if (need_interval_init) {
    DEBUG_PRINTLN("Initializing Interval defaults (not loaded from settings)");
    interval_values.page_title = "Interval";
    interval_values.option1_label = "Interval";
    interval_values.option2_label = "";  // Empty, not used
    interval_values.option1 = {0, VALUE_FORMAT_MM_SS, 0, 3599, VALUE_INCREMENT_SMALL};
    interval_values.option2 = {0, VALUE_FORMAT_COUNT, 0, 0, 1}; // Not used
  }
  
  // Always update page content from loaded/initialized values
  update_page_content_from_values(STATE_TIMER);
  update_page_content_from_values(STATE_TLAPSE);
  update_page_content_from_values(STATE_INTERVAL);
  
  DEBUG_PRINTLN("Value storage initialized - all pages ready");
}

String format_time_value(uint32_t seconds, uint8_t format) {
  // Handle trigger mode for timer release
  if (seconds == 4294967295 || (int32_t)seconds == -1) {
    return "SHOT";
  }
  
  switch (format) {
    case VALUE_FORMAT_MM_SS: {
      uint16_t minutes = seconds / 60;
      uint16_t secs = seconds % 60;
      return (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
             (secs < 10 ? "0" : "") + String(secs);
    }
    case VALUE_FORMAT_SS: {
      return (seconds < 10 ? "0" : "") + String(seconds);
    }
    case VALUE_FORMAT_COUNT:
    default:
      return String(seconds);
  }
}

PageValues* get_current_page_values() {
  switch (app_state.current_state) {
    case STATE_TIMER: return &timer_values;
    case STATE_TLAPSE: return &tlapse_values;
    case STATE_INTERVAL: return &interval_values;
    default: return &timer_values;
  }
}

uint32_t get_option_value(AppState page, int option) {
  PageValues* values = nullptr;
  switch (page) {
    case STATE_TIMER: values = &timer_values; break;
    case STATE_TLAPSE: values = &tlapse_values; break;
    case STATE_INTERVAL: values = &interval_values; break;
    default: return 0;
  }
  
  return (option == 0) ? values->option1.seconds : values->option2.seconds;
}

void update_option_value(AppState page, int option, int32_t delta) {
  PageValues* values = nullptr;
  switch (page) {
    case STATE_TIMER: values = &timer_values; break;
    case STATE_TLAPSE: values = &tlapse_values; break;
    case STATE_INTERVAL: values = &interval_values; break;
    default: return;
  }
  
  OptionValue* target_option = (option == 0) ? &values->option1 : &values->option2;
  
  // Special handling: Timer Release (Option 1) with Trigger Mode
  if (page == STATE_TIMER && option == 1) { // Release time
    int32_t current_value = (int32_t)target_option->seconds;
    
    // Handle Trigger Mode (-1)
    if (current_value == -1) {
      if (delta > 0) {
        // From Trigger up = to 00:00
        target_option->seconds = 0;
        DEBUG_PRINTLN("Switched from Trigger to 00:00");
      } else {
        // From Trigger down = BLOCKED
        DEBUG_PRINTLN("Blocked: Cannot scroll down from Trigger mode");
        return;
      }
    }
    // Handle normal values (>= 0)
    else if (current_value >= 0) {
      int32_t new_value = current_value + (delta * target_option->increment);
      
      if (new_value < 0) {
        // Scroll below 0 = to Trigger
        target_option->seconds = (uint32_t)-1;
        DEBUG_PRINTLN("Switched from 00:00 to Trigger mode");
      } else if (new_value > target_option->max_value) {
        target_option->seconds = target_option->max_value;
      } else {
        target_option->seconds = (uint32_t)new_value;
      }
    } else {
      DEBUG_PRINTF("ERROR: Invalid timer release value: %d, resetting to 0\n", current_value);
      target_option->seconds = 0;
    }
  } 
  // Normal handling for all other options
  else {
    int64_t new_value = (int64_t)target_option->seconds + (delta * target_option->increment);
    
    // Apply bounds checking
    if (new_value < target_option->min_value) {
      new_value = target_option->min_value;
    } else if (new_value > target_option->max_value) {
      new_value = target_option->max_value;
    }
    
    target_option->seconds = (uint32_t)new_value;
  }
  
  // Special logic for T-Lapse frames: max 1 frame per second
  if (page == STATE_TLAPSE && option == 1) {
    uint32_t total_time_seconds = values->option1.seconds;
    if (values->option2.seconds > total_time_seconds) {
      values->option2.seconds = total_time_seconds;
      DEBUG_PRINTF("Frame count auto-adjusted to %d (max 1 frame per second)\n", total_time_seconds);
    }
    values->option2.max_value = total_time_seconds;
  }
  
  // If we changed T-Lapse total time, adjust frame count
  if (page == STATE_TLAPSE && option == 0) {
    uint32_t total_time = target_option->seconds;
    if (values->option2.seconds > total_time) {
      values->option2.seconds = total_time;
      DEBUG_PRINTF("Frame count auto-adjusted to %d (max 1 frame per second)\n", total_time);
    }
    values->option2.max_value = total_time;
  }
  
  DEBUG_PRINTF("Updated %s option %d: %s\n", 
               values->page_title.c_str(), 
               option + 1,
               format_time_value(target_option->seconds, target_option->format).c_str());
  
  update_page_content_from_values(page);

  // Auto-save after value change
  if (settings_initialized) {
    save_timer_values(); // Only save timer values for performance
  }

}

void update_page_content_from_values(AppState page) {
  PageValues* values = nullptr;
  PageContent* content = nullptr;
  
  switch (page) {
    case STATE_TIMER:
      values = &timer_values;
      content = &timer_content;
      break;
    case STATE_TLAPSE:
      values = &tlapse_values;
      content = &tlapse_content;
      break;
    case STATE_INTERVAL:
      values = &interval_values;
      content = &interval_content;
      break;
    default:
      return;
  }
  
  content->heading = values->page_title;
  content->option1_text = values->option1_label;
  content->option2_text = values->option2_label;
  content->option1_time = format_time_value(values->option1.seconds, values->option1.format);
  
  // Special handling for different pages
  if (page == STATE_INTERVAL) {
    content->option2_text = "";   // Interval has no second option
    content->option2_time = "";   // Empty
  } else {
    content->option2_time = format_time_value(values->option2.seconds, values->option2.format);
  }
}

// =============================================================================
// STATE MANAGEMENT IMPLEMENTATION - SIMPLIFIED
// =============================================================================
void state_machine_init() {
  if (LOADING_SCREEN_ENABLED) {
    app_state.current_state = STATE_LOADING;
    app_state.loading_start_time = millis();
    DEBUG_PRINTLN("Starting with loading screen for " + String(LOADING_DURATION_MS) + "ms");
  } else {
    app_state.current_state = STATE_MAIN;
    DEBUG_PRINTLN("Loading screen disabled - starting with main page");
  }
  
  app_state.previous_state = STATE_LOADING;
  app_state.current_option = 0;
  app_state.is_animating = false;
  app_state.encoder_editing_mode = false;
  app_state.dynamic_text = LOADING_SCREEN_ENABLED ? "Loading..." : "Welcome to Camera Control";
  
  // Initialize value storage
  values_init();
  
  DEBUG_PRINTLN("State machine initialized");
}

void change_state(AppState new_state) {
  app_state.previous_state = app_state.current_state;
  app_state.current_state = new_state;
  app_state.encoder_editing_mode = false; // Exit editing mode on state change
  DEBUG_PRINTF("State changed: %d -> %d\n", app_state.previous_state, new_state);
}

void go_back() {
  AppState target_state = get_parent_state(app_state.current_state);
  DEBUG_PRINTF("Smart back: %d -> %d\n", app_state.current_state, target_state);
  change_state(target_state);
  show_current_page();
}

AppState get_parent_state(AppState current_state) {
  switch (current_state) {
    case STATE_LOADING:
      return STATE_MAIN;
      
    // All template states go back to main
    case STATE_TIMER:
    case STATE_TLAPSE:
    case STATE_INTERVAL:
    case STATE_SETTINGS:
      return STATE_MAIN;
    
    case STATE_WIRE_SETTINGS:  
      return STATE_SETTINGS;

    case STATE_MAIN:
    default:
      return STATE_MAIN;
  }
  // REMOVED: STATE_DETAIL handling
}

bool is_main_template_state(AppState state) {
  return (state == STATE_TIMER || state == STATE_TLAPSE || state == STATE_INTERVAL);
}

void check_loading_timeout() {
  if (app_state.current_state == STATE_LOADING && LOADING_SCREEN_ENABLED) {
    unsigned long elapsed = millis() - app_state.loading_start_time;
    if (elapsed >= LOADING_DURATION_MS) {
      DEBUG_PRINTLN("Loading complete - transitioning to main page");
      change_state(STATE_MAIN);
      show_current_page();
    }
  }
}

void update_dynamic_text(String new_text) {
  app_state.dynamic_text = new_text;
  DEBUG_PRINTLN("Dynamic text updated: " + new_text);
}

PageContent get_current_content() {
  switch (app_state.current_state) {
    case STATE_TIMER: return timer_content;
    case STATE_TLAPSE: return tlapse_content;
    case STATE_INTERVAL: return interval_content;
    default: return timer_content;
  }
}

// REMOVED: get_detail_heading() - no longer needed
// REMOVED: get_detail_context() - no longer needed

#endif // STATE_MACHINE_H