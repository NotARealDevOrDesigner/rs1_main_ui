/*
=============================================================================
battery.h - Battery Management System with MAX17048 Integration
=============================================================================
*/

#ifndef BATTERY_H
#define BATTERY_H

#include "config.h"
#include <lvgl.h>
#include <Wire.h>

// =============================================================================
// CONFIGURATION
// =============================================================================
#define MAX17048_ADDRESS    0x36
#define MAX17048_SOC        0x04
#define MAX17048_VERSION    0x08
#define CHARGE_PIN          3
#define POWER_SWITCH_PIN    4

#define BATTERY_WIDGET_WIDTH  28
#define BATTERY_WIDGET_HEIGHT 16
#define BATTERY_FRAME_WIDTH   24
#define BATTERY_FRAME_HEIGHT  12
#define BATTERY_TERMINAL_WIDTH 2
#define BATTERY_TERMINAL_HEIGHT 6
#define BATTERY_FILL_MAX_WIDTH  20
#define BATTERY_FILL_HEIGHT     8
#define BATTERY_FILL_OFFSET_X   0
#define BATTERY_FILL_OFFSET_Y   0

// =============================================================================
// BATTERY ICON DATA
// =============================================================================
static const uint8_t battery_icon_data[] = {
    0xFF, 0xFF, 0xFF, 0xF0, 0x80, 0x00, 0x00, 0x18, 0x80, 0x00, 0x00, 0x18,
    0x80, 0x00, 0x00, 0x18, 0x80, 0x00, 0x00, 0x18, 0x80, 0x00, 0x00, 0x18,
    0x80, 0x00, 0x00, 0x18, 0x80, 0x00, 0x00, 0x18, 0x80, 0x00, 0x00, 0x18,
    0x80, 0x00, 0x00, 0x18, 0x80, 0x00, 0x00, 0x18, 0x80, 0x00, 0x00, 0x18,
    0x80, 0x00, 0x00, 0x18, 0x80, 0x00, 0x00, 0x18, 0xFF, 0xFF, 0xFF, 0xF0
};

const lv_img_dsc_t icon_battery = {
    {LV_IMG_CF_ALPHA_1BIT, 0, 0, BATTERY_WIDGET_WIDTH, BATTERY_WIDGET_HEIGHT},
    sizeof(battery_icon_data), battery_icon_data
};

// =============================================================================
// DATA STRUCTURES
// =============================================================================
typedef struct {
    uint8_t level, real_level;
    bool is_charging, is_power_switch_on, show_percentage, max17048_available;
    int system_state; // 0=normal, 1=charging_anim, 2=charging_overlay, 3=demo, 4=off
} battery_info_t;

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================
extern battery_info_t battery_state;
extern unsigned long last_battery_update, last_battery_animation;
extern bool battery_demo_enabled, battery_animation_increasing;
extern lv_obj_t *charging_overlay, *off_screen;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================
void battery_init();
void battery_system_update();
void battery_set_level(uint8_t level);
void battery_set_charging(bool charging);
uint8_t battery_get_level();
bool battery_is_charging();
lv_obj_t* create_battery_widget(lv_obj_t *parent, lv_coord_t x, lv_coord_t y);
void update_battery_widget(lv_obj_t *battery_widget);
void update_all_battery_widgets();
lv_color_t get_battery_color(uint8_t level);
void set_real_battery_level(uint8_t level);
void toggle_battery_demo(bool enable);
void print_battery_status();
void handle_battery_serial_commands(String command);

// =============================================================================
// IMPLEMENTATION
// =============================================================================
battery_info_t battery_state = {85, 85, false, true, false, false, 3};
unsigned long last_battery_update = 0, last_battery_animation = 0;
bool battery_demo_enabled = true, battery_animation_increasing = true;
lv_obj_t *charging_overlay = nullptr, *off_screen = nullptr;

// MAX17048 Functions
uint16_t max17048_read_register(uint8_t reg) {
    Wire.beginTransmission(MAX17048_ADDRESS);
    Wire.write(reg);
    if (Wire.endTransmission() != 0 || Wire.requestFrom(MAX17048_ADDRESS, 2) != 2) return 0xFFFF;
    return (Wire.read() << 8) | Wire.read();
}

