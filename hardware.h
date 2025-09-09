/*
=============================================================================
hardware.h - Hardware Abstraction Layer with RotaryEncoder Library
=============================================================================
*/

#ifndef HARDWARE_H
#define HARDWARE_H
#define LEDC_FREQ 5000 

#include <lvgl.h>
#include "esp_lcd_touch_axs5106l.h"
#include <Arduino_GFX_Library.h>
#include <RotaryEncoder.h>  // NEW: Include RotaryEncoder library
#include "config.h"

// =============================================================================
// HARDWARE OBJECTS
// =============================================================================
extern Arduino_DataBus *bus;
extern Arduino_GFX *gfx;

extern uint32_t screenWidth;
extern uint32_t screenHeight;
extern uint32_t bufSize;
extern lv_disp_draw_buf_t draw_buf;
extern lv_color_t *disp_draw_buf;
extern lv_disp_drv_t disp_drv;

// =============================================================================
// ROTARY ENCODER VARIABLES - REFACTORED WITH LIBRARY
// =============================================================================
extern RotaryEncoder *encoder;  // NEW: RotaryEncoder library instance
extern volatile bool encoder_button_pressed;
extern unsigned long last_button_press;

// Adaptive encoder variables - KEEPING EXISTING FUNCTIONALITY
extern unsigned long last_encoder_change_time;
extern int encoder_speed_step_size;
extern unsigned long encoder_smoothing_buffer[ENCODER_SMOOTHING_SAMPLES];
extern int encoder_smoothing_index;
extern int current_speed_level;
extern int last_encoder_position;  // Track last position for delta calculation

// =============================================================================
// HARDWARE FUNCTIONS
// =============================================================================
void hardware_init();
void lcd_reg_init();
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

// Rotary encoder functions - UPDATED FOR LIBRARY
void encoder_init();
void encoder_init_extended();
int32_t get_encoder_delta();
int32_t get_adaptive_encoder_delta();
bool is_encoder_button_pressed();
void encoder_button_isr();  // Only button ISR needed now
void check_encoder_position();  // NEW: Library interrupt handler

// Helper functions for extended encoder features - KEEPING EXISTING
unsigned long get_smoothed_encoder_time(unsigned long current_time);
int determine_speed_level(unsigned long time_since_change, int current_level);

#if LV_USE_LOG != 0
void my_print(const char *buf);
#endif

// =============================================================================
// HARDWARE IMPLEMENTATION
// =============================================================================

// Hardware objects
Arduino_DataBus *bus = new Arduino_HWSPI(15 /* DC */, 14 /* CS */, 1 /* SCK */, 2 /* MOSI */);

Arduino_GFX *gfx = new Arduino_ST7789(
  bus, 22 /* RST */, 0 /* rotation */, false /* IPS */,
  SCREEN_WIDTH, SCREEN_HEIGHT,
  34 /*col_offset1*/, 0 /*uint8_t row_offset1*/,
  34 /*col_offset2*/, 0 /*row_offset2*/);

uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;
lv_disp_draw_buf_t draw_buf;
lv_color_t *disp_draw_buf;
lv_disp_drv_t disp_drv;

// =============================================================================
// ROTARY ENCODER IMPLEMENTATION - REFACTORED WITH LIBRARY
// =============================================================================
RotaryEncoder *encoder = nullptr;  // NEW: Library instance
volatile bool encoder_button_pressed = false;
unsigned long last_button_press = 0;

// Adaptive encoder variables - KEEPING ALL EXISTING FUNCTIONALITY
unsigned long last_encoder_change_time = 0;
int encoder_speed_step_size = 1;
unsigned long encoder_smoothing_buffer[ENCODER_SMOOTHING_SAMPLES];
int encoder_smoothing_index = 0;
int current_speed_level = 2; // Start at slowest level
int last_encoder_position = 0;  // Track position for delta calculation

// NEW: Interrupt handler for RotaryEncoder library
void IRAM_ATTR check_encoder_position() {
  encoder->tick(); // Library handles all the logic
}

