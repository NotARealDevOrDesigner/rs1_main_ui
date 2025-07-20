/*
=============================================================================
battery.h - Complete Battery Management System für ESP32-C6 Camera Control App
=============================================================================
*/

#ifndef BATTERY_H
#define BATTERY_H

#include "config.h"
#include <lvgl.h>

// =============================================================================
// BATTERY ICON CONFIGURATION
// =============================================================================

// Battery Icon Dimensionen - HIER KANNST DU DIE GRÖSSE ANPASSEN
#define BATTERY_WIDGET_WIDTH  28    // Gesamtbreite des Battery Widgets
#define BATTERY_WIDGET_HEIGHT 16    // Gesamthöhe des Battery Widgets
#define BATTERY_FRAME_WIDTH   24    // Breite des Hauptrahmens
#define BATTERY_FRAME_HEIGHT  12    // Höhe des Hauptrahmens
#define BATTERY_TERMINAL_WIDTH 2    // Breite des rechten Terminals
#define BATTERY_TERMINAL_HEIGHT 6   // Höhe des rechten Terminals

// Battery Fill Dimensionen - HIER KANNST DU DAS FILL-RECHTECK ANPASSEN
#define BATTERY_FILL_MAX_WIDTH  20  // Maximale Breite des Fills (bei 100%)
#define BATTERY_FILL_HEIGHT     8   // Höhe des Fills
#define BATTERY_FILL_OFFSET_X   2   // X-Position des Fills im Rahmen (von links)
#define BATTERY_FILL_OFFSET_Y   2   // Y-Position des Fills im Rahmen (von oben)

// =============================================================================
// BATTERY ICON DATA
// =============================================================================

static const uint8_t battery_icon_data[] = {
    // Custom Clean Battery - minimalistisches Design
    // Schwarzer Rahmen, weißer Hintergrund, schwarzer Fill
    0xFF, 0xFF, 0xFF, 0xF0,  // Top border
    0x80, 0x00, 0x00, 0x18,  // Left border + right terminal space
    0x80, 0x00, 0x00, 0x18,  // 
    0x80, 0x00, 0x00, 0x18,  // Fill area (wird programmatisch gefüllt)
    0x80, 0x00, 0x00, 0x18,  // 
    0x80, 0x00, 0x00, 0x18,  // 
    0x80, 0x00, 0x00, 0x18,  // 
    0x80, 0x00, 0x00, 0x18,  // 
    0x80, 0x00, 0x00, 0x18,  // 
    0x80, 0x00, 0x00, 0x18,  // 
    0x80, 0x00, 0x00, 0x18,  // 
    0x80, 0x00, 0x00, 0x18,  // 
    0x80, 0x00, 0x00, 0x18,  // 
    0x80, 0x00, 0x00, 0x18,  // Bottom area
    0xFF, 0xFF, 0xFF, 0xF0   // Bottom border
};

const lv_img_dsc_t icon_battery = {
    {LV_IMG_CF_ALPHA_1BIT, 0, 0, BATTERY_WIDGET_WIDTH, BATTERY_WIDGET_HEIGHT},
    sizeof(battery_icon_data),
    battery_icon_data
};

// =============================================================================
// BATTERY DATA STRUCTURES
// =============================================================================

typedef struct {
    uint8_t level;        // 0-100 percent
    bool is_charging;     // Charging status
    bool show_percentage; // Show percentage text
} battery_info_t;

// =============================================================================
// GLOBAL BATTERY STATE
// =============================================================================

// Battery state (wird in battery_init() initialisiert)
extern battery_info_t battery_state;

// Battery system variables
extern unsigned long last_battery_update;
extern unsigned long last_battery_animation;
extern bool battery_demo_enabled;
extern bool battery_animation_increasing;

// =============================================================================
// CORE BATTERY FUNCTIONS
// =============================================================================

// Initialization
void battery_init();

// State management
void battery_set_level(uint8_t level);
void battery_set_charging(bool charging);
uint8_t battery_get_level();
bool battery_is_charging();

// System updates
void battery_system_update();
void set_real_battery_level(uint8_t level);
void toggle_battery_demo(bool enable);
void print_battery_status();

