/*
=============================================================================
state_machine.h - State Management System with Value Storage
=============================================================================
*/

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "config.h"

// =============================================================================
// STATE DEFINITIONS
// =============================================================================
enum AppState {
  STATE_LOADING,    
  STATE_MAIN,
  STATE_TIMER,
  STATE_TLAPSE, 
  STATE_INTERVAL,
  STATE_SETTINGS,
  STATE_DETAIL
};

enum DetailContext {
  DETAIL_TIMER_OPTION1,
  DETAIL_TIMER_OPTION2,
  DETAIL_TLAPSE_OPTION1,
  DETAIL_TLAPSE_OPTION2,
  DETAIL_INTERVAL_OPTION1,
  DETAIL_INTERVAL_OPTION2
};

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
// DATA STRUCTURES
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
  DetailContext detail_context;
  int current_option;
  bool is_animating;
  String dynamic_text;
  unsigned long loading_start_time;
  bool encoder_editing_mode;    // New: Track if we're in editing mode
};

// =============================================================================
// GLOBAL STATE AND VALUES
// =============================================================================
extern AppStateData app_state;
extern PageContent timer_content;
extern PageContent tlapse_content;
extern PageContent interval_content;

// New: Value storage for all pages
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
// DATA MANAGEMENT FUNCTIONS
// =============================================================================
void update_dynamic_text(String new_text);
PageContent get_current_content();
String get_detail_heading();
DetailContext get_detail_context();

// =============================================================================
// STATE MACHINE IMPLEMENTATION
// =============================================================================

// Global state instance
AppStateData app_state = {
  LOADING_SCREEN_ENABLED ? STATE_LOADING : STATE_MAIN, 
  STATE_LOADING, 
  DETAIL_TIMER_OPTION1, 
  0, 
  false, 
  "Loading...", 
  0,
  false  // encoder_editing_mode
};

// Default page values - these will be overridden by values_init()
PageValues timer_values;
PageValues tlapse_values;
PageValues interval_values;

// Content data - will be updated from values
PageContent timer_content = {"Timer", "Delay", "Release", "00:00", "00:00"};
PageContent tlapse_content = {"Timelapse", "Total", "Frames", "00:00", "0"};  // CHANGED: "Total" statt "Total Time"
PageContent interval_content = {"Interval", "Interval", "", "00:00", ""}; // Keine zweite Option

// Forward declaration for UI
void show_current_page();
void update_template_content(PageContent content);

// =============================================================================
// VALUE MANAGEMENT IMPLEMENTATION
// =============================================================================
void values_init() {
  DEBUG_PRINTLN("Initializing value storage system...");
  
  // Initialize Timer values - beide starten bei 00:00
  timer_values.page_title = "Timer";
  timer_values.option1_label = "Delay";
  timer_values.option2_label = "Release";
  timer_values.option1 = {0, VALUE_FORMAT_MM_SS, 0, 3599, VALUE_INCREMENT_SMALL};  // 00:00
  timer_values.option2 = {0, VALUE_FORMAT_MM_SS, 0, 3599, VALUE_INCREMENT_SMALL};  // 00:00
  
  // Initialize Timelapse values - GEÄNDERT: Total Time verwendet jetzt auch adaptive Schritte
  tlapse_values.page_title = "Timelapse";
  tlapse_values.option1_label = "Total";
  tlapse_values.option2_label = "Frames";
  tlapse_values.option1 = {0, VALUE_FORMAT_MM_SS, 0, 3599, VALUE_INCREMENT_SMALL}; // 00:00 - GEÄNDERT: war VALUE_INCREMENT_LARGE
  tlapse_values.option2 = {0, VALUE_FORMAT_COUNT, 0, 0, 1};                       // 0 frames (max wird dynamisch gesetzt)
  
  // Initialize Interval values - nur Timer, keine zweite Option
  interval_values.page_title = "Interval";
  interval_values.option1_label = "Interval";
  interval_values.option2_label = "";  // Leer, wird nicht verwendet
  interval_values.option1 = {0, VALUE_FORMAT_MM_SS, 0, 3599, VALUE_INCREMENT_SMALL};  // 00:00
  interval_values.option2 = {0, VALUE_FORMAT_COUNT, 0, 0, 1};                        // Nicht verwendet
  
  // Update page content from these values
  update_page_content_from_values(STATE_TIMER);
  update_page_content_from_values(STATE_TLAPSE);  
  update_page_content_from_values(STATE_INTERVAL);
  
  DEBUG_PRINTLN("Value storage initialized - all values start at 00:00");
  DEBUG_PRINTLN("T-Lapse Total Time now uses adaptive encoder steps (1s/10s/30s)");
}