// Button ISR - UNCHANGED
void IRAM_ATTR encoder_button_isr() {
  unsigned long current_time = millis();
  if (current_time - last_button_press > ENCODER_DEBOUNCE_MS) {
    encoder_button_pressed = true;
    last_button_press = current_time;
  }
}

void encoder_init() {
  DEBUG_PRINTLN("Initializing rotary encoder with RotaryEncoder library...");
  
  // NEW: Create RotaryEncoder instance
  // Use TWO03 mode for most common encoders (signals both LOW or HIGH in latch position)
  encoder = new RotaryEncoder(ENCODER_PIN_A, ENCODER_PIN_B, RotaryEncoder::LatchMode::TWO03);
  
  
  
  // NEW: Attach interrupts using library approach
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), check_encoder_position, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), check_encoder_position, CHANGE);
  
  // Initialize variables
  last_encoder_position = 0;
  encoder_button_pressed = false;
  last_button_press = 0;
  

}

void encoder_init_extended() {
  // Call basic initialization
  encoder_init();
  
  // Initialize extended variables - KEEPING ALL EXISTING LOGIC
  current_speed_level = 2; // Start at slowest level
  encoder_smoothing_index = 0;
  
  // Initialize smoothing buffer
  for (int i = 0; i < ENCODER_SMOOTHING_SAMPLES; i++) {
    encoder_smoothing_buffer[i] = 1000; // High start value = slow
  }
  
  ENCODER_DEBUG_PRINTLN("Extended encoder features initialized");
  ENCODER_DEBUG_PRINTF("Hysteresis: %s, Smoothing: %s\n", 
                      ENCODER_HYSTERESIS_ENABLED ? "ON" : "OFF",
                      ENCODER_SMOOTHING_ENABLED ? "ON" : "OFF");
}

int32_t get_encoder_delta() {
  // NEW: Use library's getPosition() instead of manual tracking
  encoder->tick(); // Ensure latest state is read
  int current_pos = encoder->getPosition();
  
  // Calculate delta from last known position
  int32_t raw_delta = current_pos - last_encoder_position;
  last_encoder_position = current_pos;
  
  // Convert library steps to logical clicks using ENCODER_STEPS_PER_CLICK
  int32_t delta = raw_delta / ENCODER_STEPS_PER_CLICK;
  
  return delta;
}

// Helper functions - KEEPING ALL EXISTING LOGIC
unsigned long get_smoothed_encoder_time(unsigned long current_time) {
  if (!ENCODER_SMOOTHING_ENABLED) {
    return current_time;
  }
  
  encoder_smoothing_buffer[encoder_smoothing_index] = current_time;
  encoder_smoothing_index = (encoder_smoothing_index + 1) % ENCODER_SMOOTHING_SAMPLES;
  
  unsigned long sum = 0;
  for (int i = 0; i < ENCODER_SMOOTHING_SAMPLES; i++) {
    sum += encoder_smoothing_buffer[i];
  }
  return sum / ENCODER_SMOOTHING_SAMPLES;
}

int determine_speed_level(unsigned long time_since_change, int current_level) {
  // KEEPING ALL EXISTING HYSTERESIS AND SPEED LOGIC
  unsigned long fast_threshold = ENCODER_SPEED_FAST_MS;      // 80ms
  unsigned long medium_threshold = ENCODER_SPEED_MEDIUM_MS;  // 250ms
  
  if (ENCODER_HYSTERESIS_ENABLED) {
    if (current_level <= 0) {
      fast_threshold = (unsigned long)(fast_threshold * ENCODER_HYSTERESIS_FACTOR);
    }
    if (current_level <= 1) {
      medium_threshold = (unsigned long)(medium_threshold * ENCODER_HYSTERESIS_FACTOR);
    }
  }
  
  if (time_since_change < fast_threshold) {
    return 0; // Fast (30s steps)
  } else if (time_since_change < medium_threshold) {
    return 1; // Medium (10s steps)
  } else {
    return 2; // Slow (1s steps)
  }
}