// =============================================================================
// UI FUNCTIONS
// =============================================================================

// Widget creation and updates
lv_obj_t* create_battery_widget(lv_obj_t *parent, lv_coord_t x, lv_coord_t y);
void update_battery_widget(lv_obj_t *battery_widget);
void update_all_battery_widgets(); // Diese Funktion muss in ui.h implementiert werden

// Visual helpers
lv_color_t get_battery_color(uint8_t level);

// Advanced positioning functions
void battery_set_fill_position(lv_obj_t *battery_widget, lv_coord_t x, lv_coord_t y);
void battery_set_fill_height(lv_obj_t *battery_widget, lv_coord_t height);
void battery_set_fill_max_width(lv_obj_t *battery_widget, lv_coord_t max_width);

// =============================================================================
// HARDWARE INTEGRATION
// =============================================================================

// Hardware reading functions (to be implemented)
uint8_t read_battery_voltage();
bool read_charging_status();
void update_battery_from_hardware();

// =============================================================================
// TESTING AND DEBUG FUNCTIONS
// =============================================================================

// Demo and testing
void battery_test_animation();
void battery_debug_positions();

// Serial command handling
void handle_battery_serial_commands(String command);

// =============================================================================
// IMPLEMENTATION
// =============================================================================

// Battery state (wird in battery_init() initialisiert)
battery_info_t battery_state = {85, false, false}; // Default: 85%, not charging

// Battery system variables
unsigned long last_battery_update = 0;
unsigned long last_battery_animation = 0;
bool battery_demo_enabled = true; // Für Testing - später deaktivieren
bool battery_animation_increasing = true;

// =============================================================================
// CORE FUNCTIONS IMPLEMENTATION
// =============================================================================

void battery_init() {
    battery_state.level = 85;
    battery_state.is_charging = false;
    battery_state.show_percentage = false;
    DEBUG_PRINTF("Battery system initialized - Level: %d%%\n", battery_get_level());
}

void battery_set_level(uint8_t level) {
    if (level > 100) level = 100;
    battery_state.level = level;
}

void battery_set_charging(bool charging) {
    battery_state.is_charging = charging;
}

uint8_t battery_get_level() {
    return battery_state.level;
}

bool battery_is_charging() {
    return battery_state.is_charging;
}

// =============================================================================
// VISUAL FUNCTIONS IMPLEMENTATION
// =============================================================================

// Farbe basierend auf Battery Level - Custom Clean Style
lv_color_t get_battery_color(uint8_t level) {
    // Immer schwarz für normalen Betrieb
    // Spezialfall: Grün wenn geladen wird (zukünftiges Feature)
    if (battery_is_charging()) {
        return lv_color_hex(COLOR_BATTERY_LOAD); // Grün für Charging (zukünftiges Feature)
    }
    
    // Standard: Immer schwarz - dein cleaner Look
    return lv_color_hex(COLOR_TEXT_PRIMARY); // Schwarz (#000000)
}

