/*
=============================================================================
state_machine.h - State Management System
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
  STATE_LOADING,    // New loading screen state
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
  unsigned long loading_start_time;  // Track when loading started
};

// =============================================================================
// GLOBAL STATE
// =============================================================================
extern AppStateData app_state;
extern PageContent timer_content;
extern PageContent tlapse_content;
extern PageContent interval_content;

// =============================================================================
// STATE MANAGEMENT FUNCTIONS
// =============================================================================
void state_machine_init();
void change_state(AppState new_state);
void go_back();
void check_loading_timeout();  // New function to check if loading should end

// Navigation hierarchy functions
AppState get_parent_state(AppState current_state);
bool is_main_template_state(AppState state);

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

// Global state instance - Start with loading screen if enabled
AppStateData app_state = {
  LOADING_SCREEN_ENABLED ? STATE_LOADING : STATE_MAIN, 
  STATE_LOADING, 
  DETAIL_TIMER_OPTION1, 
  0, 
  false, 
  "Loading...", 
  0
};

// Content data
PageContent timer_content = {"Timer", "Delay", "Release", "05:00", "00:00"};
PageContent tlapse_content = {"Timelapse", "Total", "Frames", "02:00", "04:00"};
PageContent interval_content = {"Interval", "Interval", "-", "00:30", "01:00"};

// Forward declaration for UI
void show_current_page();

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
  app_state.dynamic_text = LOADING_SCREEN_ENABLED ? "Loading..." : "Welcome to Camera Control";
  
  DEBUG_PRINTLN("State machine initialized");
}

void change_state(AppState new_state) {
  app_state.previous_state = app_state.current_state;
  app_state.current_state = new_state;
  DEBUG_PRINTF("State changed: %d -> %d\n", app_state.previous_state, new_state);
}

void go_back() {
  // Smart navigation: understand the page hierarchy
  AppState target_state = get_parent_state(app_state.current_state);
  
  DEBUG_PRINTF("Smart back: %d -> %d\n", app_state.current_state, target_state);
  
  change_state(target_state);
  show_current_page();
}

// Get the logical parent state for proper back navigation
AppState get_parent_state(AppState current_state) {
  switch (current_state) {
    case STATE_LOADING:
      return STATE_MAIN;  // Loading should always go to main
      
    case STATE_DETAIL:
      // Detail pages go back to their template page
      // Use the previous state if it's a template page, otherwise go to main
      if (is_main_template_state(app_state.previous_state)) {
        return app_state.previous_state;
      } else {
        return STATE_MAIN;  // Fallback to main if previous state is unclear
      }
      
    case STATE_TIMER:
    case STATE_TLAPSE:
    case STATE_INTERVAL:
      // Template pages always go back to main
      return STATE_MAIN;
      
    case STATE_SETTINGS:
      return STATE_MAIN;
      
    case STATE_MAIN:
    default:
      return STATE_MAIN;  // Main page stays at main
  }
}

// Check if a state is one of the main template states
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