int32_t get_adaptive_encoder_delta() {
  // Get basic encoder delta - NOW USING LIBRARY
  int32_t delta = get_encoder_delta();
  
  if (delta != 0) {
    unsigned long current_time = millis();
    unsigned long time_since_last_change = current_time - last_encoder_change_time;
    
    // KEEPING ALL EXISTING ADAPTIVE LOGIC
    if (ENCODER_SMOOTHING_ENABLED && time_since_last_change < ENCODER_MIN_CHANGE_TIME) {
      return 0; // Too fast, ignore
    }
    
    last_encoder_change_time = current_time;
    
    unsigned long smoothed_time = get_smoothed_encoder_time(time_since_last_change);
    int new_speed_level = determine_speed_level(smoothed_time, current_speed_level);
    
    if (new_speed_level != current_speed_level) {
      ENCODER_DEBUG_PRINTF("Speed level change: %d -> %d (time: %lu ms)\n", 
                          current_speed_level, new_speed_level, smoothed_time);
    }
    current_speed_level = new_speed_level;
    
    // Determine step size based on level
    switch (current_speed_level) {
      case 0: // Fast
        encoder_speed_step_size = ENCODER_STEP_FAST;  // 30
        DEBUG_PRINTF("Fast encoder: %d steps (%lu ms)\n", 
                    encoder_speed_step_size, smoothed_time);
        break;
      case 1: // Medium
        encoder_speed_step_size = ENCODER_STEP_MEDIUM; // 10
        ENCODER_DEBUG_PRINTF("Medium encoder: %d steps (%lu ms)\n", 
                    encoder_speed_step_size, smoothed_time);
        break;
      case 2: // Slow
      default:
        encoder_speed_step_size = ENCODER_STEP_SLOW;   // 1
        ENCODER_DEBUG_PRINTF("Slow encoder: %d steps (%lu ms)\n", 
                    encoder_speed_step_size, smoothed_time);
        break;
    }
    
    return delta * encoder_speed_step_size;
  }
  
  return 0;
}

bool is_encoder_button_pressed() {
  if (encoder_button_pressed) {
    encoder_button_pressed = false;
    return true;
  }
  return false;
}

// =============================================================================
// EXISTING DISPLAY IMPLEMENTATION - UNCHANGED
// =============================================================================
void lcd_reg_init(void) {
  static const uint8_t init_operations[] = {
    BEGIN_WRITE,
    WRITE_COMMAND_8, 0x11,  
    END_WRITE,
    DELAY, 120,

    BEGIN_WRITE,
    WRITE_C8_D16, 0xDF, 0x98, 0x53,
    WRITE_C8_D8, 0xB2, 0x23, 

    WRITE_COMMAND_8, 0xB7,
    WRITE_BYTES, 4,
    0x00, 0x47, 0x00, 0x6F,

    WRITE_COMMAND_8, 0xBB,
    WRITE_BYTES, 6,
    0x1C, 0x1A, 0x55, 0x73, 0x63, 0xF0,

    WRITE_C8_D16, 0xC0, 0x44, 0xA4,
    WRITE_C8_D8, 0xC1, 0x16, 

    WRITE_COMMAND_8, 0xC3,
    WRITE_BYTES, 8,
    0x7D, 0x07, 0x14, 0x06, 0xCF, 0x71, 0x72, 0x77,

    WRITE_COMMAND_8, 0xC4,
    WRITE_BYTES, 12,
    0x00, 0x00, 0xA0, 0x79, 0x0B, 0x0A, 0x16, 0x79, 0x0B, 0x0A, 0x16, 0x82,

    WRITE_COMMAND_8, 0xC8,
    WRITE_BYTES, 32,
    0x3F, 0x32, 0x29, 0x29, 0x27, 0x2B, 0x27, 0x28, 0x28, 0x26, 0x25, 0x17, 0x12, 0x0D, 0x04, 0x00, 0x3F, 0x32, 0x29, 0x29, 0x27, 0x2B, 0x27, 0x28, 0x28, 0x26, 0x25, 0x17, 0x12, 0x0D, 0x04, 0x00,

    WRITE_COMMAND_8, 0xD0,
    WRITE_BYTES, 5,
    0x04, 0x06, 0x6B, 0x0F, 0x00,

    WRITE_C8_D16, 0xD7, 0x00, 0x30,
    WRITE_C8_D8, 0xE6, 0x14, 
    WRITE_C8_D8, 0xDE, 0x01, 

    WRITE_COMMAND_8, 0xB7,
    WRITE_BYTES, 5,
    0x03, 0x13, 0xEF, 0x35, 0x35,

    WRITE_COMMAND_8, 0xC1,
    WRITE_BYTES, 3,
    0x14, 0x15, 0xC0,

    WRITE_C8_D16, 0xC2, 0x06, 0x3A,
    WRITE_C8_D16, 0xC4, 0x72, 0x12,
    WRITE_C8_D8, 0xBE, 0x00, 
    WRITE_C8_D8, 0xDE, 0x02, 

    WRITE_COMMAND_8, 0xE5,
    WRITE_BYTES, 3,
    0x00, 0x02, 0x00,

    WRITE_COMMAND_8, 0xE5,
    WRITE_BYTES, 3,
    0x01, 0x02, 0x00,

    WRITE_C8_D8, 0xDE, 0x00, 
    WRITE_C8_D8, 0x35, 0x00, 
    WRITE_C8_D8, 0x3A, 0x05, 

    WRITE_COMMAND_8, 0x2A,
    WRITE_BYTES, 4,
    0x00, 0x22, 0x00, 0xCD,

    WRITE_COMMAND_8, 0x2B,
    WRITE_BYTES, 4,
    0x00, 0x00, 0x01, 0x3F,

    WRITE_C8_D8, 0xDE, 0x02, 

    WRITE_COMMAND_8, 0xE5,
    WRITE_BYTES, 3,
    0x00, 0x02, 0x00,
    
    WRITE_C8_D8, 0xDE, 0x00, 
    WRITE_C8_D8, 0x36, 0x00,
    WRITE_COMMAND_8, 0x21,
    END_WRITE,
    
    DELAY, 10,

    BEGIN_WRITE,
    WRITE_COMMAND_8, 0x29,
    END_WRITE
  };
  bus->batchOperation(init_operations, sizeof(init_operations));
}

