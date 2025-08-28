/*
=============================================================================
bluetooth.h - BLE Remote Control System
=============================================================================
*/

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "config.h"
#include "state_machine.h"

// =============================================================================
// BLE CONFIGURATION
// =============================================================================
#define BLE_DEVICE_NAME "RS1 Camera"
#define BLE_SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define BLE_CHARACTERISTIC_UUID "87654321-4321-4321-4321-cba987654321"

// BLE Commands - App Remote Control
#define BLE_CMD_TIMER_REMOTE    "TIMER:"      // Format: TIMER:delay:release:start (TIMER:5:2:1)
#define BLE_CMD_TLAPSE_REMOTE   "TLAPSE:"     // Format: TLAPSE:total:frames:start (TLAPSE:120:30:1)  
#define BLE_CMD_INTERVAL_REMOTE "INTERVAL:"   // Format: INTERVAL:interval:start (INTERVAL:10:1)
#define BLE_CMD_SIMPLE_TRIGGER  "SIMPLE"     // Simple immediate trigger
#define BLE_CMD_CANCEL_ALL      "CANCEL"     // Cancel any running timer/tlapse/interval
#define BLE_CMD_DISCONNECT      "DISCONNECT"
#define BLE_CMD_STATUS          "STATUS"

// BLE Response Codes
#define BLE_RESP_OK             "OK:"
#define BLE_RESP_ERROR          "ERROR:"
#define BLE_RESP_STATUS         "STATUS:"

// =============================================================================
// BLE STATE
// =============================================================================
enum BLEConnectionState {
  BLE_DISCONNECTED,
  BLE_ADVERTISING,
  BLE_CONNECTED
};

enum BLEControlMode {
  BLE_CONTROL_NONE,      // Device operates independently
  BLE_CONTROL_ACTIVE     // App has control, but device UI still accessible
};

struct BLESystemState {
  BLEConnectionState connection_state;
  BLEControlMode control_mode;
  bool enabled;
  bool client_connected;
  String connected_device_name;
  unsigned long connection_start_time;
  unsigned long last_heartbeat;
};

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================
extern BLESystemState ble_state;
extern BLEServer* ble_server;
extern BLECharacteristic* ble_characteristic;

// UI Objects for BLE overlay
extern lv_obj_t *ble_overlay;
extern lv_obj_t *ble_overlay_title;
extern lv_obj_t *ble_overlay_device_name;
extern lv_obj_t *ble_overlay_connection_time;
extern lv_obj_t *ble_overlay_disconnect_btn;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================
void bluetooth_init();
void bluetooth_enable();
void bluetooth_disable();
void bluetooth_update();
void bluetooth_disconnect_client();

// BLE Server Callbacks
class RS1ServerCallbacks;
class RS1CharacteristicCallbacks;

// Overlay Management
void create_ble_overlay();
void show_ble_overlay();
void hide_ble_overlay();
void update_ble_overlay_display();

// Command Processing
void process_ble_command(String command);
void send_ble_response(String response);
void start_remote_timer(int delay, int release);
void start_remote_tlapse(int total, int frames);
void start_remote_interval(int interval);
String get_device_status();
bool parse_timer_command(String command, int &delay, int &release, bool &start);
bool parse_tlapse_command(String command, int &total, int &frames, bool &start);
bool parse_interval_command(String command, int &interval, bool &start);

// Event Callbacks
void ble_disconnect_cb(lv_event_t *e);

// =============================================================================
// BLE SYSTEM STATE
// =============================================================================
BLESystemState ble_state = {
  BLE_DISCONNECTED,
  BLE_CONTROL_NONE,
  false,
  false,
  "",
  0,
  0
};

BLEServer* ble_server = nullptr;
BLECharacteristic* ble_characteristic = nullptr;

// UI Objects
lv_obj_t *ble_overlay = nullptr;
lv_obj_t *ble_overlay_title = nullptr;
lv_obj_t *ble_overlay_device_name = nullptr;
lv_obj_t *ble_overlay_connection_time = nullptr;
lv_obj_t *ble_overlay_disconnect_btn = nullptr;

