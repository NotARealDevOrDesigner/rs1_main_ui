/*
=============================================================================
timer_system.h - Fixed Timer/T-Lapse/Interval System with Servo Completion
=============================================================================
*/

#ifndef TIMER_SYSTEM_H
#define TIMER_SYSTEM_H

#include <ESP32Servo.h>
#include "config.h"
#include "state_machine.h"

// =============================================================================
// TIMER SYSTEM CONFIGURATION
// =============================================================================
#define SERVO_PIN 9
// Servo initialization tracking
bool servo_initialization_complete = false;
unsigned long servo_init_start_time = 0;
#define SERVO_INIT_TIME_MS 500  // Time needed for servo to reach initial position

// Timer States - ERWEITERT für Servo-Completion
enum TimerExecutionMode {
  TIMER_EXEC_MODE,
  TLAPSE_EXEC_MODE,
  INTERVAL_EXEC_MODE
};

enum TimerExecutionState {
  TIMER_IDLE,
  TIMER_DELAY_RUNNING,
  TIMER_RELEASE_RUNNING,
  TLAPSE_RUNNING,
  INTERVAL_RUNNING,
  TIMER_COMPLETING_SERVO,    // NEU: Warten auf Servo-Completion
  TLAPSE_COMPLETING_SERVO,   // NEU: Warten auf Servo-Completion
  INTERVAL_COMPLETING_SERVO  // NEU: Warten auf Servo-Completion (falls benötigt)
};

// Timer Runtime Data - ERWEITERT
struct TimerRuntime {
  TimerExecutionMode mode;
  TimerExecutionState state;
  unsigned long startTime;
  unsigned long currentPhaseStartTime;
  int frameCount;
  int totalDelayTime;     // in seconds
  int totalReleaseTime;   // in seconds
  int totalTime;          // for T-Lapse in seconds
  int totalFrames;        // for T-Lapse
  int intervalTime;       // for Interval in seconds
  float frameInterval;    // calculated interval between frames for T-Lapse
  
  // NEU: Servo completion tracking
  bool waiting_for_servo_completion;
  unsigned long servo_completion_timeout;
  bool logic_completed;   // Logic is done, but servo still moving
};

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================
extern Servo cameraServo;
extern TimerRuntime runtime;

extern int servoStartPosition;      
extern int servoEndPosition;        // This becomes the "working stop position"
extern int servoAbsoluteMaxPosition; // NEW: The true 100% reference point
extern float servoActivationTime;

bool servo_is_activating = false;
unsigned long servo_activation_start_time = 0;

// Servo Settings (adjustable)
extern int servoStartPosition;      
extern int servoEndPosition;        
extern float servoActivationTime;   

// UI Objects for Overlays
extern lv_obj_t *timer_overlay;
extern lv_obj_t *timer_overlay_time_label;
extern lv_obj_t *timer_overlay_time_remaining_label;
extern lv_obj_t *timer_overlay_cancel_btn;

extern lv_obj_t *tlapse_overlay;
extern lv_obj_t *tlapse_overlay_time_label;
extern lv_obj_t *tlapse_overlay_frame_counter;
extern lv_obj_t *tlapse_overlay_cancel_btn;

extern lv_obj_t *interval_overlay;
extern lv_obj_t *interval_overlay_time_label;
extern lv_obj_t *interval_overlay_frame_counter;
extern lv_obj_t *interval_overlay_cancel_btn;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Bluetooth
extern void send_ble_response(String response);

// System Functions
void timer_system_init();
void timer_system_update();

// Servo Control Functions
void servo_init();
void servo_activate();
void servo_move_to_position(int position);
bool is_servo_completion_needed();

// Timer Execution Functions
void start_timer_execution();
void start_tlapse_execution();
void start_interval_execution();
void cancel_timer_execution();

// Timer Update Functions
void update_timer_execution();
void update_tlapse_execution();
void update_interval_execution();

// Overlay Management Functions
void create_timer_overlays();
void show_timer_overlay();
void show_tlapse_overlay();
void show_interval_overlay();
void hide_timer_overlays();
void update_timer_overlay_display();
void update_tlapse_overlay_display();
void update_interval_overlay_display();

