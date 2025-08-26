/*
=============================================================================
timer_system.h - Timer/T-Lapse/Interval System with Servo Control
TEIL 1 - Kopiere diesen Block komplett in timer_system.h
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

// Timer States
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
  INTERVAL_RUNNING
};

// Timer Runtime Data
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
};

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================
extern Servo cameraServo;
extern TimerRuntime runtime;

bool servo_is_activating = false;
unsigned long servo_activation_start_time = 0;

// Servo Settings (adjustable)
extern int servoStartPosition;      // Start position (0-180°)
extern int servoEndPosition;        // End position (0-180°)
extern float servoActivationTime;   // Time in seconds for servo activation

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

// System Functions
void timer_system_init();
void timer_system_update();

// Servo Control Functions
void servo_init();
void servo_activate();
void servo_move_to_position(int position);

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
int servoStartPosition = 0;      // Start position
int servoEndPosition = 90;       // End position  
float servoActivationTime = 0.6; // Activation time in seconds

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
// SERVO FUNCTIONS
// =============================================================================
void servo_init() {
  DEBUG_PRINTLN("Initializing servo...");
  cameraServo.attach(SERVO_PIN);
  servo_move_to_position(servoStartPosition);
  delay(500); // Allow servo to reach position
  DEBUG_PRINTF("Servo initialized on pin %d (start: %d°, end: %d°)\n", 
               SERVO_PIN, servoStartPosition, servoEndPosition);
}

void servo_move_to_position(int position) {
  position = constrain(position, 0, 180);
  cameraServo.write(position);
  DEBUG_PRINTF("Servo moved to %d°\n", position);
}

void servo_activate() {
  if (!servo_is_activating) {
    // Start activation
    servo_is_activating = true;
    servo_activation_start_time = millis();
    servo_move_to_position(servoEndPosition);
    DEBUG_PRINTF("Servo activation started: %d° -> %d° for %.1fs\n", 
                 servoStartPosition, servoEndPosition, servoActivationTime);
  }
}

void servo_update() {
  if (servo_is_activating) {
    unsigned long elapsed = millis() - servo_activation_start_time;
    if (elapsed >= (servoActivationTime * 1000)) {
      // Activation time complete - return to start position
      servo_move_to_position(servoStartPosition);
      servo_is_activating = false;
      DEBUG_PRINTLN("Servo activation complete - returned to start position");
    }
  }
}

// =============================================================================
// TIMER SYSTEM FUNCTIONS
// =============================================================================
void timer_system_init() {
  DEBUG_PRINTLN("Initializing timer system...");
  
  // Initialize servo
  servo_init();
  
  // Initialize runtime data
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
  
  // Create overlays
  create_timer_overlays();
  
  DEBUG_PRINTLN("Timer system initialized successfully!");
}

void timer_system_update() {
  // Update non-blocking servo first
  servo_update();
  
  if (runtime.state == TIMER_IDLE) return;
  
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
  DEBUG_PRINTLN("Starting Timer execution...");
  
  // Get current timer values from state machine
  runtime.totalDelayTime = get_option_value(STATE_TIMER, 0);    // Delay
  uint32_t releaseValue = get_option_value(STATE_TIMER, 1);     // Release
  
  // SICHERE TRIGGER-ERKENNUNG
  bool isTriggerMode = (releaseValue == 4294967295 || (int32_t)releaseValue == -1);
  
  if (isTriggerMode) {
    runtime.totalReleaseTime = 0; // Trigger = keine Release Zeit
    DEBUG_PRINTLN("Timer in TRIGGER mode - single activation after delay");
  } else {
    runtime.totalReleaseTime = releaseValue;
  }
  
  runtime.mode = TIMER_EXEC_MODE;
  runtime.state = TIMER_DELAY_RUNNING;
  runtime.startTime = millis();
  runtime.currentPhaseStartTime = millis();
  runtime.frameCount = 0;
  
  // Servo bleibt in START-Position während Delay
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
  
  // Get current T-Lapse values from state machine
  runtime.totalTime = get_option_value(STATE_TLAPSE, 0);    // Total time
  runtime.totalFrames = get_option_value(STATE_TLAPSE, 1);  // Frames
  
  runtime.mode = TLAPSE_EXEC_MODE;
  runtime.state = TLAPSE_RUNNING;
  runtime.startTime = millis();
  runtime.currentPhaseStartTime = millis();
  runtime.frameCount = 0;
  
  // Calculate interval between frames
  if (runtime.totalFrames > 0) {
    runtime.frameInterval = (float)runtime.totalTime / runtime.totalFrames;
  } else {
    runtime.frameInterval = 1.0; // Default 1 second if no frames
  }
  
  // KORRIGIERT: Servo in Ruheposition lassen zu Beginn
  servo_move_to_position(servoStartPosition);
  
  show_tlapse_overlay();
  
  DEBUG_PRINTF("T-Lapse started: %ds total, %d frames, %.2fs interval\n", 
               runtime.totalTime, runtime.totalFrames, runtime.frameInterval);
}

void start_interval_execution() {
  DEBUG_PRINTLN("Starting Interval execution...");
  
  // Get current Interval value from state machine
  runtime.intervalTime = get_option_value(STATE_INTERVAL, 0);  // Interval
  
  runtime.mode = INTERVAL_EXEC_MODE;
  runtime.state = INTERVAL_RUNNING;
  runtime.startTime = millis();
  runtime.currentPhaseStartTime = millis();
  runtime.frameCount = 0;
  
  // KORRIGIERT: Servo in Ruheposition lassen zu Beginn
  servo_move_to_position(servoStartPosition);
  
  show_interval_overlay();
  
  DEBUG_PRINTF("Interval started: %ds interval\n", runtime.intervalTime);
}

void cancel_timer_execution() {
  DEBUG_PRINTLN("Timer execution cancelled");
  
  runtime.state = TIMER_IDLE;
  runtime.frameCount = 0;
  
  // Reset servo to start position
  servo_move_to_position(servoStartPosition);
  
  // Hide all overlays
  hide_timer_overlays();
}

// =============================================================================
// TIMER UPDATE FUNCTIONS  
// =============================================================================


void update_timer_execution() {
  unsigned long currentTime = millis();
  unsigned long elapsedTotal = (currentTime - runtime.startTime) / 1000;
  unsigned long elapsedPhase = (currentTime - runtime.currentPhaseStartTime) / 1000;
  
  switch (runtime.state) {
    case TIMER_DELAY_RUNNING:
      if (elapsedPhase >= runtime.totalDelayTime) {
        if (runtime.totalReleaseTime == 0) {
          // TRIGGER MODE: Kurze Aktivierung und fertig
          servo_activate(); // 0.6s Aktivierung
          runtime.frameCount = 1;
          DEBUG_PRINTLN("Timer: Delay complete, triggered once (Trigger mode), finishing");
          delay(100); // Kurze Pause damit User die Aktivierung sieht
          cancel_timer_execution();
          return;
        } else {
          // NEUE LOGIK: NORMALE RELEASE - Servo für GESAMTE Release-Dauer aktivieren
          runtime.state = TIMER_RELEASE_RUNNING;
          runtime.currentPhaseStartTime = millis();
          
          // Servo auf END-Position für die gesamte Release-Dauer
          servo_move_to_position(servoEndPosition);
          runtime.frameCount = 1;
          
          DEBUG_PRINTF("Timer: Delay complete, servo ON for entire release duration (%ds)\n", 
                       runtime.totalReleaseTime);
        }
      }
      break;
      
    case TIMER_RELEASE_RUNNING:
      if (elapsedPhase >= runtime.totalReleaseTime) {
        // RELEASE COMPLETE: Servo zurück zur Start-Position
        servo_move_to_position(servoStartPosition);
        DEBUG_PRINTLN("Timer execution complete - servo returned to start position");
        cancel_timer_execution();
        return;
      }
      break;
  }
  
  update_timer_overlay_display();
}

void update_tlapse_execution() {
  unsigned long currentTime = millis();
  unsigned long elapsedTotal = (currentTime - runtime.startTime) / 1000;
  
  // KORRIGIERT: Berechne welcher Frame jetzt fällig ist
  int expectedFrames = (int)(elapsedTotal / runtime.frameInterval);
  
  // KORRIGIERT: Prüfe ob genug Zeit vergangen ist für nächsten Frame
  if (expectedFrames > runtime.frameCount && runtime.frameCount < runtime.totalFrames) {
    // KORRIGIERT: Frame erst nach dem berechneten Interval auslösen
    servo_activate();
    runtime.frameCount++;
    DEBUG_PRINTF("T-Lapse: Frame %d/%d triggered after %.1fs\n", 
                 runtime.frameCount, runtime.totalFrames, elapsedTotal);
  }
  
  // Check if T-Lapse is complete
  if (elapsedTotal >= runtime.totalTime || runtime.frameCount >= runtime.totalFrames) {
    DEBUG_PRINTF("T-Lapse execution complete: %d frames taken\n", runtime.frameCount);
    cancel_timer_execution();
    return;
  }
  
  update_tlapse_overlay_display();
}

void update_interval_execution() {
  unsigned long currentTime = millis();
  unsigned long elapsedPhase = (currentTime - runtime.currentPhaseStartTime) / 1000;
  
  // KORRIGIERT: Erst NACH dem Interval auslösen
  if (elapsedPhase >= runtime.intervalTime) {
    // KORRIGIERT: Interval Zeit abgelaufen - jetzt auslösen
    servo_activate();
    runtime.frameCount++;
    runtime.currentPhaseStartTime = millis(); // Reset for next interval
    
    DEBUG_PRINTF("Interval: Frame %d triggered after %ds interval\n", 
                 runtime.frameCount, runtime.intervalTime);
  }
  
  update_interval_overlay_display();
}



// =============================================================================
// OVERLAY FUNCTIONS - TIMER OVERLAY
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
  
  // Timer Overlay - Time Left Label
  lv_obj_t *timer_time_left_label = lv_label_create(timer_overlay);
  lv_label_set_text(timer_time_left_label, "Time left");
  lv_obj_set_style_text_font(timer_time_left_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(timer_time_left_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(timer_time_left_label, LV_ALIGN_TOP_MID, 0, 40);
  
  // Timer Overlay - Main Time Display
  timer_overlay_time_label = lv_label_create(timer_overlay);
  lv_label_set_text(timer_overlay_time_label, "00:15");
  lv_obj_set_style_text_font(timer_overlay_time_label, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(timer_overlay_time_label, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_align(timer_overlay_time_label, LV_ALIGN_CENTER, 0, -20);
  
  // Timer Overlay - Remaining Time (for release phase)
  timer_overlay_time_remaining_label = lv_label_create(timer_overlay);
  lv_label_set_text(timer_overlay_time_remaining_label, "");
  lv_obj_set_style_text_font(timer_overlay_time_remaining_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(timer_overlay_time_remaining_label, lv_color_hex(0x808080), 0);
  lv_obj_align(timer_overlay_time_remaining_label, LV_ALIGN_CENTER, 0, 40);
  
  // Timer Overlay - Cancel Button
  timer_overlay_cancel_btn = lv_btn_create(timer_overlay);
  lv_obj_set_size(timer_overlay_cancel_btn, 120, 40);
  lv_obj_align(timer_overlay_cancel_btn, LV_ALIGN_BOTTOM_MID, 0, -30);
  lv_obj_set_style_bg_color(timer_overlay_cancel_btn, lv_color_hex(0x404040), 0);
  lv_obj_add_event_cb(timer_overlay_cancel_btn, timer_cancel_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *timer_cancel_label = lv_label_create(timer_overlay_cancel_btn);
  lv_label_set_text(timer_cancel_label, "Cancel");
  lv_obj_set_style_text_color(timer_cancel_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
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
  lv_obj_set_style_text_font(tlapse_started_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(tlapse_started_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(tlapse_started_label, LV_ALIGN_TOP_MID, 0, 40);
  
  tlapse_overlay_time_label = lv_label_create(tlapse_overlay);
  lv_label_set_text(tlapse_overlay_time_label, "02:21");
  lv_obj_set_style_text_font(tlapse_overlay_time_label, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(tlapse_overlay_time_label, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_align(tlapse_overlay_time_label, LV_ALIGN_CENTER, 0, -40);
  
  // T-Lapse Frame Counter (rounded rectangle)
  lv_obj_t *tlapse_frame_container = lv_obj_create(tlapse_overlay);
  lv_obj_set_size(tlapse_frame_container, 80, 40);
  lv_obj_align(tlapse_frame_container, LV_ALIGN_CENTER, 0, 20);
  lv_obj_set_style_bg_color(tlapse_frame_container, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_set_style_radius(tlapse_frame_container, 10, 0);
  lv_obj_set_style_border_width(tlapse_frame_container, 0, 0);
  
  tlapse_overlay_frame_counter = lv_label_create(tlapse_frame_container);
  lv_label_set_text(tlapse_overlay_frame_counter, "1");
  lv_obj_set_style_text_font(tlapse_overlay_frame_counter, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(tlapse_overlay_frame_counter, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_center(tlapse_overlay_frame_counter);
  
  tlapse_overlay_cancel_btn = lv_btn_create(tlapse_overlay);
  lv_obj_set_size(tlapse_overlay_cancel_btn, 120, 40);
  lv_obj_align(tlapse_overlay_cancel_btn, LV_ALIGN_BOTTOM_MID, 0, -30);
  lv_obj_set_style_bg_color(tlapse_overlay_cancel_btn, lv_color_hex(0x404040), 0);
  lv_obj_add_event_cb(tlapse_overlay_cancel_btn, tlapse_cancel_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *tlapse_cancel_label = lv_label_create(tlapse_overlay_cancel_btn);
  lv_label_set_text(tlapse_cancel_label, "Cancel");
  lv_obj_set_style_text_color(tlapse_cancel_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
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
  lv_label_set_text(interval_started_label, "Started Since");
  lv_obj_set_style_text_font(interval_started_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(interval_started_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(interval_started_label, LV_ALIGN_TOP_MID, 0, 40);
  
  interval_overlay_time_label = lv_label_create(interval_overlay);
  lv_label_set_text(interval_overlay_time_label, "00:00");
  lv_obj_set_style_text_font(interval_overlay_time_label, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(interval_overlay_time_label, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_align(interval_overlay_time_label, LV_ALIGN_CENTER, 0, -40);
  
  // Interval Frame Counter
  lv_obj_t *interval_frame_container = lv_obj_create(interval_overlay);
  lv_obj_set_size(interval_frame_container, 80, 40);
  lv_obj_align(interval_frame_container, LV_ALIGN_CENTER, 0, 20);
  lv_obj_set_style_bg_color(interval_frame_container, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_set_style_radius(interval_frame_container, 10, 0);
  lv_obj_set_style_border_width(interval_frame_container, 0, 0);
  
  interval_overlay_frame_counter = lv_label_create(interval_frame_container);
  lv_label_set_text(interval_overlay_frame_counter, "0");
  lv_obj_set_style_text_font(interval_overlay_frame_counter, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(interval_overlay_frame_counter, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_center(interval_overlay_frame_counter);
  
  interval_overlay_cancel_btn = lv_btn_create(interval_overlay);
  lv_obj_set_size(interval_overlay_cancel_btn, 120, 40);
  lv_obj_align(interval_overlay_cancel_btn, LV_ALIGN_BOTTOM_MID, 0, -30);
  lv_obj_set_style_bg_color(interval_overlay_cancel_btn, lv_color_hex(0x404040), 0);
  lv_obj_add_event_cb(interval_overlay_cancel_btn, interval_cancel_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *interval_cancel_label = lv_label_create(interval_overlay_cancel_btn);
  lv_label_set_text(interval_cancel_label, "Cancel");
  lv_obj_set_style_text_color(interval_cancel_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_center(interval_cancel_label);
  
  DEBUG_PRINTLN("Timer overlays created successfully!");
}

// =============================================================================
// OVERLAY MANAGEMENT
// =============================================================================
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
// OVERLAY UPDATE FUNCTIONS
// =============================================================================

void update_timer_overlay_display() {
  unsigned long currentTime = millis();
  unsigned long elapsedTotal = (currentTime - runtime.startTime) / 1000;
  unsigned long elapsedPhase = (currentTime - runtime.currentPhaseStartTime) / 1000;
  
  if (runtime.state == TIMER_DELAY_RUNNING) {
    // During delay: show delay countdown in primary color
    int remaining = runtime.totalDelayTime - elapsedPhase;
    if (remaining < 0) remaining = 0;
    
    int minutes = remaining / 60;
    int seconds = remaining % 60;
    String timeStr = (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
                     (seconds < 10 ? "0" : "") + String(seconds);
    
    lv_label_set_text(timer_overlay_time_label, timeStr.c_str());
    lv_obj_set_style_text_color(timer_overlay_time_label, lv_color_hex(COLOR_BTN_PRIMARY), 0);
    
    // ANGEPASST: Zeige Release Info basierend auf Modus
    if (runtime.totalReleaseTime > 0) {
      int releaseMin = runtime.totalReleaseTime / 60;
      int releaseSec = runtime.totalReleaseTime % 60;
      String releaseStr = String("+") + (releaseMin < 10 ? "0" : "") + String(releaseMin) + ":" + 
                         (releaseSec < 10 ? "0" : "") + String(releaseSec);
      lv_label_set_text(timer_overlay_time_remaining_label, releaseStr.c_str());
    } else {
      // Trigger Mode
      lv_label_set_text(timer_overlay_time_remaining_label, "SHOT");
    }
  } 
  else if (runtime.state == TIMER_RELEASE_RUNNING) {
    // During release: show release countdown in light gray
    int remaining = runtime.totalReleaseTime - elapsedPhase;
    if (remaining < 0) remaining = 0;
    
    int minutes = remaining / 60;
    int seconds = remaining % 60;
    String timeStr = (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
                     (seconds < 10 ? "0" : "") + String(seconds);
    
    lv_label_set_text(timer_overlay_time_label, timeStr.c_str());
    lv_obj_set_style_text_color(timer_overlay_time_label, lv_color_hex(0x808080), 0);
    lv_label_set_text(timer_overlay_time_remaining_label, "Servo ON");
  }
}

void update_tlapse_overlay_display() {
  unsigned long currentTime = millis();
  unsigned long elapsedTotal = (currentTime - runtime.startTime) / 1000;
  
  // Show elapsed time in primary color
  int minutes = elapsedTotal / 60;
  int seconds = elapsedTotal % 60;
  String timeStr = (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
                   (seconds < 10 ? "0" : "") + String(seconds);
  
  lv_label_set_text(tlapse_overlay_time_label, timeStr.c_str());
  
  // Update frame counter
  lv_label_set_text(tlapse_overlay_frame_counter, String(runtime.frameCount).c_str());
}

void update_interval_overlay_display() {
  unsigned long currentTime = millis();
  unsigned long elapsedTotal = (currentTime - runtime.startTime) / 1000;
  
  // Show elapsed time in primary color
  int minutes = elapsedTotal / 60;
  int seconds = elapsedTotal % 60;
  String timeStr = (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
                   (seconds < 10 ? "0" : "") + String(seconds);
  
  lv_label_set_text(interval_overlay_time_label, timeStr.c_str());
  
  // Update frame counter
  lv_label_set_text(interval_overlay_frame_counter, String(runtime.frameCount).c_str());
}

// =============================================================================
// EVENT CALLBACKS
// =============================================================================
void timer_cancel_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Timer cancelled by user");
    cancel_timer_execution();
    // Return to previous state
    show_current_page();
  }
}

void tlapse_cancel_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("T-Lapse cancelled by user");
    cancel_timer_execution();
    // Return to previous state
    show_current_page();
  }
}

void interval_cancel_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Interval cancelled by user");
    cancel_timer_execution();
    // Return to previous state
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