bool max17048_init() {
    uint16_t version = max17048_read_register(MAX17048_VERSION);
    return (version != 0xFFFF && version != 0x0000);
}

bool max17048_read_soc(uint8_t &soc) {
    uint16_t raw_soc = max17048_read_register(MAX17048_SOC);
    if (raw_soc == 0xFFFF) return false;
    soc = (raw_soc >> 8) & 0xFF;
    if (soc > 100) soc = 100;
    return true;
}

// GPIO Functions
bool read_charging_status_hw() { return (digitalRead(CHARGE_PIN) == LOW); }
bool read_power_switch_status_hw() { return (digitalRead(POWER_SWITCH_PIN) == HIGH); }

// Overlay Functions
void create_charging_overlay() {
    charging_overlay = lv_obj_create(lv_scr_act());
    lv_obj_set_size(charging_overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(charging_overlay, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(charging_overlay, 0, 0);
    lv_obj_set_style_pad_all(charging_overlay, 0, 0);
    lv_obj_add_flag(charging_overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(charging_overlay, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t *label = lv_label_create(charging_overlay);
    lv_label_set_text(label, "Charging...");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0);
    lv_obj_center(label);
}

void create_off_screen() {
    off_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(off_screen, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(off_screen, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(off_screen, 0, 0);
    lv_obj_set_style_pad_all(off_screen, 0, 0);
    lv_obj_add_flag(off_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(off_screen, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t *label = lv_label_create(off_screen);
    lv_label_set_text(label, "Off");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0);
    lv_obj_center(label);
}

void show_charging_overlay() {
    if (charging_overlay) {
        lv_obj_clear_flag(charging_overlay, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(charging_overlay);
    }
}

void hide_charging_overlay() {
    if (charging_overlay) lv_obj_add_flag(charging_overlay, LV_OBJ_FLAG_HIDDEN);
}

void show_off_screen() {
    if (off_screen) {
        lv_obj_clear_flag(off_screen, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(off_screen);
    }
}

void hide_off_screen() {
    if (off_screen) lv_obj_add_flag(off_screen, LV_OBJ_FLAG_HIDDEN);
}

// Core Functions
void battery_init() {
    battery_state = {85, 85, false, true, false, false, 3};
    pinMode(CHARGE_PIN, INPUT_PULLUP);
    pinMode(POWER_SWITCH_PIN, INPUT_PULLUP);
    
    if (max17048_init()) {
        battery_state.max17048_available = true;
        battery_state.system_state = 0;
    }
    
    create_charging_overlay();
    create_off_screen();
}

void battery_set_level(uint8_t level) {
    battery_state.level = (level > 100) ? 100 : level;
}

void battery_set_charging(bool charging) { battery_state.is_charging = charging; }
uint8_t battery_get_level() { return battery_state.level; }
bool battery_is_charging() { return battery_state.is_charging; }

lv_color_t get_battery_color(uint8_t level) {
    return battery_is_charging() ? lv_color_hex(COLOR_BATTERY_LOAD) : lv_color_hex(COLOR_TEXT_PRIMARY);
}

lv_obj_t* create_battery_widget(lv_obj_t *parent, lv_coord_t x, lv_coord_t y) {
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, BATTERY_WIDGET_WIDTH, BATTERY_WIDGET_HEIGHT);
    lv_obj_set_pos(container, x, y);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *frame = lv_obj_create(container);
    lv_obj_set_size(frame, BATTERY_FRAME_WIDTH, BATTERY_FRAME_HEIGHT);
    lv_obj_set_pos(frame, 0, 2);
    lv_obj_set_style_bg_color(frame, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(frame, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
    lv_obj_set_style_border_width(frame, 2, 0);
    lv_obj_set_style_radius(frame, 2, 0);
    lv_obj_set_style_pad_all(frame, 0, 0);
    lv_obj_set_scrollbar_mode(frame, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *terminal = lv_obj_create(container);
    lv_obj_set_size(terminal, BATTERY_TERMINAL_WIDTH, BATTERY_TERMINAL_HEIGHT);
    lv_obj_set_pos(terminal, BATTERY_FRAME_WIDTH, (BATTERY_WIDGET_HEIGHT - BATTERY_TERMINAL_HEIGHT) / 2);
    lv_obj_set_style_bg_color(terminal, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
    lv_obj_set_style_bg_opa(terminal, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(terminal, 0, 0);
    lv_obj_set_style_radius(terminal, 0, 0);
    lv_obj_set_scrollbar_mode(terminal, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(terminal, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *fill = lv_obj_create(frame);
    lv_obj_set_pos(fill, BATTERY_FILL_OFFSET_X, BATTERY_FILL_OFFSET_Y);
    lv_obj_set_size(fill, BATTERY_FILL_MAX_WIDTH, BATTERY_FILL_HEIGHT);
    lv_obj_set_style_bg_color(fill, get_battery_color(battery_get_level()), 0);
    lv_obj_set_style_bg_opa(fill, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(fill, 0, 0);
    lv_obj_set_style_radius(fill, 0, 0);
    lv_obj_set_style_pad_all(fill, 0, 0);
    lv_obj_set_scrollbar_mode(fill, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(fill, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(fill);
    lv_obj_clear_flag(fill, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_set_user_data(container, fill);
    update_battery_widget(container);
    return container;
}

void update_battery_widget(lv_obj_t *battery_widget) {
    if (!battery_widget) return;
    lv_obj_t *fill = (lv_obj_t*)lv_obj_get_user_data(battery_widget);
    if (!fill) return;

    uint8_t level = battery_get_level();
    lv_coord_t fill_width = (level * BATTERY_FILL_MAX_WIDTH) / 100;
    if (fill_width < 1 && level > 0) fill_width = 1;
    
    lv_obj_set_width(fill, fill_width);
    lv_obj_set_style_bg_color(fill, get_battery_color(level), 0);
}

void battery_system_update() {
    unsigned long current_time = millis();
    if (current_time - last_battery_update <= 500) return;
    
    last_battery_update = current_time;
    
    bool charging = read_charging_status_hw();
    bool power_switch_on = read_power_switch_status_hw();
    battery_state.is_charging = charging;
    battery_state.is_power_switch_on = power_switch_on;
    
    // Determine system state
    if (!charging && !power_switch_on) {
        battery_state.system_state = 4; // Off screen
    } else if (charging && !power_switch_on) {
        battery_state.system_state = 2; // Charging overlay
    } else if (!charging && power_switch_on) {
        battery_state.system_state = battery_state.max17048_available ? 0 : 3;
    } else if (charging && power_switch_on) {
        battery_state.system_state = 1; // Charging animation
    }
    
    // Handle states
    switch (battery_state.system_state) {
        case 0: // Normal operation
            hide_charging_overlay(); hide_off_screen();
            if (battery_state.max17048_available) {
                uint8_t real_soc;
                if (max17048_read_soc(real_soc)) {
                    battery_state.real_level = real_soc;
                    battery_set_level(real_soc);
                } else {
                    battery_state.system_state = 3;
                    battery_state.max17048_available = false;
                }
            }
            break;
            
        case 1: // Charging animation
            hide_charging_overlay(); hide_off_screen();
            if (current_time - last_battery_animation > 800) {
                last_battery_animation = current_time;
                uint8_t real_soc = battery_state.real_level;
                if (battery_state.max17048_available) {
                    if (max17048_read_soc(real_soc)) battery_state.real_level = real_soc;
                }
                
                uint8_t current_display = battery_state.level;
                uint8_t new_level = battery_animation_increasing ? current_display + 3 : current_display - 3;
                
                if (battery_animation_increasing && new_level >= 100) {
                    new_level = 100; battery_animation_increasing = false;
                } else if (!battery_animation_increasing && new_level <= real_soc) {
                    new_level = real_soc; battery_animation_increasing = true;
                }
                battery_set_level(new_level);
            }
            break;
            
        case 2: // Charging overlay
            show_charging_overlay(); hide_off_screen();
            if (battery_state.max17048_available) {
                uint8_t real_soc;
                if (max17048_read_soc(real_soc)) {
                    battery_state.real_level = real_soc;
                    battery_set_level(real_soc);
                }
            }
            break;
            
        case 3: // Demo mode
            hide_charging_overlay(); hide_off_screen();
            if (battery_demo_enabled && (current_time - last_battery_animation > 2000)) {
                last_battery_animation = current_time;
                uint8_t current_level = battery_get_level();
                uint8_t new_level = battery_animation_increasing ? current_level + 5 : current_level - 5;
                
                if (battery_animation_increasing && new_level >= 100) {
                    new_level = 100; battery_animation_increasing = false;
                } else if (!battery_animation_increasing && new_level <= 10) {
                    new_level = 10; battery_animation_increasing = true;
                }
                battery_set_level(new_level);
            }
            break;
            
        case 4: // Off screen
        default:
            hide_charging_overlay(); show_off_screen();
            if (battery_state.max17048_available) {
                uint8_t real_soc;
                if (max17048_read_soc(real_soc)) {
                    battery_state.real_level = real_soc;
                    battery_set_level(real_soc);
                }
            }
            break;
    }
    
    update_all_battery_widgets();
}

void set_real_battery_level(uint8_t level) {
    battery_demo_enabled = false;
    battery_state.system_state = 3;
    battery_set_level(level);
    update_all_battery_widgets();
}

void toggle_battery_demo(bool enable) {
    battery_demo_enabled = enable;
    if (enable) {
        battery_state.system_state = 3;
    } else if (battery_state.max17048_available) {
        battery_state.system_state = 0;
    }
}

void print_battery_status() {
    const char* state_names[] = {"Normal", "Charging Animation", "Charging Overlay", "Demo", "Off Screen"};
    Serial.println("=== Battery Status ===");
    Serial.printf("Display Level: %d%%, Real Level: %d%%\n", battery_get_level(), battery_state.real_level);
    Serial.printf("Charging: %s, Switch: %s\n", battery_is_charging() ? "Yes" : "No", 
                  battery_state.is_power_switch_on ? "On" : "Off");
    Serial.printf("MAX17048: %s\n", battery_state.max17048_available ? "Available" : "Not available");
    Serial.printf("System State: %s\n", state_names[battery_state.system_state]);
    Serial.printf("GPIO %d: %s, GPIO %d: %s\n", CHARGE_PIN, 
                  read_charging_status_hw() ? "CHARGING" : "NOT CHARGING",
                  POWER_SWITCH_PIN, read_power_switch_status_hw() ? "ON" : "OFF");
    Serial.println("======================");
}

void handle_battery_serial_commands(String command) {
    if (command.startsWith("bat ")) {
        int level = command.substring(4).toInt();
        if (level >= 0 && level <= 100) {
            set_real_battery_level(level);
            Serial.printf("Battery level set to %d%%\n", level);
        }
    }
    else if (command == "bat demo on") { toggle_battery_demo(true); Serial.println("Demo enabled"); }
    else if (command == "bat demo off") { toggle_battery_demo(false); Serial.println("Demo disabled"); }
    else if (command == "bat status") { print_battery_status(); }
    else if (command == "bat charge on") { battery_set_charging(true); Serial.println("Charging: ON"); }
    else if (command == "bat charge off") { battery_set_charging(false); Serial.println("Charging: OFF"); }
    else if (command == "bat max17048") {
        if (battery_state.max17048_available) {
            uint8_t soc;
            Serial.printf("MAX17048 SOC: %s\n", max17048_read_soc(soc) ? String(soc + "%").c_str() : "Error");
        } else {
            Serial.println("MAX17048 not available");
        }
    }
    else if (command == "bat pins") {
        Serial.printf("GPIO %d: %s, GPIO %d: %s\n", CHARGE_PIN,
                     digitalRead(CHARGE_PIN) ? "HIGH" : "LOW", POWER_SWITCH_PIN,
                     digitalRead(POWER_SWITCH_PIN) ? "HIGH" : "LOW");
    }
    else if (command == "bat state") {
        const char* state_names[] = {"Normal", "Charging Animation", "Charging Overlay", "Demo", "Off Screen"};
        Serial.printf("System State: %s\n", state_names[battery_state.system_state]);
    }
}

#endif // BATTERY_H