// Utility Functions
String format_countdown_time(int totalSeconds, int elapsedSeconds, bool showBoth = false);

// Event Callbacks
void timer_cancel_cb(lv_event_t *e);
void tlapse_cancel_cb(lv_event_t *e);
void interval_cancel_cb(lv_event_t *e);

// =============================================================================
// IMPLEMENTATION
// =============================================================================

// Global Variables Implementation
Servo cameraServo;
TimerRuntime runtime;

// Servo Settings (adjustable)
int servoStartPosition = 0;      
int servoEndPosition = 90;       // Working stop position
int servoAbsoluteMaxPosition = SERVO_ABSOLUTE_MAX_POSITION; // True 100% reference
float servoActivationTime = 0.6;

// UI Objects Implementation
lv_obj_t *timer_overlay = nullptr;
lv_obj_t *timer_overlay_time_label = nullptr;
lv_obj_t *timer_overlay_time_remaining_label = nullptr;
lv_obj_t *timer_overlay_cancel_btn = nullptr;

lv_obj_t *tlapse_overlay = nullptr;
lv_obj_t *tlapse_overlay_time_label = nullptr;
lv_obj_t *tlapse_overlay_frame_counter = nullptr;
lv_obj_t *tlapse_overlay_cancel_btn = nullptr;

lv_obj_t *interval_overlay = nullptr;
lv_obj_t *interval_overlay_time_label = nullptr;
lv_obj_t *interval_overlay_frame_counter = nullptr;
lv_obj_t *interval_overlay_cancel_btn = nullptr;

// =============================================================================
// SERVO FUNCTIONS - ERWEITERT
// =============================================================================
void servo_init() {
  DEBUG_PRINTLN("Initializing servo...");
  cameraServo.attach(SERVO_PIN);
  
  // Initialize absolute maximum (this should never change)
  servoAbsoluteMaxPosition = SERVO_ABSOLUTE_MAX_POSITION;
  
  // WICHTIG: Set initial working stop based on LOADED percentage from settings
  int servo_range = servoAbsoluteMaxPosition - servoStartPosition;
  servoEndPosition = servoStartPosition + (servo_range * app_state.servo_wire_percentage / 100);
  
  servo_move_to_position(servoStartPosition);
  
  DEBUG_PRINTF("Servo initialized - Start: %d°, Working Stop: %d° (from %d%% setting), Absolute Max: %d°\n", 
               servoStartPosition, servoEndPosition, app_state.servo_wire_percentage, servoAbsoluteMaxPosition);
}

void servo_move_to_position(int position) {
  position = constrain(position, 0, 180);
  cameraServo.write(position);
  DEBUG_PRINTF("Servo moved to %dÂ°\n", position);
}

void servo_activate() {
  if (!servo_is_activating) {
    servo_is_activating = true;
    servo_activation_start_time = millis();
    servo_move_to_position(servoEndPosition);
    DEBUG_PRINTF("Servo activation started: %dÂ° -> %dÂ° for %.1fs\n", 
                 servoStartPosition, servoEndPosition, servoActivationTime);
  }
}

void servo_update() {
  if (servo_is_activating) {
    unsigned long elapsed = millis() - servo_activation_start_time;
    if (elapsed >= (servoActivationTime * 1000)) {
      servo_move_to_position(servoStartPosition);
      servo_is_activating = false;
      DEBUG_PRINTLN("Servo activation complete - returned to start position");
    }
  }
}

// NEU: Check if we need to wait for servo completion
bool is_servo_completion_needed() {
  return servo_is_activating || runtime.waiting_for_servo_completion;
}