// Custom Clean Battery Widget erstellen (für Header)
lv_obj_t* create_battery_widget(lv_obj_t *parent, lv_coord_t x, lv_coord_t y) {
    // Container für Battery - Custom Clean Größe
    lv_obj_t *battery_container = lv_obj_create(parent);
    lv_obj_set_size(battery_container, BATTERY_WIDGET_WIDTH, BATTERY_WIDGET_HEIGHT);
    lv_obj_set_pos(battery_container, x, y);
    lv_obj_set_style_bg_opa(battery_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(battery_container, 0, 0);
    lv_obj_set_style_pad_all(battery_container, 0, 0);

    // Battery Rahmen (äußere Hülle) - Custom Clean Style
    lv_obj_t *battery_frame = lv_obj_create(battery_container);
    lv_obj_set_size(battery_frame, BATTERY_FRAME_WIDTH, BATTERY_FRAME_HEIGHT);
    lv_obj_set_pos(battery_frame, 0, 2); // Vertikal zentriert
    lv_obj_set_style_bg_color(battery_frame, lv_color_hex(0xFFFFFF), 0);    // Weißer Hintergrund
    lv_obj_set_style_bg_opa(battery_frame, LV_OPA_COVER, 0);                // Volldeckend
    lv_obj_set_style_border_color(battery_frame, lv_color_hex(COLOR_TEXT_PRIMARY), 0); // Schwarzer Rahmen
    lv_obj_set_style_border_width(battery_frame, 2, 0);                     // 2px Rahmen
    lv_obj_set_style_radius(battery_frame, 2, 0);                           // Leicht abgerundet

    // Battery Terminal (rechts) - Custom Clean Style
    lv_obj_t *battery_terminal = lv_obj_create(battery_container);
    lv_obj_set_size(battery_terminal, BATTERY_TERMINAL_WIDTH, BATTERY_TERMINAL_HEIGHT);
    lv_obj_set_pos(battery_terminal, BATTERY_FRAME_WIDTH, (BATTERY_WIDGET_HEIGHT - BATTERY_TERMINAL_HEIGHT) / 2); // Zentriert
    lv_obj_set_style_bg_color(battery_terminal, lv_color_hex(COLOR_TEXT_PRIMARY), 0); // Schwarzer Terminal
    lv_obj_set_style_border_width(battery_terminal, 0, 0);
    lv_obj_set_style_radius(battery_terminal, 0, 0);                        // Eckig, wie dein Design

    // ========================================================================
    // BATTERY FILL - HIER KANNST DU DIE POSITION ANPASSEN
    // ========================================================================
    lv_obj_t *battery_fill = lv_obj_create(battery_frame);
    
    // POSITION DES FILLS - HIER ANPASSEN:
    lv_obj_set_pos(battery_fill, BATTERY_FILL_OFFSET_X, BATTERY_FILL_OFFSET_Y);
    
    // GRÖSSE DES FILLS - HIER ANPASSEN:
    lv_obj_set_size(battery_fill, BATTERY_FILL_MAX_WIDTH, BATTERY_FILL_HEIGHT);
    
    // STYLE DES FILLS:
    lv_obj_set_style_bg_color(battery_fill, get_battery_color(battery_get_level()), 0);
    lv_obj_set_style_border_width(battery_fill, 0, 0);
    lv_obj_set_style_radius(battery_fill, 0, 0);                            // Eckig, clean look

    // User data speichern für Updates
    lv_obj_set_user_data(battery_container, battery_fill);

    // Initial update
    update_battery_widget(battery_container);

    return battery_container;
}

// Battery Widget updaten - Custom Clean Style
void update_battery_widget(lv_obj_t *battery_widget) {
    if (!battery_widget) return;
    
    lv_obj_t *battery_fill = (lv_obj_t*)lv_obj_get_user_data(battery_widget);
    if (!battery_fill) return;

    uint8_t level = battery_get_level();
    
    // ========================================================================
    // FILL WIDTH BERECHNUNG - HIER KANNST DU DIE SKALIERUNG ANPASSEN
    // ========================================================================
    
    // Fill width berechnen (0 bis BATTERY_FILL_MAX_WIDTH pixels)
    lv_coord_t fill_width = (level * BATTERY_FILL_MAX_WIDTH) / 100;
    
    // Mindestens 1 pixel wenn level > 0 (optinal, kannst du entfernen)
    if (fill_width < 1 && level > 0) fill_width = 1;
    
    // FILL BREITE SETZEN:
    lv_obj_set_width(battery_fill, fill_width);
    
    // FILL FARBE AKTUALISIEREN:
    lv_obj_set_style_bg_color(battery_fill, get_battery_color(level), 0);
    
    // ========================================================================
    // SPEZIELLE EFFEKTE - HIER KANNST DU ANIMATIONEN HINZUFÜGEN
    // ========================================================================
    
    // Charging Animation (zukünftiges Feature)
    if (battery_is_charging()) {
        // Hier könntest du eine Pulse-Animation implementieren
        // Beispiel: lv_obj_set_style_bg_opa mit Animation
        // Aktuell: Nur Farbwechsel zu Grün
    }
    
    // Critical Level Indication (optional)
    if (level < 15) {
        // Hier könntest du ein Blink-Effekt implementieren
        // Aktuell keine spezielle Anzeige da Fill immer schwarz ist
    }
}

// =============================================================================
// SYSTEM UPDATE FUNCTIONS
// =============================================================================

void battery_system_update() {
    unsigned long current_time = millis();
    
    // Demo Animation für Testing (alle 2 Sekunden)
    if (battery_demo_enabled && (current_time - last_battery_animation > 2000)) {
        last_battery_animation = current_time;
        
        uint8_t current_level = battery_get_level();
        uint8_t new_level;
        
        if (battery_animation_increasing) {
            new_level = current_level + 5;
            if (new_level >= 100) {
                new_level = 100;
                battery_animation_increasing = false;
            }
        } else {
            new_level = current_level - 5;
            if (new_level <= 10) {
                new_level = 10;
                battery_animation_increasing = true;
            }
        }
        
        battery_set_level(new_level);
        DEBUG_PRINTF("Battery Demo: Level set to %d%%\n", new_level);
    }
    
    // Battery Widget Updates (alle 500ms für smooth animation)
    if (current_time - last_battery_update > 500) {
        last_battery_update = current_time;
        update_all_battery_widgets();
    }
}

// Funktion zum Setzen des echten Battery Levels (später von Hardware)
void set_real_battery_level(uint8_t level) {
    battery_demo_enabled = false; // Demo deaktivieren
    battery_set_level(level);
    update_all_battery_widgets();
    DEBUG_PRINTF("Real battery level set: %d%%\n", level);
}

// Funktion zum Aktivieren/Deaktivieren der Demo
void toggle_battery_demo(bool enable) {
    battery_demo_enabled = enable;
    if (enable) {
        DEBUG_PRINTLN("Battery demo animation enabled");
    } else {
        DEBUG_PRINTLN("Battery demo animation disabled");
    }
}

// Battery Status für Debugging ausgeben
void print_battery_status() {
    DEBUG_PRINTLN("=== Battery Status ===");
    DEBUG_PRINTF("Level: %d%%\n", battery_get_level());
    DEBUG_PRINTF("Charging: %s\n", battery_is_charging() ? "Yes" : "No");
    DEBUG_PRINTF("Demo Mode: %s\n", battery_demo_enabled ? "Enabled" : "Disabled");
    DEBUG_PRINTF("Color: 0x%06X\n", get_battery_color(battery_get_level()).full);
    DEBUG_PRINTLN("======================");
}

// =============================================================================
// HARDWARE INTEGRATION IMPLEMENTATION
// =============================================================================

uint8_t read_battery_voltage() {
    // TODO: Echte ADC-Lesung implementieren
    // Beispiel: ADC lesen und in Prozent umrechnen
    // return map(analogRead(BATTERY_PIN), 0, 4095, 0, 100);
    return 85; // Placeholder
}

bool read_charging_status() {
    // TODO: Charging Pin lesen
    // return digitalRead(CHARGING_PIN);
    return false; // Placeholder
}

void update_battery_from_hardware() {
    // Diese Funktion kann periodisch aufgerufen werden, um echte Werte zu lesen
    uint8_t hw_level = read_battery_voltage();
    bool hw_charging = read_charging_status();
    
    battery_set_level(hw_level);
    battery_set_charging(hw_charging);
    
    DEBUG_PRINTF("Hardware battery update: %d%% (Charging: %s)\n", 
                 hw_level, hw_charging ? "Yes" : "No");
}

// =============================================================================
// POSITION HELPER FUNCTIONS IMPLEMENTATION
// =============================================================================

// Fill Position zur Laufzeit ändern (falls gewünscht)
void battery_set_fill_position(lv_obj_t *battery_widget, lv_coord_t x, lv_coord_t y) {
    if (!battery_widget) return;
    
    lv_obj_t *battery_fill = (lv_obj_t*)lv_obj_get_user_data(battery_widget);
    if (!battery_fill) return;
    
    lv_obj_set_pos(battery_fill, x, y);
}

// Fill Höhe zur Laufzeit ändern (falls gewünscht)
void battery_set_fill_height(lv_obj_t *battery_widget, lv_coord_t height) {
    if (!battery_widget) return;
    
    lv_obj_t *battery_fill = (lv_obj_t*)lv_obj_get_user_data(battery_widget);
    if (!battery_fill) return;
    
    lv_obj_set_height(battery_fill, height);
}

// Fill Max-Width zur Laufzeit ändern (falls gewünscht)
void battery_set_fill_max_width(lv_obj_t *battery_widget, lv_coord_t max_width) {
    if (!battery_widget) return;
    
    lv_obj_t *battery_fill = (lv_obj_t*)lv_obj_get_user_data(battery_widget);
    if (!battery_fill) return;
    
    // Aktuelle Level beibehalten, aber neue Max-Width verwenden
    uint8_t level = battery_get_level();
    lv_coord_t new_width = (level * max_width) / 100;
    lv_obj_set_width(battery_fill, new_width);
}

// =============================================================================
// TESTING AND DEBUG IMPLEMENTATION
// =============================================================================

// Test-Funktion für Battery Animation - Custom Clean Style
void battery_test_animation() {
    static uint8_t test_level = 0;
    static bool increasing = true;
    
    if (increasing) {
        test_level += 5;
        if (test_level >= 100) {
            increasing = false;
        }
    } else {
        test_level -= 5;
        if (test_level <= 0) {
            increasing = true;
        }
    }
    
    battery_set_level(test_level);
}

// Debug-Funktionen für Position-Testing
void battery_debug_positions() {
    DEBUG_PRINTLN("=== Battery Fill Position Debug ===");
    DEBUG_PRINTF("Widget Size: %dx%d\n", BATTERY_WIDGET_WIDTH, BATTERY_WIDGET_HEIGHT);
    DEBUG_PRINTF("Frame Size: %dx%d\n", BATTERY_FRAME_WIDTH, BATTERY_FRAME_HEIGHT);
    DEBUG_PRINTF("Terminal Size: %dx%d\n", BATTERY_TERMINAL_WIDTH, BATTERY_TERMINAL_HEIGHT);
    DEBUG_PRINTF("Fill Position: %d,%d\n", BATTERY_FILL_OFFSET_X, BATTERY_FILL_OFFSET_Y);
    DEBUG_PRINTF("Fill Size: %dx%d (max)\n", BATTERY_FILL_MAX_WIDTH, BATTERY_FILL_HEIGHT);
    DEBUG_PRINTLN("===================================");
}

// =============================================================================
// SERIAL COMMAND HANDLING
// =============================================================================

void handle_battery_serial_commands(String command) {
    if (command.startsWith("bat ")) {
        // Battery Level setzen: "bat 75"
        int level = command.substring(4).toInt();
        if (level >= 0 && level <= 100) {
            set_real_battery_level(level);
            Serial.printf("Battery level set to %d%%\n", level);
        } else {
            Serial.println("Invalid battery level (0-100)");
        }
    }
    else if (command == "bat demo on") {
        toggle_battery_demo(true);
        Serial.println("Battery demo enabled");
    }
    else if (command == "bat demo off") {
        toggle_battery_demo(false);
        Serial.println("Battery demo disabled");
    }
    else if (command == "bat status") {
        print_battery_status();
    }
    else if (command == "bat charge on") {
        battery_set_charging(true);
        Serial.println("Charging status: ON");
    }
    else if (command == "bat charge off") {
        battery_set_charging(false);
        Serial.println("Charging status: OFF");
    }
    else if (command == "bat debug") {
        battery_debug_positions();
    }
    else if (command == "bat test") {
        battery_test_animation();
        Serial.println("Battery test animation executed");
    }
    else if (command == "bat help") {
        Serial.println("=== Battery Commands ===");
        Serial.println("  bat <0-100>     - Set battery level");
        Serial.println("  bat demo on/off - Enable/disable demo animation");
        Serial.println("  bat status      - Show battery status");
        Serial.println("  bat charge on/off - Set charging status");
        Serial.println("  bat debug       - Show battery debug info");
        Serial.println("  bat test        - Run battery test animation");
        Serial.println("  bat help        - Show battery commands");
    }
}

#endif // BATTERY_H