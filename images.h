/*
=============================================================================
images.h - Icons
=============================================================================


=============================================================================
*/

#ifndef IMAGES_H
#define IMAGES_H

#include "config.h"
#include <lvgl.h>
#include "SettingsIcon.h"
#include "TimelapseIcon.h"
#include "TimerIcon.h"
#include "IntervalIcon.h"
#include "battery.h"  // Battery management system

// =============================================================================
// IMAGE DECLARATIONS
// =============================================================================
LV_IMG_DECLARE(icon_timer);
LV_IMG_DECLARE(icon_timelapse);
LV_IMG_DECLARE(icon_interval);
LV_IMG_DECLARE(icon_settings);

// =============================================================================
// VERBESSERTE ICONS (16x16 pixels für bessere Qualität)
// =============================================================================

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_ICONS
#define LV_ATTRIBUTE_IMG_ICONS
#endif

// =============================================================================
// TIMER ICON - Uhr mit Zeigern
// =============================================================================
extern const lv_img_dsc_t TimerIcon;  
#define icon_timer TimerIcon    

// =============================================================================
// TIMELAPSE ICON - Kamera mit Objektiv
// =============================================================================
extern const lv_img_dsc_t TimelapseIcon;  
#define icon_timelapse TimelapseIcon    

// =============================================================================
// INTERVAL ICON - Blitz/Zickzack
// =============================================================================
extern const lv_img_dsc_t IntervalIcon;  
#define icon_interval IntervalIcon 

// =============================================================================
// SETTINGS ICON (16x16) - Zahnrad
// =============================================================================
extern const lv_img_dsc_t SettingsIcon;  // Aus SettingsIcon.h
#define icon_settings SettingsIcon       // Alias für Kompatibilität

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

// Icon basierend auf Index laden
const lv_img_dsc_t* get_main_card_icon(int index) {
    switch(index) {
        case 0: return &TimerIcon;
        case 1: return &TimelapseIcon;
        case 2: return &IntervalIcon;
        case 3: return &SettingsIcon;
        default: return &TimerIcon;
    }
}

// Icon-Namen für Debugging
const char* get_icon_name(int index) {
    switch(index) {
        case 0: return "Timer";
        case 1: return "Time-Lapse";
        case 2: return "Interval";
        case 3: return "Settings";
        default: return "Unknown";
    }
}

#endif // IMAGES_H