// =============================================================================
// TIMER SYSTEM FUNCTIONS - ERWEITERT
// =============================================================================
void timer_system_init() {
  DEBUG_PRINTLN("Initializing timer system...");
  
  servo_init();
  
  // Initialize servo tracking
  servo_initialization_complete = false;
  servo_init_start_time = 0;
  
  // Initialize runtime data - ERWEITERT
  runtime.mode = TIMER_EXEC_MODE;
  runtime.state = TIMER_IDLE;
  runtime.startTime = 0;
  runtime.currentPhaseStartTime = 0;
  runtime.frameCount = 0;
  runtime.totalDelayTime = 0;
  runtime.totalReleaseTime = 0;
  runtime.totalTime = 0;
  runtime.totalFrames = 0;
  runtime.intervalTime = 0;
  runtime.frameInterval = 0.0;
  
  // NEU: Servo completion tracking
  runtime.waiting_for_servo_completion = false;
  runtime.servo_completion_timeout = 0;
  runtime.logic_completed = false;
  
  create_timer_overlays();
  
  DEBUG_PRINTLN("Timer system initialized successfully!");
}

void timer_system_update() {
  // Handle servo initialization (non-blocking)
  if (!servo_initialization_complete) {
    if (servo_init_start_time == 0) {
      servo_init_start_time = millis();
    }
    
    if (millis() - servo_init_start_time >= SERVO_INIT_TIME_MS) {
      servo_initialization_complete = true;
      DEBUG_PRINTLN("Servo initialization complete");
    }
    
    // Don't process timer functions until servo is ready
    return;
  }
  // Update non-blocking servo first
  servo_update();
  
  if (runtime.state == TIMER_IDLE) return;
  
  // NEU: Handle completion states - wait for servo to finish
  if (runtime.state == TIMER_COMPLETING_SERVO || 
      runtime.state == TLAPSE_COMPLETING_SERVO || 
      runtime.state == INTERVAL_COMPLETING_SERVO) {
    
    // Check if servo is done OR timeout reached
    if (!servo_is_activating || millis() > runtime.servo_completion_timeout) {
      DEBUG_PRINTLN("Servo completion phase finished");
      runtime.state = TIMER_IDLE;
      runtime.waiting_for_servo_completion = false;
      runtime.logic_completed = false;
      hide_timer_overlays();
      show_current_page(); // Return to previous UI state
      return;
    }
    
    // Continue updating display during completion
    switch (runtime.mode) {
      case TIMER_EXEC_MODE:
        update_timer_overlay_display();
        break;
      case TLAPSE_EXEC_MODE:
        update_tlapse_overlay_display();
        break;
      case INTERVAL_EXEC_MODE:
        update_interval_overlay_display();
        break;
    }
    return;
  }
  
  // Normal execution states
  switch (runtime.mode) {
    case TIMER_EXEC_MODE:
      update_timer_execution();
      break;
    case TLAPSE_EXEC_MODE:
      update_tlapse_execution();
      break;
    case INTERVAL_EXEC_MODE:
      update_interval_execution();
      break;
  }
}

// =============================================================================
// TIMER EXECUTION FUNCTIONS
// =============================================================================
void start_timer_execution() {

  if (!servo_initialization_complete) {
    DEBUG_PRINTLN("Timer start blocked - servo still initializing");
    return;
  }

  DEBUG_PRINTLN("Starting Timer execution...");
  
  runtime.totalDelayTime = get_option_value(STATE_TIMER, 0);    
  uint32_t releaseValue = get_option_value(STATE_TIMER, 1);     
  
  bool isTriggerMode = (releaseValue == 4294967295 || (int32_t)releaseValue == -1);
  
  if (isTriggerMode) {
    runtime.totalReleaseTime = 0; 
    DEBUG_PRINTLN("Timer in TRIGGER mode - single activation after delay");
  } else {
    runtime.totalReleaseTime = releaseValue;
  }
  
  runtime.mode = TIMER_EXEC_MODE;
  runtime.state = TIMER_DELAY_RUNNING;
  runtime.startTime = millis();
  runtime.currentPhaseStartTime = millis();
  runtime.frameCount = 0;
  runtime.logic_completed = false;
  
  servo_move_to_position(servoStartPosition);
  show_timer_overlay();
  
  if (isTriggerMode) {
    DEBUG_PRINTF("Timer started: Delay %ds, Mode: TRIGGER\n", runtime.totalDelayTime);
  } else {
    DEBUG_PRINTF("Timer started: Delay %ds, Release %ds\n", 
                 runtime.totalDelayTime, runtime.totalReleaseTime);
  }
}

