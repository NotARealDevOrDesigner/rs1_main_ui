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
AppStateData app_state = {STATE_MAIN, STATE_MAIN, DETAIL_TIMER_OPTION1, 0, false, "Default Text"};

// Content data
PageContent timer_content = {"Timer", "Quick Timer", "Custom Timer", "05:00", "00:00"};
PageContent tlapse_content = {"Timelapse", "Standard", "Advanced", "02:00", "04:00"};
PageContent interval_content = {"Interval", "Short Burst", "Long Burst", "00:30", "01:00"};

// Forward declaration for UI
void show_current_page();

void state_machine_init() {
  app_state.current_state = STATE_MAIN;
  app_state.previous_state = STATE_MAIN;
  app_state.current_option = 0;
  app_state.is_animating = false;
  app_state.dynamic_text = "Welcome to Camera Control";
  
  DEBUG_PRINTLN("State machine initialized");
}

void change_state(AppState new_state) {
  app_state.previous_state = app_state.current_state;
  app_state.current_state = new_state;
  DEBUG_PRINTF("State changed: %d -> %d\n", app_state.previous_state, new_state);
}

void go_back() {
  change_state(app_state.previous_state);
  show_current_page();
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