#if LV_USE_LOG != 0
void my_print(const char *buf) {
  Serial.printf(buf);
  Serial.flush();
}
#endif

void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

  lv_disp_flush_ready(disp_drv);
}

void touchpad_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
  touch_data_t touch_data;
  uint8_t touchpad_cnt = 0;

  bsp_touch_read();
  bool touchpad_pressed = bsp_touch_get_coordinates(&touch_data);

  if (touchpad_pressed) {
    data->point.x = touch_data.coords[0].x;
    data->point.y = touch_data.coords[0].y;
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void hardware_init() {
  DEBUG_PRINTLN("Initializing hardware...");
  
  // Init Display
  if (!gfx->begin()) {
    DEBUG_PRINTLN("ERROR: gfx->begin() failed!");
    return;
  }
  lcd_reg_init();
  gfx->setRotation(ROTATION);
  gfx->fillScreen(RGB565_BLACK);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  // Init touch device
  Wire.begin(Touch_I2C_SDA, Touch_I2C_SCL);
  bsp_touch_init(&Wire, Touch_RST, Touch_INT, gfx->getRotation(), gfx->width(), gfx->height());
  lv_init();

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  screenWidth = gfx->width();
  screenHeight = gfx->height();
  bufSize = screenWidth * 40;

#ifdef ESP32
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!disp_draw_buf) {
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_8BIT);
  }
#else
  DEBUG_PRINTLN("LVGL disp_draw_buf heap_caps_malloc failed! malloc again...");
  disp_draw_buf = (lv_color_t *)malloc(bufSize * 2);
#endif

  if (!disp_draw_buf) {
    DEBUG_PRINTLN("ERROR: LVGL disp_draw_buf allocate failed!");
    return;
  }
  
  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, bufSize);

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read_cb;
  lv_indev_drv_register(&indev_drv);
  
  // Initialize rotary encoder with extended features
  encoder_init_extended();
  
  DEBUG_PRINTLN("Hardware initialized successfully!");
}

#endif // HARDWARE_H