void start_tlapse_execution() {
  DEBUG_PRINTLN("Starting T-Lapse execution...");
  
  runtime.totalTime = get_option_value(STATE_TLAPSE, 0);    
  runtime.totalFrames = get_option_value(STATE_TLAPSE, 1);  
  
  runtime.mode = TLAPSE_EXEC_MODE;
  runtime.state = TLAPSE_RUNNING;
  runtime.startTime = millis();
  runtime.currentPhaseStartTime = millis();
  runtime.frameCount = 0;
  runtime.logic_completed = false;
  
  if (runtime.totalFrames > 0) {
    runtime.frameInterval = (float)runtime.totalTime / runtime.totalFrames;
  } else {
    runtime.frameInterval = 1.0; 
  }
  
  servo_move_to_position(servoStartPosition);
  show_tlapse_overlay();
  
  DEBUG_PRINTF("T-Lapse started: %ds total, %d frames, %.2fs interval\n", 
               runtime.totalTime, runtime.totalFrames, runtime.frameInterval);
}

void start_interval_execution() {
  DEBUG_PRINTLN("Starting Interval execution...");
  
  runtime.intervalTime = get_option_value(STATE_INTERVAL, 0);  
  
  runtime.mode = INTERVAL_EXEC_MODE;
  runtime.state = INTERVAL_RUNNING;
  runtime.startTime = millis();
  runtime.currentPhaseStartTime = millis();
  runtime.frameCount = 0;
  runtime.logic_completed = false;
  
  servo_move_to_position(servoStartPosition);
  show_interval_overlay();
  
  DEBUG_PRINTF("Interval started: %ds interval\n", runtime.intervalTime);
}

void cancel_timer_execution() {
  DEBUG_PRINTLN("Timer execution cancelled");
  
  runtime.state = TIMER_IDLE;
  runtime.frameCount = 0;
  runtime.waiting_for_servo_completion = false;
  runtime.logic_completed = false;
  
  servo_move_to_position(servoStartPosition);
  hide_timer_overlays();
}

// =============================================================================
// TIMER UPDATE FUNCTIONS - GEÄNDERT für Completion Handling  
// =============================================================================
void update_timer_execution() {
  unsigned long currentTime = millis();
  unsigned long elapsedTotal = (currentTime - runtime.startTime) / 1000;
  unsigned long elapsedPhase = (currentTime - runtime.currentPhaseStartTime) / 1000;
  
  switch (runtime.state) {
    case TIMER_DELAY_RUNNING:
      if (elapsedPhase >= runtime.totalDelayTime) {
        if (runtime.totalReleaseTime == 0) {
          // TRIGGER MODE: Start servo and enter completion state
          servo_activate(); 
          runtime.frameCount = 1;
          runtime.logic_completed = true;
          runtime.waiting_for_servo_completion = true;
          runtime.servo_completion_timeout = millis() + (servoActivationTime * 1000) + 500; // Safety margin
          runtime.state = TIMER_COMPLETING_SERVO;
          
          DEBUG_PRINTLN("Timer: Delay complete, triggered once (Trigger mode), waiting for servo completion");
        } else {
          // RELEASE MODE
          runtime.state = TIMER_RELEASE_RUNNING;
          runtime.currentPhaseStartTime = millis();
          servo_move_to_position(servoEndPosition);
          runtime.frameCount = 1;
          
          DEBUG_PRINTF("Timer: Delay complete, servo ON for entire release duration (%ds)\n", 
                       runtime.totalReleaseTime);
        }
      }
      break;
      
    case TIMER_RELEASE_RUNNING:
      if (elapsedPhase >= runtime.totalReleaseTime) {
        servo_move_to_position(servoStartPosition);
        DEBUG_PRINTLN("Timer execution complete - servo returned to start position");
        cancel_timer_execution();
        show_current_page(); // Return to UI
        return;
      }
      break;
  }
  
  update_timer_overlay_display();
}