String format_time_value(uint32_t seconds, uint8_t format) {
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
  
  // Calculate new value
  int64_t new_value = (int64_t)target_option->seconds + (delta * target_option->increment);
  
  // Apply bounds checking
  if (new_value < target_option->min_value) {
    new_value = target_option->min_value;
  } else if (new_value > target_option->max_value) {
    new_value = target_option->max_value;
  }
  
  // Special logic for T-Lapse frames: max 1 frame per second
  if (page == STATE_TLAPSE && option == 1) {
    // Frame count is limited by total time (max 1 frame per second)
    uint32_t total_time_seconds = values->option1.seconds;
    if (new_value > total_time_seconds) {
      new_value = total_time_seconds;
      DEBUG_PRINTF("Frame count limited to %d (max 1 frame per second)\n", (int)new_value);
    }
  }
  
  target_option->seconds = (uint32_t)new_value;
  
  // If we changed T-Lapse total time, we might need to adjust frame count
  if (page == STATE_TLAPSE && option == 0) {
    uint32_t total_time = target_option->seconds;
    if (values->option2.seconds > total_time) {
      values->option2.seconds = total_time;
      DEBUG_PRINTF("Frame count auto-adjusted to %d (max 1 frame per second)\n", total_time);
    }
    // Update the maximum allowed frames dynamically
    values->option2.max_value = total_time;
  }
  
  DEBUG_PRINTF("Updated %s option %d: %s\n", 
               values->page_title.c_str(), 
               option + 1,
               format_time_value(target_option->seconds, target_option->format).c_str());
  
  // Update the page content to reflect new values
  update_page_content_from_values(page);
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
    content->option2_text = "";   // Interval hat keine zweite Option
    content->option2_time = "";   // Leer lassen
  } else {
    content->option2_time = format_time_value(values->option2.seconds, values->option2.format);
  }
  
  // Note: UI updates will be handled by the UI system when values change
  // We don't call update_template_content or update_interval_content directly here
  // because that would create circular dependencies
}

// =============================================================================
// EXISTING FUNCTIONS (unchanged)
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
      
    case STATE_DETAIL:
      if (is_main_template_state(app_state.previous_state)) {
        return app_state.previous_state;
      } else {
        return STATE_MAIN;
      }
      
    case STATE_TIMER:
    case STATE_TLAPSE:
    case STATE_INTERVAL:
      return STATE_MAIN;
      
    case STATE_SETTINGS:
      return STATE_MAIN;
      
    case STATE_MAIN:
    default:
      return STATE_MAIN;
  }
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

String get_detail_heading() {
  PageContent content = get_current_content();
  if (app_state.current_option == 0) {
    return content.option1_text;
  } else {
    return content.option2_text;
  }
}

DetailContext get_detail_context() {
  if (app_state.current_state == STATE_TIMER) {
    return app_state.current_option == 0 ? DETAIL_TIMER_OPTION1 : DETAIL_TIMER_OPTION2;
  } else if (app_state.current_state == STATE_TLAPSE) {
    return app_state.current_option == 0 ? DETAIL_TLAPSE_OPTION1 : DETAIL_TLAPSE_OPTION2;
  } else if (app_state.current_state == STATE_INTERVAL) {
    return app_state.current_option == 0 ? DETAIL_INTERVAL_OPTION1 : DETAIL_INTERVAL_OPTION2;
  }
  return DETAIL_TIMER_OPTION1;
}

#endif // STATE_MACHINE_H