// =============================================================================
// BLE SERVER CALLBACKS
// =============================================================================
class RS1ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* server) {
    ble_state.client_connected = true;
    ble_state.connection_state = BLE_CONNECTED;
    ble_state.control_mode = BLE_CONTROL_ACTIVE;  // App has control but UI stays accessible
    ble_state.connection_start_time = millis();
    ble_state.last_heartbeat = millis();
    
    DEBUG_PRINTLN("BLE Client connected - App can control device remotely");
    show_ble_overlay();
    
    // Send welcome message with device status
    send_ble_response("OK:CONNECTED:REMOTE_CONTROL_READY");
  }

  void onDisconnect(BLEServer* server) {
    ble_state.client_connected = false;
    ble_state.connection_state = BLE_ADVERTISING;
    ble_state.control_mode = BLE_CONTROL_NONE;    // Return to device-only control
    ble_state.connected_device_name = "";
    
    DEBUG_PRINTLN("BLE Client disconnected - Device operating independently");
    hide_ble_overlay();
    
    // Cancel any running remote timers if they were started via BLE
    if (runtime.state != TIMER_IDLE) {
      // Let the user decide if they want to cancel - don't auto-cancel
      DEBUG_PRINTLN("Timer still running - user can cancel manually if needed");
    }
    
    // Restart advertising
    server->startAdvertising();
  }
};

class RS1CharacteristicCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) {
    String command = characteristic->getValue().c_str();
    command.trim();
    
    DEBUG_PRINTF("BLE Command received: %s\n", command.c_str());
    ble_state.last_heartbeat = millis();
    
    process_ble_command(command);
  }
};

// =============================================================================
// BLUETOOTH SYSTEM FUNCTIONS
// =============================================================================
void bluetooth_init() {
  DEBUG_PRINTLN("Initializing Bluetooth system...");
  
  ble_state.enabled = app_state.bluetooth_enabled;
  ble_state.connection_state = BLE_DISCONNECTED;
  ble_state.client_connected = false;
  
  create_ble_overlay();
  
  if (ble_state.enabled) {
    bluetooth_enable();
  }
  
  DEBUG_PRINTLN("Bluetooth system initialized");
}