void update_tlapse_execution() {
  unsigned long currentTime = millis();
  unsigned long elapsedTotal = (currentTime - runtime.startTime) / 1000;
  
  int expectedFrames = (int)(elapsedTotal / runtime.frameInterval);
  
  // Trigger frame if needed
  if (expectedFrames > runtime.frameCount && runtime.frameCount < runtime.totalFrames) {
    servo_activate();
    runtime.frameCount++;
    DEBUG_PRINTF("T-Lapse: Frame %d/%d triggered after %.1fs\n", 
                 runtime.frameCount, runtime.totalFrames, (float)elapsedTotal);
  }
  
  // Check if T-Lapse logic is complete
  if ((elapsedTotal >= runtime.totalTime || runtime.frameCount >= runtime.totalFrames) && 
      !runtime.logic_completed) {
    
    runtime.logic_completed = true;
    DEBUG_PRINTF("T-Lapse logic complete: %d frames taken\n", runtime.frameCount);
    
    // NEU: Check if we need to wait for servo completion
    if (is_servo_completion_needed()) {
      runtime.waiting_for_servo_completion = true;
      runtime.servo_completion_timeout = millis() + (servoActivationTime * 1000) + 500;
      runtime.state = TLAPSE_COMPLETING_SERVO;
      DEBUG_PRINTLN("T-Lapse waiting for final servo completion");
    } else {
      // No servo running, complete immediately
      cancel_timer_execution();
      show_current_page();
      return;
    }
  }
  
  update_tlapse_overlay_display();
}

void update_interval_execution() {
  unsigned long currentTime = millis();
  unsigned long elapsedPhase = (currentTime - runtime.currentPhaseStartTime) / 1000;
  
  if (elapsedPhase >= runtime.intervalTime) {
    servo_activate();
    runtime.frameCount++;
    runtime.currentPhaseStartTime = millis(); 
    
    DEBUG_PRINTF("Interval: Frame %d triggered after %ds interval\n", 
                 runtime.frameCount, runtime.intervalTime);
  }
  
  update_interval_overlay_display();
}

// =============================================================================
// OVERLAY FUNCTIONS - VOLLSTÄNDIG
// =============================================================================
void create_timer_overlays() {
  DEBUG_PRINTLN("Creating timer overlays...");
  
  // Timer Overlay
  timer_overlay = lv_obj_create(lv_scr_act());
  lv_obj_set_size(timer_overlay, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(timer_overlay, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_style_border_width(timer_overlay, 0, 0);
  lv_obj_set_style_pad_all(timer_overlay, 20, 0);
  lv_obj_add_flag(timer_overlay, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(timer_overlay, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t *timer_time_left_label = lv_label_create(timer_overlay);
  lv_label_set_text(timer_time_left_label, "Time left");
  lv_obj_set_style_text_font(timer_time_left_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(timer_time_left_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(timer_time_left_label, LV_ALIGN_TOP_MID, 0, 40);
  
  timer_overlay_time_label = lv_label_create(timer_overlay);
  lv_label_set_text(timer_overlay_time_label, "00:15");
  lv_obj_set_style_text_font(timer_overlay_time_label, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(timer_overlay_time_label, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_align(timer_overlay_time_label, LV_ALIGN_CENTER, 0, -20);
  
  timer_overlay_time_remaining_label = lv_label_create(timer_overlay);
  lv_label_set_text(timer_overlay_time_remaining_label, "");
  lv_obj_set_style_text_font(timer_overlay_time_remaining_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(timer_overlay_time_remaining_label, lv_color_hex(0x808080), 0);
  lv_obj_align(timer_overlay_time_remaining_label, LV_ALIGN_CENTER, 0, 38);
  
  timer_overlay_cancel_btn = lv_btn_create(timer_overlay);
  lv_obj_set_size(timer_overlay_cancel_btn, 150, 46);
  lv_obj_align(timer_overlay_cancel_btn, LV_ALIGN_BOTTOM_MID, 0, -16);
  lv_obj_set_style_bg_color(timer_overlay_cancel_btn, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_add_event_cb(timer_overlay_cancel_btn, timer_cancel_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *timer_cancel_label = lv_label_create(timer_overlay_cancel_btn);
  lv_label_set_text(timer_cancel_label, "Cancel");
  lv_obj_set_style_text_color(timer_cancel_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_set_style_text_font(timer_cancel_label, &lv_font_montserrat_20, 0);
  lv_obj_center(timer_cancel_label);

  // T-Lapse Overlay
  tlapse_overlay = lv_obj_create(lv_scr_act());
  lv_obj_set_size(tlapse_overlay, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(tlapse_overlay, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_style_border_width(tlapse_overlay, 0, 0);
  lv_obj_set_style_pad_all(tlapse_overlay, 20, 0);
  lv_obj_add_flag(tlapse_overlay, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(tlapse_overlay, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t *tlapse_started_label = lv_label_create(tlapse_overlay);
  lv_label_set_text(tlapse_started_label, "Started");
  lv_obj_set_style_text_font(tlapse_started_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(tlapse_started_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(tlapse_started_label, LV_ALIGN_TOP_MID, 0, 24);
  
  tlapse_overlay_time_label = lv_label_create(tlapse_overlay);
  lv_label_set_text(tlapse_overlay_time_label, "02:21");
  lv_obj_set_style_text_font(tlapse_overlay_time_label, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(tlapse_overlay_time_label, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_align(tlapse_overlay_time_label, LV_ALIGN_CENTER, 0, -46);
  
  // T-Lapse Frame Counter (rounded rectangle)
  lv_obj_t *tlapse_frame_container = lv_obj_create(tlapse_overlay);
  lv_obj_set_size(tlapse_frame_container, 96, 50);
  lv_obj_align(tlapse_frame_container, LV_ALIGN_CENTER, 0, 24);
  lv_obj_set_style_bg_color(tlapse_frame_container, lv_color_hex(COLOR_BTN_SECONDARY), 0);
  lv_obj_set_style_radius(tlapse_frame_container, 10, 0);
  lv_obj_set_style_border_width(tlapse_frame_container, 0, 0);
  lv_obj_set_scrollbar_mode(tlapse_frame_container, LV_SCROLLBAR_MODE_OFF);
  
  tlapse_overlay_frame_counter = lv_label_create(tlapse_frame_container);
  lv_label_set_text(tlapse_overlay_frame_counter, "1");
  lv_obj_set_style_text_font(tlapse_overlay_frame_counter, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(tlapse_overlay_frame_counter, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_center(tlapse_overlay_frame_counter);
  
  tlapse_overlay_cancel_btn = lv_btn_create(tlapse_overlay);
  lv_obj_set_size(tlapse_overlay_cancel_btn, 150, 46);
  lv_obj_align(tlapse_overlay_cancel_btn, LV_ALIGN_BOTTOM_MID, 0, -16);
  lv_obj_set_style_bg_color(tlapse_overlay_cancel_btn, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_add_event_cb(tlapse_overlay_cancel_btn, tlapse_cancel_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *tlapse_cancel_label = lv_label_create(tlapse_overlay_cancel_btn);
  lv_label_set_text(tlapse_cancel_label, "Cancel");
  lv_obj_set_style_text_color(tlapse_cancel_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_set_style_text_font(tlapse_cancel_label, &lv_font_montserrat_20, 0);
  lv_obj_center(tlapse_cancel_label);
  
  // Interval Overlay
  interval_overlay = lv_obj_create(lv_scr_act());
  lv_obj_set_size(interval_overlay, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(interval_overlay, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_style_border_width(interval_overlay, 0, 0);
  lv_obj_set_style_pad_all(interval_overlay, 20, 0);
  lv_obj_add_flag(interval_overlay, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(interval_overlay, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t *interval_started_label = lv_label_create(interval_overlay);
  lv_label_set_text(interval_started_label, "Started");
  lv_obj_set_style_text_font(interval_started_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(interval_started_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(interval_started_label, LV_ALIGN_TOP_MID, 0, 24);
  
  interval_overlay_time_label = lv_label_create(interval_overlay);
  lv_label_set_text(interval_overlay_time_label, "00:00");
  lv_obj_set_style_text_font(interval_overlay_time_label, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(interval_overlay_time_label, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_align(interval_overlay_time_label, LV_ALIGN_CENTER, 0, -46);
  
  // Interval Frame Counter
  lv_obj_t *interval_frame_container = lv_obj_create(interval_overlay);
  lv_obj_set_size(interval_frame_container, 96, 50);
  lv_obj_align(interval_frame_container, LV_ALIGN_CENTER, 0, 24);
  lv_obj_set_style_bg_color(interval_frame_container, lv_color_hex(COLOR_BTN_SECONDARY), 0);
  lv_obj_set_style_radius(interval_frame_container, 10, 0);
  lv_obj_set_style_border_width(interval_frame_container, 0, 0);
  lv_obj_set_scrollbar_mode(interval_frame_container, LV_SCROLLBAR_MODE_OFF);
  
  interval_overlay_frame_counter = lv_label_create(interval_frame_container);
  lv_label_set_text(interval_overlay_frame_counter, "0");
  lv_obj_set_style_text_font(interval_overlay_frame_counter, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(interval_overlay_frame_counter, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_center(interval_overlay_frame_counter);
  
  interval_overlay_cancel_btn = lv_btn_create(interval_overlay);
  lv_obj_set_size(interval_overlay_cancel_btn, 150, 46);
  lv_obj_align(interval_overlay_cancel_btn, LV_ALIGN_BOTTOM_MID, 0, -16);
  lv_obj_set_style_bg_color(interval_overlay_cancel_btn, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_add_event_cb(interval_overlay_cancel_btn, interval_cancel_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *interval_cancel_label = lv_label_create(interval_overlay_cancel_btn);
  lv_label_set_text(interval_cancel_label, "Cancel");
  lv_obj_set_style_text_color(interval_cancel_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_set_style_text_font(interval_cancel_label, &lv_font_montserrat_20, 0);
  lv_obj_center(interval_cancel_label);
  
  DEBUG_PRINTLN("Timer overlays created successfully!");
}

// Overlay Management
void show_timer_overlay() {
  hide_timer_overlays();
  lv_obj_clear_flag(timer_overlay, LV_OBJ_FLAG_HIDDEN);
  update_timer_overlay_display();
}

void show_tlapse_overlay() {
  hide_timer_overlays();
  lv_obj_clear_flag(tlapse_overlay, LV_OBJ_FLAG_HIDDEN);
  update_tlapse_overlay_display();
}

void show_interval_overlay() {
  hide_timer_overlays();
  lv_obj_clear_flag(interval_overlay, LV_OBJ_FLAG_HIDDEN);
  update_interval_overlay_display();
}

void hide_timer_overlays() {
  if (timer_overlay) lv_obj_add_flag(timer_overlay, LV_OBJ_FLAG_HIDDEN);
  if (tlapse_overlay) lv_obj_add_flag(tlapse_overlay, LV_OBJ_FLAG_HIDDEN);
  if (interval_overlay) lv_obj_add_flag(interval_overlay, LV_OBJ_FLAG_HIDDEN);
}

// =============================================================================
// OVERLAY UPDATE FUNCTIONS - GEÄNDERT für Completion States
// =============================================================================
void update_timer_overlay_display() {
  unsigned long currentTime = millis();
  unsigned long elapsedTotal = (currentTime - runtime.startTime) / 1000;
  unsigned long elapsedPhase = (currentTime - runtime.currentPhaseStartTime) / 1000;
  
  if (runtime.state == TIMER_COMPLETING_SERVO) {
    // NEU: During servo completion - show completion message
    lv_label_set_text(timer_overlay_time_label, "SHOT");
    lv_obj_set_style_text_color(timer_overlay_time_label, lv_color_hex(COLOR_BTN_SUCCESS), 0);
    lv_label_set_text(timer_overlay_time_remaining_label, "Completing...");
    return;
  }
  
  if (runtime.state == TIMER_DELAY_RUNNING) {
    int remaining = runtime.totalDelayTime - elapsedPhase;
    if (remaining < 0) remaining = 0;
    
    int minutes = remaining / 60;
    int seconds = remaining % 60;
    String timeStr = (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
                     (seconds < 10 ? "0" : "") + String(seconds);
    
    lv_label_set_text(timer_overlay_time_label, timeStr.c_str());
    lv_obj_set_style_text_color(timer_overlay_time_label, lv_color_hex(COLOR_BTN_PRIMARY), 0);
    
    if (runtime.totalReleaseTime > 0) {
      int releaseMin = runtime.totalReleaseTime / 60;
      int releaseSec = runtime.totalReleaseTime % 60;
      String releaseStr = String("+") + (releaseMin < 10 ? "0" : "") + String(releaseMin) + ":" + 
                         (releaseSec < 10 ? "0" : "") + String(releaseSec);
      lv_label_set_text(timer_overlay_time_remaining_label, releaseStr.c_str());
    } else {
      lv_label_set_text(timer_overlay_time_remaining_label, "SHOT");
    }
  } 
  else if (runtime.state == TIMER_RELEASE_RUNNING) {
    int remaining = runtime.totalReleaseTime - elapsedPhase;
    if (remaining < 0) remaining = 0;
    
    int minutes = remaining / 60;
    int seconds = remaining % 60;
    String timeStr = (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
                     (seconds < 10 ? "0" : "") + String(seconds);
    
    lv_label_set_text(timer_overlay_time_label, timeStr.c_str());
    lv_obj_set_style_text_color(timer_overlay_time_label, lv_color_hex(0x808080), 0);
    lv_label_set_text(timer_overlay_time_remaining_label, "HOLD");
  }
}

void update_tlapse_overlay_display() {
  unsigned long currentTime = millis();
  unsigned long elapsedTotal = (currentTime - runtime.startTime) / 1000;
  
  // Normale Anzeige auch während Completion - keine visuellen Änderungen
  int minutes = elapsedTotal / 60;
  int seconds = elapsedTotal % 60;
  String timeStr = (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
                   (seconds < 10 ? "0" : "") + String(seconds);
  
  lv_label_set_text(tlapse_overlay_time_label, timeStr.c_str());
  lv_label_set_text(tlapse_overlay_frame_counter, String(runtime.frameCount).c_str());
}

void update_interval_overlay_display() {
  unsigned long currentTime = millis();
  unsigned long elapsedTotal = (currentTime - runtime.startTime) / 1000;
  
  // Normale Anzeige auch während Completion - keine visuellen Änderungen
  int minutes = elapsedTotal / 60;
  int seconds = elapsedTotal % 60;
  String timeStr = (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
                   (seconds < 10 ? "0" : "") + String(seconds);
  
  lv_label_set_text(interval_overlay_time_label, timeStr.c_str());
  lv_label_set_text(interval_overlay_frame_counter, String(runtime.frameCount).c_str());
}

// =============================================================================
// EVENT CALLBACKS
// =============================================================================
void timer_cancel_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Timer cancelled by user");
    cancel_timer_execution();
    show_current_page();
  }
}

void tlapse_cancel_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("T-Lapse cancelled by user");
    cancel_timer_execution();
    show_current_page();
  }
}

void interval_cancel_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Interval cancelled by user");
    cancel_timer_execution();
    show_current_page();
  }
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================
String format_countdown_time(int totalSeconds, int elapsedSeconds, bool showBoth) {
  int remaining = totalSeconds - elapsedSeconds;
  if (remaining < 0) remaining = 0;
  
  int minutes = remaining / 60;
  int seconds = remaining % 60;
  
  String result = (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
                  (seconds < 10 ? "0" : "") + String(seconds);
  
  if (showBoth && elapsedSeconds > 0) {
    int elapsedMin = elapsedSeconds / 60;
    int elapsedSec = elapsedSeconds % 60;
    String elapsed = (elapsedMin < 10 ? "0" : "") + String(elapsedMin) + ":" + 
                     (elapsedSec < 10 ? "0" : "") + String(elapsedSec);
    result = elapsed + " / " + result;
  }
  
  return result;
}

#endif // TIMER_SYSTEM_H