void bluetooth_enable() {
  if (ble_state.enabled) return;
  
  DEBUG_PRINTLN("Enabling Bluetooth...");
  
  // Initialize BLE
  BLEDevice::init(BLE_DEVICE_NAME);
  ble_server = BLEDevice::createServer();
  ble_server->setCallbacks(new RS1ServerCallbacks());
  
  // Create service
  BLEService *service = ble_server->createService(BLE_SERVICE_UUID);
  
  // Create characteristic
  ble_characteristic = service->createCharacteristic(
                         BLE_CHARACTERISTIC_UUID,
                         BLECharacteristic::PROPERTY_READ |
                         BLECharacteristic::PROPERTY_WRITE |
                         BLECharacteristic::PROPERTY_NOTIFY
                       );
  
  ble_characteristic->setCallbacks(new RS1CharacteristicCallbacks());
  ble_characteristic->addDescriptor(new BLE2902());
  
  // Start service
  service->start();
  
  // Start advertising
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(BLE_SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->setMinPreferred(0x12);
  
  ble_server->startAdvertising();
  
  ble_state.enabled = true;
  ble_state.connection_state = BLE_ADVERTISING;
  
  DEBUG_PRINTLN("Bluetooth enabled and advertising");
}

void bluetooth_disable() {
  if (!ble_state.enabled) return;
  
  DEBUG_PRINTLN("Disabling Bluetooth...");
  
  // Disconnect any connected clients
  if (ble_state.client_connected) {
    bluetooth_disconnect_client();
  }
  
  // Stop advertising and deinit
  if (ble_server) {
    ble_server->getAdvertising()->stop();
  }
  
  BLEDevice::deinit(false);
  
  ble_state.enabled = false;
  ble_state.connection_state = BLE_DISCONNECTED;
  ble_state.client_connected = false;
  
  hide_ble_overlay();
  
  DEBUG_PRINTLN("Bluetooth disabled");
}

void bluetooth_update() {
  if (!ble_state.enabled) return;
  
  // Update connection display
  if (ble_state.client_connected && ble_overlay) {
    update_ble_overlay_display();
  }
  
  // Check for connection timeout (optional)
  if (ble_state.client_connected) {
    unsigned long timeout = millis() - ble_state.last_heartbeat;
    if (timeout > 30000) { // 30 second timeout
      DEBUG_PRINTLN("BLE connection timeout - disconnecting");
      bluetooth_disconnect_client();
    }
  }
}

void bluetooth_disconnect_client() {
  if (ble_state.client_connected && ble_server) {
    DEBUG_PRINTLN("Disconnecting BLE client");
    ble_server->disconnect(ble_server->getConnId());
  }
}

// =============================================================================
// COMMAND PROCESSING
// =============================================================================
void process_ble_command(String command) {
  if (command.startsWith(BLE_CMD_TIMER_REMOTE)) {
    // Format: TIMER:delay:release:start (e.g., TIMER:5:2:1)
    int delay, release;
    bool start;
    if (parse_timer_command(command, delay, release, start)) {
      if (start) {
        start_remote_timer(delay, release);
        send_ble_response("OK:TIMER_STARTED:" + String(delay) + ":" + String(release));
      } else {
        send_ble_response("OK:TIMER_SET:" + String(delay) + ":" + String(release));
      }
    } else {
      send_ble_response("ERROR:INVALID_TIMER_FORMAT");
    }
  }
  else if (command.startsWith(BLE_CMD_TLAPSE_REMOTE)) {
    // Format: TLAPSE:total:frames:start (e.g., TLAPSE:120:30:1)  
    int total, frames;
    bool start;
    if (parse_tlapse_command(command, total, frames, start)) {
      if (start) {
        start_remote_tlapse(total, frames);
        send_ble_response("OK:TLAPSE_STARTED:" + String(total) + ":" + String(frames));
      } else {
        send_ble_response("OK:TLAPSE_SET:" + String(total) + ":" + String(frames));
      }
    } else {
      send_ble_response("ERROR:INVALID_TLAPSE_FORMAT");
    }
  }
  else if (command.startsWith(BLE_CMD_INTERVAL_REMOTE)) {
    // Format: INTERVAL:interval:start (e.g., INTERVAL:10:1)
    int interval;
    bool start;
    if (parse_interval_command(command, interval, start)) {
      if (start) {
        start_remote_interval(interval);
        send_ble_response("OK:INTERVAL_STARTED:" + String(interval));
      } else {
        send_ble_response("OK:INTERVAL_SET:" + String(interval));
      }
    } else {
      send_ble_response("ERROR:INVALID_INTERVAL_FORMAT");
    }
  }
  else if (command == BLE_CMD_SIMPLE_TRIGGER) {
    // Simple immediate trigger - works in any mode
    servo_activate();
    send_ble_response("OK:SIMPLE_TRIGGERED");
    DEBUG_PRINTLN("BLE: Simple trigger activated");
  }
  else if (command == BLE_CMD_CANCEL_ALL) {
    if (runtime.state != TIMER_IDLE) {
      cancel_timer_execution();
      show_current_page();
      send_ble_response("OK:ALL_CANCELLED");
    } else {
      send_ble_response("OK:NOTHING_TO_CANCEL");
    }
  }
  else if (command == BLE_CMD_STATUS) {
    send_ble_response(get_device_status());
  }
  else if (command == BLE_CMD_DISCONNECT) {
    send_ble_response("OK:DISCONNECTING");
    delay(100); // Give time for response to send
    bluetooth_disconnect_client();
  }
  else {
    send_ble_response("ERROR:UNKNOWN_COMMAND:" + command);
  }
}

bool parse_timer_command(String command, int &delay, int &release, bool &start) {
  // Format: TIMER:delay:release:start
  String params = command.substring(6); // Remove "TIMER:"
  
  int first_colon = params.indexOf(':');
  int second_colon = params.indexOf(':', first_colon + 1);
  
  if (first_colon == -1 || second_colon == -1) return false;
  
  delay = params.substring(0, first_colon).toInt();
  release = params.substring(first_colon + 1, second_colon).toInt();
  start = params.substring(second_colon + 1).toInt() == 1;
  
  // Validate ranges
  if (delay < 0 || delay > 3599 || release < 0 || release > 3599) return false;
  
  return true;
}

bool parse_tlapse_command(String command, int &total, int &frames, bool &start) {
  // Format: TLAPSE:total:frames:start
  String params = command.substring(7); // Remove "TLAPSE:"
  
  int first_colon = params.indexOf(':');
  int second_colon = params.indexOf(':', first_colon + 1);
  
  if (first_colon == -1 || second_colon == -1) return false;
  
  total = params.substring(0, first_colon).toInt();
  frames = params.substring(first_colon + 1, second_colon).toInt();
  start = params.substring(second_colon + 1).toInt() == 1;
  
  // Validate ranges and logic
  if (total < 1 || total > 3599 || frames < 0 || frames > total) return false;
  
  return true;
}

bool parse_interval_command(String command, int &interval, bool &start) {
  // Format: INTERVAL:interval:start
  String params = command.substring(9); // Remove "INTERVAL:"
  
  int colon = params.indexOf(':');
  if (colon == -1) return false;
  
  interval = params.substring(0, colon).toInt();
  start = params.substring(colon + 1).toInt() == 1;
  
  // Validate range
  if (interval < 1 || interval > 3599) return false;
  
  return true;
}

void start_remote_timer(int delay, int release) {
  DEBUG_PRINTF("Starting remote timer: %ds delay, %ds release\n", delay, release);
  
  // Set up runtime with app values (not device values)
  runtime.totalDelayTime = delay;
  runtime.totalReleaseTime = release;
  runtime.mode = TIMER_EXEC_MODE;
  runtime.state = TIMER_DELAY_RUNNING;
  runtime.startTime = millis();
  runtime.currentPhaseStartTime = millis();
  runtime.frameCount = 0;
  runtime.logic_completed = false;
  
  servo_move_to_position(servoStartPosition);
  show_timer_overlay();
}

void start_remote_tlapse(int total, int frames) {
  DEBUG_PRINTF("Starting remote T-Lapse: %ds total, %d frames\n", total, frames);
  
  // Set up runtime with app values
  runtime.totalTime = total;
  runtime.totalFrames = frames;
  runtime.mode = TLAPSE_EXEC_MODE;
  runtime.state = TLAPSE_RUNNING;
  runtime.startTime = millis();
  runtime.currentPhaseStartTime = millis();
  runtime.frameCount = 0;
  runtime.logic_completed = false;
  
  if (frames > 0) {
    runtime.frameInterval = (float)total / frames;
  } else {
    runtime.frameInterval = 1.0;
  }
  
  servo_move_to_position(servoStartPosition);
  show_tlapse_overlay();
}

void start_remote_interval(int interval) {
  DEBUG_PRINTF("Starting remote Interval: %ds interval\n", interval);
  
  // Set up runtime with app values
  runtime.intervalTime = interval;
  runtime.mode = INTERVAL_EXEC_MODE;
  runtime.state = INTERVAL_RUNNING;
  runtime.startTime = millis();
  runtime.currentPhaseStartTime = millis();
  runtime.frameCount = 0;
  runtime.logic_completed = false;
  
  servo_move_to_position(servoStartPosition);
  show_interval_overlay();
}

void send_ble_response(String response) {
  if (ble_characteristic && ble_state.client_connected) {
    ble_characteristic->setValue(response.c_str());
    ble_characteristic->notify();
    DEBUG_PRINTF("BLE Response sent: %s\n", response.c_str());
  }
}

String get_device_status() {
  String status = "STATUS:";
  
  // Current page
  switch (app_state.current_state) {
    case STATE_MAIN: status += "MAIN"; break;
    case STATE_TIMER: status += "TIMER"; break;
    case STATE_TLAPSE: status += "TLAPSE"; break;
    case STATE_INTERVAL: status += "INTERVAL"; break;
    case STATE_SETTINGS: status += "SETTINGS"; break;
    default: status += "UNKNOWN"; break;
  }
  
  // Timer state
  status += ",TIMER_STATE:";
  switch (runtime.state) {
    case TIMER_IDLE: status += "IDLE"; break;
    case TIMER_DELAY_RUNNING: status += "DELAY"; break;
    case TIMER_RELEASE_RUNNING: status += "RELEASE"; break;
    case TLAPSE_RUNNING: status += "TLAPSE"; break;
    case INTERVAL_RUNNING: status += "INTERVAL"; break;
    default: status += "UNKNOWN"; break;
  }
  
  return status;
}

// =============================================================================
// BLE OVERLAY FUNCTIONS
// =============================================================================
void create_ble_overlay() {
  DEBUG_PRINTLN("Creating BLE overlay...");
  
  ble_overlay = lv_obj_create(lv_scr_act());
  lv_obj_set_size(ble_overlay, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(ble_overlay, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_style_border_width(ble_overlay, 0, 0);
  lv_obj_set_style_pad_all(ble_overlay, 20, 0);
  lv_obj_add_flag(ble_overlay, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(ble_overlay, LV_OBJ_FLAG_SCROLLABLE);
  
  ble_overlay_title = lv_label_create(ble_overlay);
  lv_label_set_text(ble_overlay_title, "Remote Connected");
  lv_obj_set_style_text_font(ble_overlay_title, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(ble_overlay_title, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(ble_overlay_title, LV_ALIGN_TOP_MID, 0, 40);
  
  ble_overlay_device_name = lv_label_create(ble_overlay);
  lv_label_set_text(ble_overlay_device_name, "Mobile App");
  lv_obj_set_style_text_font(ble_overlay_device_name, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(ble_overlay_device_name, lv_color_hex(0x5E81AC), 0);
  lv_obj_align(ble_overlay_device_name, LV_ALIGN_CENTER, 0, -20);
  
  ble_overlay_connection_time = lv_label_create(ble_overlay);
  lv_label_set_text(ble_overlay_connection_time, "Connected: 00:00");
  lv_obj_set_style_text_font(ble_overlay_connection_time, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(ble_overlay_connection_time, lv_color_hex(0x808080), 0);
  lv_obj_align(ble_overlay_connection_time, LV_ALIGN_CENTER, 0, 20);
  
  // Info text
  lv_obj_t *info_text = lv_label_create(ble_overlay);
  lv_label_set_text(info_text, "App can control device remotely\nDevice controls remain active");
  lv_obj_set_style_text_font(info_text, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(info_text, lv_color_hex(0x606060), 0);
  lv_obj_set_style_text_align(info_text, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(info_text, LV_ALIGN_CENTER, 0, 60);
  
  ble_overlay_disconnect_btn = lv_btn_create(ble_overlay);
  lv_obj_set_size(ble_overlay_disconnect_btn, 150, 46);
  lv_obj_align(ble_overlay_disconnect_btn, LV_ALIGN_BOTTOM_MID, 0, -16);
  lv_obj_set_style_bg_color(ble_overlay_disconnect_btn, lv_color_hex(COLOR_BTN_DANGER), 0);
  lv_obj_add_event_cb(ble_overlay_disconnect_btn, ble_disconnect_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *disconnect_label = lv_label_create(ble_overlay_disconnect_btn);
  lv_label_set_text(disconnect_label, "Disconnect");
  lv_obj_set_style_text_color(disconnect_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_set_style_text_font(disconnect_label, &lv_font_montserrat_20, 0);
  lv_obj_center(disconnect_label);
  
  DEBUG_PRINTLN("BLE overlay created");
}

void show_ble_overlay() {
  if (ble_overlay) {
    lv_obj_clear_flag(ble_overlay, LV_OBJ_FLAG_HIDDEN);
    update_ble_overlay_display();
  }
}

void hide_ble_overlay() {
  if (ble_overlay) {
    lv_obj_add_flag(ble_overlay, LV_OBJ_FLAG_HIDDEN);
  }
}

void update_ble_overlay_display() {
  if (!ble_overlay || !ble_state.client_connected) return;
  
  // Update connection time
  unsigned long connected_time = (millis() - ble_state.connection_start_time) / 1000;
  int minutes = connected_time / 60;
  int seconds = connected_time % 60;
  String time_str = "Connected: " + 
                   (minutes < 10 ? String("0") : String("")) + String(minutes) + ":" + 
                   (seconds < 10 ? String("0") : String("")) + String(seconds);
  
  lv_label_set_text(ble_overlay_connection_time, time_str.c_str());
}

// =============================================================================
// EVENT CALLBACKS
// =============================================================================
void ble_disconnect_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("BLE disconnect requested by user");
    bluetooth_disconnect_client();
  }
}

#endif // BLUETOOTH_H