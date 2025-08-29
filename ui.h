/*
=============================================================================
ui.h - Simplified UI System (Main → Sub, no Detail pages)
=============================================================================
*/

#ifndef UI_H
#define UI_H

#include <lvgl.h>
#include "config.h"
#include "state_machine.h"
#include "images.h"
#include "battery.h"
#include "timer_system.h"

// Forward declarations für Bluetooth
extern void bluetooth_enable();
extern void bluetooth_disable();

// =============================================================================
// STRUCTURES
// =============================================================================
struct MainCard {
  String title;
  const lv_img_dsc_t* icon_img;
  AppState target_state;
};

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================
#ifndef UI_GLOBALS_DEFINED
#define UI_GLOBALS_DEFINED

// External
extern void save_app_state();

// Forward declarations für Bluetooth
extern bool app_state_bluetooth_enabled;

// BT Indicator Configuration
struct BTIndicatorConfig {
  // Erscheinungsbild
  int size_width = 12;
  int size_height = 12;
  int radius = 6;
  uint32_t color = COLOR_BLE_INDICATOR;
  
  // Animation
  int blink_interval_ms = 1000;
  int opacity_min = LV_OPA_30;
  int opacity_max = LV_OPA_COVER;
  
  // Positionierung - Standard für alle Seiten
  struct {
    int main_x_offset = -45;
    int main_y_offset = 8;
    int header_x_offset = -45;  // Für alle Header-basierten Seiten
    int header_y_offset = 0;
  } position;
  
  // Spezifische Überschreibungen pro Seite (optional)
  struct {
    bool use_custom_positions = true;
    struct { int x = -45; int y = 12; } main;
    struct { int x = -55; int y = -5; } timer;
    struct { int x = -55; int y = -5; } tlapse;
    struct { int x = -55; int y = -4; } interval;
    struct { int x = -55; int y = -4; } settings;
    struct { int x = -55; int y = -4; } wire_settings;
  } custom_positions;
} bt_config;

// Loading screen objects
lv_obj_t *loading_page;
lv_obj_t *loading_spinner;
lv_obj_t *loading_title;
lv_obj_t *loading_subtitle;
lv_obj_t *loading_progress_dots[3];
lv_timer_t *loading_spinner_timer;
lv_timer_t *loading_dots_timer;
int loading_spinner_angle = 0;
int loading_current_dot = 0;

// Main page card data
MainCard main_cards[4] = {
  {"Timer", &icon_timer, STATE_TIMER},
  {"T-Lapse", &icon_timelapse, STATE_TLAPSE}, 
  {"Interval", &icon_interval, STATE_INTERVAL},
  {"Settings", &icon_settings, STATE_SETTINGS}
};

// Main page objects
lv_obj_t *main_page;
lv_obj_t *main_card_container;
lv_obj_t *main_card_timer, *main_card_tlapse, *main_card_interval, *main_card_settings;
lv_obj_t *main_card_objects[4];
lv_obj_t *main_swipe_area;
lv_obj_t *main_dot1, *main_dot2, *main_dot3, *main_dot4;

// Battery widgets
lv_obj_t *main_battery_widget;
lv_obj_t *template_battery_widget;
lv_obj_t *interval_battery_widget;

// BT INDICATOR SYSTEM
lv_obj_t *main_bt_indicator = nullptr;
lv_obj_t *template_bt_indicator = nullptr;
lv_obj_t *interval_bt_indicator = nullptr; 
lv_obj_t *settings_bt_indicator = nullptr;
lv_obj_t *wire_bt_indicator = nullptr;

lv_timer_t *bt_blink_timer = nullptr;
bool bt_indicator_visible = true;

// Main page state
int main_current_card = 0;
int main_total_cards = 4;
bool main_is_animating = false;

// Template page objects (für Timer/T-Lapse)
lv_obj_t *template_page;
lv_obj_t *template_header_label;
lv_obj_t *template_button_container;
lv_obj_t *template_option1_btn, *template_option2_btn;
lv_obj_t *template_option1_label, *template_option2_label;
lv_obj_t *template_option1_time, *template_option2_time;
lv_obj_t *template_swipe_area;
lv_obj_t *template_start_btn;
lv_obj_t *template_dot1, *template_dot2;

// Interval page objects (nur 1 Karte)
lv_obj_t *interval_page;
lv_obj_t *interval_header_label;
lv_obj_t *interval_single_btn;
lv_obj_t *interval_single_label;
lv_obj_t *interval_single_time;
lv_obj_t *interval_start_btn;

// Popup objects
lv_obj_t *popup_overlay;
lv_obj_t *popup_modal;

// Settings page objects
lv_obj_t *settings_page;
lv_obj_t *settings_header_label;
lv_obj_t *settings_battery_widget;
lv_obj_t *settings_wire_btn;
lv_obj_t *settings_led_switch;
lv_obj_t *settings_bt_switch;

// Wire settings page objects  
lv_obj_t *wire_settings_page;
lv_obj_t *wire_header_label;
lv_obj_t *wire_battery_widget;
lv_obj_t *wire_percentage_label;
lv_obj_t *wire_percent_text_label;
lv_obj_t *wire_save_btn;


#endif // UI_GLOBALS_DEFINED

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================
void ui_init();
void show_current_page();

// Page creation functions
void create_loading_page();
void create_main_page();
void create_template_page();
void create_interval_page();
void create_popup();

// Loading screen functions
void loading_spinner_timer_cb(lv_timer_t *timer);
void cleanup_loading_screen();

// Header management functions
lv_obj_t* create_page_header(lv_obj_t *parent, const char* title, bool show_back_btn);
void update_all_battery_widgets();

// Page management functions
void hide_all_pages();
void update_template_content(PageContent content);
void init_template_content(PageContent content);
void update_interval_content(PageContent content);

// Main page functions
void update_main_dots(int active_index);
void animate_main_to_card(int target_index);
void main_anim_complete_cb(lv_anim_t *a);
lv_obj_t* create_main_card(lv_obj_t *parent, MainCard card_data, int initial_x);

// Template page swipe functionality
void update_template_dots(int active_index);
void animate_to_option(int target_option);
void anim_complete_cb(lv_anim_t *a);

// Event callbacks - SIMPLIFIED (no detail callbacks)
void main_card_cb(lv_event_t *e);
void main_swipe_cb(lv_event_t *e);
void template_back_cb(lv_event_t *e);
void template_start_cb(lv_event_t *e);
void template_swipe_cb(lv_event_t *e);
void interval_back_cb(lv_event_t *e);
void interval_start_cb(lv_event_t *e);
void popup_close_cb(lv_event_t *e);

// Function declarations
void create_settings_page();
void create_wire_settings_page();
void update_wire_percentage_display();

// Event callbacks
void settings_back_cb(lv_event_t *e);
void settings_wire_cb(lv_event_t *e);
void settings_led_switch_cb(lv_event_t *e);
void settings_bt_switch_cb(lv_event_t *e);
void wire_settings_back_cb(lv_event_t *e);
void wire_save_cb(lv_event_t *e);

// BT indicator functions
void bt_blink_timer_cb(lv_timer_t *timer);
lv_obj_t* create_bt_indicator(lv_obj_t *parent, int x_offset, int y_offset);
void delete_all_bt_indicators();
void start_bt_indicator_system();
void stop_bt_indicator_system();
void recreate_bt_indicators_for_current_page();
void update_bt_indicator_system();

// =============================================================================
// IMPLEMENTATIONS
// =============================================================================

void loading_spinner_timer_cb(lv_timer_t *timer) {
  loading_spinner_angle += 30;
  if (loading_spinner_angle >= 360) {
    loading_spinner_angle = 0;
  }
  if (loading_spinner) {
    lv_img_set_angle(loading_spinner, loading_spinner_angle * 10);
  }
}

void cleanup_loading_screen() {
  if (loading_spinner_timer) {
    lv_timer_del(loading_spinner_timer);
    loading_spinner_timer = NULL;
  }
  if (loading_dots_timer) {
    lv_timer_del(loading_dots_timer);
    loading_dots_timer = NULL;
  }
}

void bt_blink_timer_cb(lv_timer_t *timer) {
  bt_indicator_visible = !bt_indicator_visible;
  int target_opacity = bt_indicator_visible ? bt_config.opacity_max : bt_config.opacity_min;
  
  if (main_bt_indicator) {
    lv_obj_set_style_opa(main_bt_indicator, target_opacity, 0);
  }
  if (template_bt_indicator) {
    lv_obj_set_style_opa(template_bt_indicator, target_opacity, 0);
  }
  if (interval_bt_indicator) {
    lv_obj_set_style_opa(interval_bt_indicator, target_opacity, 0);
  }
  if (settings_bt_indicator) {
    lv_obj_set_style_opa(settings_bt_indicator, target_opacity, 0);
  }
  if (wire_bt_indicator) {
    lv_obj_set_style_opa(wire_bt_indicator, target_opacity, 0);
  }
}

lv_obj_t* create_bt_indicator(lv_obj_t *parent) {
  if (!app_state.bluetooth_enabled) {
    return nullptr;
  }
  
  lv_obj_t *indicator = lv_obj_create(parent);
  lv_obj_set_size(indicator, bt_config.size_width, bt_config.size_height);
  lv_obj_set_style_bg_color(indicator, lv_color_hex(bt_config.color), 0);
  lv_obj_set_style_border_width(indicator, 0, 0);
  lv_obj_set_style_radius(indicator, bt_config.radius, 0);
  lv_obj_clear_flag(indicator, LV_OBJ_FLAG_SCROLLABLE);
  
  return indicator;
}

void delete_all_bt_indicators() {
  if (main_bt_indicator) {
    lv_obj_del(main_bt_indicator);
    main_bt_indicator = nullptr;
  }
  if (template_bt_indicator) {
    lv_obj_del(template_bt_indicator);
    template_bt_indicator = nullptr;
  }
  if (interval_bt_indicator) {
    lv_obj_del(interval_bt_indicator);
    interval_bt_indicator = nullptr;
  }
  if (settings_bt_indicator) {
    lv_obj_del(settings_bt_indicator);
    settings_bt_indicator = nullptr;
  }
  if (wire_bt_indicator) {
    lv_obj_del(wire_bt_indicator);
    wire_bt_indicator = nullptr;
  }
}

void start_bt_indicator_system() {
  if (!app_state.bluetooth_enabled) {
    return;
  }
  
  if (!bt_blink_timer) {
    bt_blink_timer = lv_timer_create(bt_blink_timer_cb, bt_config.blink_interval_ms, NULL);
  }
}

void stop_bt_indicator_system() {
  if (bt_blink_timer) {
    lv_timer_del(bt_blink_timer);
    bt_blink_timer = nullptr;
  }
  delete_all_bt_indicators();
}

void recreate_bt_indicators_for_current_page() {
  delete_all_bt_indicators();
  
  if (!app_state.bluetooth_enabled) {
    return;
  }
  
  int x_offset, y_offset;
  lv_align_t align_type;
  
  switch (app_state.current_state) {
    case STATE_MAIN:
      if (main_page) {
        if (bt_config.custom_positions.use_custom_positions) {
          x_offset = bt_config.custom_positions.main.x;
          y_offset = bt_config.custom_positions.main.y;
        } else {
          x_offset = bt_config.position.main_x_offset;
          y_offset = bt_config.position.main_y_offset;
        }
        
        main_bt_indicator = create_bt_indicator(main_page);
        lv_obj_align(main_bt_indicator, LV_ALIGN_TOP_RIGHT, x_offset, y_offset);
      }
      break;
      
    case STATE_TIMER:
    case STATE_TLAPSE:
      if (template_page) {
        lv_obj_t *header = lv_obj_get_child(template_page, 0);
        if (header) {
          if (bt_config.custom_positions.use_custom_positions) {
            x_offset = (app_state.current_state == STATE_TIMER) ? 
                      bt_config.custom_positions.timer.x : 
                      bt_config.custom_positions.tlapse.x;
            y_offset = (app_state.current_state == STATE_TIMER) ? 
                      bt_config.custom_positions.timer.y : 
                      bt_config.custom_positions.tlapse.y;
          } else {
            x_offset = bt_config.position.header_x_offset;
            y_offset = bt_config.position.header_y_offset;
          }
          
          template_bt_indicator = create_bt_indicator(header);
          lv_obj_align(template_bt_indicator, LV_ALIGN_RIGHT_MID, x_offset, y_offset);
        }
      }
      break;
      
    case STATE_INTERVAL:
      if (interval_page) {
        lv_obj_t *header = lv_obj_get_child(interval_page, 0);
        if (header) {
          if (bt_config.custom_positions.use_custom_positions) {
            x_offset = bt_config.custom_positions.interval.x;
            y_offset = bt_config.custom_positions.interval.y;
          } else {
            x_offset = bt_config.position.header_x_offset;
            y_offset = bt_config.position.header_y_offset;
          }
          
          interval_bt_indicator = create_bt_indicator(header);
          lv_obj_align(interval_bt_indicator, LV_ALIGN_RIGHT_MID, x_offset, y_offset);
        }
      }
      break;
      
    case STATE_SETTINGS:
      if (settings_page) {
        lv_obj_t *header = lv_obj_get_child(settings_page, 0);
        if (header) {
          if (bt_config.custom_positions.use_custom_positions) {
            x_offset = bt_config.custom_positions.settings.x;
            y_offset = bt_config.custom_positions.settings.y;
          } else {
            x_offset = bt_config.position.header_x_offset;
            y_offset = bt_config.position.header_y_offset;
          }
          
          settings_bt_indicator = create_bt_indicator(header);
          lv_obj_align(settings_bt_indicator, LV_ALIGN_RIGHT_MID, x_offset, y_offset);
        }
      }
      break;
      
    case STATE_WIRE_SETTINGS:
      if (wire_settings_page) {
        lv_obj_t *header = lv_obj_get_child(wire_settings_page, 0);
        if (header) {
          if (bt_config.custom_positions.use_custom_positions) {
            x_offset = bt_config.custom_positions.wire_settings.x;
            y_offset = bt_config.custom_positions.wire_settings.y;
          } else {
            x_offset = bt_config.position.header_x_offset;
            y_offset = bt_config.position.header_y_offset;
          }
          
          wire_bt_indicator = create_bt_indicator(header);
          lv_obj_align(wire_bt_indicator, LV_ALIGN_RIGHT_MID, x_offset, y_offset);
        }
      }
      break;
  }
}

void update_bt_indicator_system() {
  stop_bt_indicator_system();
  
  if (app_state.bluetooth_enabled) {
    recreate_bt_indicators_for_current_page();
    start_bt_indicator_system();
  }
}

void create_loading_page() {
  loading_page = lv_obj_create(lv_scr_act());
  lv_obj_set_size(loading_page, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(loading_page, lv_color_hex(COLOR_BG_LOADING), 0);
  lv_obj_set_style_border_width(loading_page, 0, 0);
  lv_obj_set_style_pad_all(loading_page, 0, 0);
  lv_obj_set_scrollbar_mode(loading_page, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(loading_page, LV_OBJ_FLAG_SCROLLABLE);

  loading_title = lv_label_create(loading_page);
  lv_label_set_text(loading_title, "RS1");
  lv_obj_set_style_text_font(loading_title, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(loading_title, lv_color_hex(COLOR_TEXT_LOADING), 0);
  lv_obj_align(loading_title, LV_ALIGN_CENTER, 0, -60);

  lv_obj_t *version_label = lv_label_create(loading_page);
  lv_label_set_text(version_label, "v1.0 - Initializing...");
  lv_obj_set_style_text_font(version_label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(version_label, lv_color_hex(0x808080), 0);
  lv_obj_align(version_label, LV_ALIGN_BOTTOM_MID, 0, -10);

  loading_spinner_timer = lv_timer_create(loading_spinner_timer_cb, LOADING_SPINNER_SPEED, NULL);
  loading_dots_timer = lv_timer_create(loading_spinner_timer_cb, 500, NULL);

  DEBUG_PRINTLN("Loading screen created");
}

lv_obj_t* create_page_header(lv_obj_t *parent, const char* title, bool show_back_btn) {
  lv_obj_t *header = lv_obj_create(parent);
  lv_obj_set_size(header, lv_pct(100), 50);
  lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 8);
  lv_obj_set_style_bg_color(header, lv_color_hex(COLOR_BG_HEADER), 0);
  lv_obj_set_style_border_width(header, 0, 0);
  lv_obj_set_style_pad_all(header, 0, 0);

  if (show_back_btn) {
    lv_obj_t *back_container = lv_obj_create(header);
    lv_obj_set_size(back_container, 80, 30);
    lv_obj_align(back_container, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_bg_opa(back_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(back_container, 0, 0);
    lv_obj_set_style_pad_all(back_container, 0, 0);
    lv_obj_clear_flag(back_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // Event Handler für den Container hinzufügen
    if (strcmp(title, "Interval") == 0) {
      lv_obj_add_event_cb(back_container, interval_back_cb, LV_EVENT_CLICKED, NULL);
    } else {
      lv_obj_add_event_cb(back_container, template_back_cb, LV_EVENT_CLICKED, NULL);
    }
    
    lv_obj_t *arrow_icon = lv_img_create(back_container);
    lv_img_set_src(arrow_icon, &icon_back);
    lv_obj_align(arrow_icon, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_img_recolor(arrow_icon, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
    lv_obj_set_style_img_recolor_opa(arrow_icon, LV_OPA_COVER, 0);

    lv_obj_t *back_label = lv_label_create(back_container);
    lv_label_set_text(back_label, "Back");
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(back_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
    lv_obj_align(back_label, LV_ALIGN_LEFT_MID, 20, 0);
  }
  
  lv_obj_t *title_label = lv_label_create(header);
  lv_label_set_text(title_label, title);
  lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(title_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);
  
  lv_obj_t *battery_widget;
  if (show_back_btn) {
    battery_widget = create_battery_widget(header, 128, 12);
  } else {
    battery_widget = create_battery_widget(header, 118, 12);
  }

  
  return header;
}

void update_all_battery_widgets() {
  if (main_battery_widget) {
    update_battery_widget(main_battery_widget);
  }
  if (template_battery_widget) {
    update_battery_widget(template_battery_widget);
  }
  if (interval_battery_widget) {
    update_battery_widget(interval_battery_widget);
  }
  if (settings_battery_widget) {      // NEW
    update_battery_widget(settings_battery_widget);
  }
  if (wire_battery_widget) {          // NEW
    update_battery_widget(wire_battery_widget);
  }
}

void update_main_dots(int active_index) {
  lv_obj_t* dots[] = {main_dot1, main_dot2, main_dot3, main_dot4};
  
  for (int i = 0; i < main_total_cards; i++) {
    if (i == active_index) {
      lv_obj_set_style_bg_color(dots[i], lv_color_hex(COLOR_DOT_ACTIVE), 0);
    } else {
      lv_obj_set_style_bg_color(dots[i], lv_color_hex(COLOR_DOT_INACTIVE), 0);
    }
  }
}

void main_anim_complete_cb(lv_anim_t *a) {
  main_is_animating = false;
  update_main_dots(main_current_card);
  DEBUG_PRINTF("Main animation complete - card: %d\n", main_current_card);
}

void animate_main_to_card(int target_index) {
  if (main_is_animating || target_index == main_current_card || target_index < 0 || target_index >= main_total_cards) return;
  
  main_is_animating = true;
  main_current_card = target_index;
  
  int card_positions[4];
  for (int i = 0; i < main_total_cards; i++) {
    if (i == target_index) {
      card_positions[i] = 5;
    } else if (i < target_index) {
      card_positions[i] = -150;
    } else {
      card_positions[i] = 155;
    }
  }
  
  for (int i = 0; i < main_total_cards; i++) {
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, main_card_objects[i]);
    lv_anim_set_values(&anim, lv_obj_get_x(main_card_objects[i]), card_positions[i]);
    lv_anim_set_time(&anim, ANIMATION_TIME_MS);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    
    if (i == main_total_cards - 1) {
      lv_anim_set_ready_cb(&anim, main_anim_complete_cb);
    }
    
    lv_anim_start(&anim);
  }
}

static lv_coord_t main_start_x = 0;
static bool main_touch_started = false;

void main_swipe_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_indev_t *indev = lv_indev_get_act();
  lv_point_t point;
  
  if (main_is_animating) return;
  
  lv_indev_get_point(indev, &point);
  
  if (code == LV_EVENT_PRESSED) {
    main_start_x = point.x;
    main_touch_started = true;
  }
  else if (code == LV_EVENT_RELEASED && main_touch_started) {
    lv_coord_t end_x = point.x;
    lv_coord_t diff = end_x - main_start_x;
    
    if (abs(diff) > SWIPE_THRESHOLD) {
      if (diff > 0 && main_current_card > 0) {
        animate_main_to_card(main_current_card - 1);
      }
      else if (diff < 0 && main_current_card < main_total_cards - 1) {
        animate_main_to_card(main_current_card + 1);
      }
    }
    
    main_touch_started = false;
  }
}

lv_obj_t* create_main_card(lv_obj_t *parent, MainCard card_data, int initial_x) {
  lv_obj_t *card = lv_btn_create(parent);
  lv_obj_set_size(card, 140, 196);
  lv_obj_set_pos(card, initial_x, 7);
  lv_obj_set_style_bg_color(card, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_set_style_radius(card, 15, 0);
  
  lv_obj_t *icon_bg = lv_obj_create(card);
  lv_obj_set_size(icon_bg, 86, 86);
  lv_obj_align(icon_bg, LV_ALIGN_CENTER, 0, -30);
  lv_obj_set_style_bg_color(icon_bg, lv_color_hex(0x505050), 0);
  lv_obj_set_style_border_width(icon_bg, 0, 0);
  lv_obj_set_style_radius(icon_bg, 43, 0);
  // FIX: Make icon background non-clickable
  lv_obj_add_flag(icon_bg, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(icon_bg, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t *icon = lv_img_create(icon_bg);
  lv_img_set_src(icon, card_data.icon_img);
  lv_obj_center(icon);
  
  lv_obj_set_style_img_recolor(icon, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_set_style_img_recolor_opa(icon, LV_OPA_COVER, 0);
  // FIX: Make icon non-clickable
  lv_obj_clear_flag(icon, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t *title = lv_label_create(card);
  lv_label_set_text(title, card_data.title.c_str());
  lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -18);
  // FIX: Make title label non-clickable
  lv_obj_clear_flag(title, LV_OBJ_FLAG_CLICKABLE);
  
  return card;
}

void create_main_page() {
  main_page = lv_obj_create(lv_scr_act());
  lv_obj_set_size(main_page, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(main_page, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_style_border_width(main_page, 0, 0);
  lv_obj_set_style_pad_all(main_page, 10, 0);
  lv_obj_set_scrollbar_mode(main_page, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(main_page, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *main_title = lv_label_create(main_page);
  lv_label_set_text(main_title, "RS1");
  lv_obj_set_style_text_font(main_title, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(main_title, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(main_title, LV_ALIGN_TOP_LEFT, 8, 8);

  main_battery_widget = create_battery_widget(main_page, 118, 10);

  main_card_container = lv_obj_create(main_page);
  lv_obj_set_size(main_card_container, 150, 210);
  lv_obj_align(main_card_container, LV_ALIGN_CENTER, 0, -8);
  lv_obj_set_style_bg_opa(main_card_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(main_card_container, 0, 0);
  lv_obj_set_style_pad_all(main_card_container, 0, 0);
  lv_obj_clear_flag(main_card_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_clip_corner(main_card_container, false, 0);

  main_card_timer = create_main_card(main_card_container, main_cards[0], 5);
  main_card_tlapse = create_main_card(main_card_container, main_cards[1], 155);
  main_card_interval = create_main_card(main_card_container, main_cards[2], 155);
  main_card_settings = create_main_card(main_card_container, main_cards[3], 155);
  
  main_card_objects[0] = main_card_timer;
  main_card_objects[1] = main_card_tlapse;
  main_card_objects[2] = main_card_interval;
  main_card_objects[3] = main_card_settings;
  
  for (int i = 0; i < main_total_cards; i++) {
    lv_obj_add_event_cb(main_card_objects[i], main_card_cb, LV_EVENT_CLICKED, NULL);
  }

  main_swipe_area = lv_obj_create(main_page);
  lv_obj_set_size(main_swipe_area, lv_pct(90), 60);
  lv_obj_align(main_swipe_area, LV_ALIGN_CENTER, 0, 90);
  lv_obj_set_style_bg_opa(main_swipe_area, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(main_swipe_area, 0, 0);
  lv_obj_add_event_cb(main_swipe_area, main_swipe_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(main_swipe_area, main_swipe_cb, LV_EVENT_RELEASED, NULL);

  lv_obj_t *main_dots_container = lv_obj_create(main_page);
  lv_obj_set_size(main_dots_container, 80, 20);
  lv_obj_align(main_dots_container, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_set_style_bg_opa(main_dots_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(main_dots_container, 0, 0);
  lv_obj_set_style_pad_all(main_dots_container, 0, 0);

  main_dot1 = lv_obj_create(main_dots_container);
  lv_obj_set_size(main_dot1, 8, 8);
  lv_obj_set_pos(main_dot1, 15, 6);
  lv_obj_set_style_bg_color(main_dot1, lv_color_hex(COLOR_DOT_ACTIVE), 0);
  lv_obj_set_style_border_width(main_dot1, 0, 0);
  lv_obj_set_style_radius(main_dot1, 4, 0);

  main_dot2 = lv_obj_create(main_dots_container);
  lv_obj_set_size(main_dot2, 8, 8);
  lv_obj_set_pos(main_dot2, 30, 6);
  lv_obj_set_style_bg_color(main_dot2, lv_color_hex(COLOR_DOT_INACTIVE), 0);
  lv_obj_set_style_border_width(main_dot2, 0, 0);
  lv_obj_set_style_radius(main_dot2, 4, 0);

  main_dot3 = lv_obj_create(main_dots_container);
  lv_obj_set_size(main_dot3, 8, 8);
  lv_obj_set_pos(main_dot3, 45, 6);
  lv_obj_set_style_bg_color(main_dot3, lv_color_hex(COLOR_DOT_INACTIVE), 0);
  lv_obj_set_style_border_width(main_dot3, 0, 0);
  lv_obj_set_style_radius(main_dot3, 4, 0);

  main_dot4 = lv_obj_create(main_dots_container);
  lv_obj_set_size(main_dot4, 8, 8);
  lv_obj_set_pos(main_dot4, 60, 6);
  lv_obj_set_style_bg_color(main_dot4, lv_color_hex(COLOR_DOT_INACTIVE), 0);
  lv_obj_set_style_border_width(main_dot4, 0, 0);
  lv_obj_set_style_radius(main_dot4, 4, 0);

  update_main_dots(0);
}

void update_template_dots(int active_index) {
  if (active_index == 0) {
    lv_obj_set_style_bg_color(template_dot1, lv_color_hex(COLOR_DOT_ACTIVE), 0);
    lv_obj_set_style_bg_color(template_dot2, lv_color_hex(COLOR_DOT_INACTIVE), 0);
  } else {
    lv_obj_set_style_bg_color(template_dot1, lv_color_hex(COLOR_DOT_INACTIVE), 0);
    lv_obj_set_style_bg_color(template_dot2, lv_color_hex(COLOR_DOT_ACTIVE), 0);
  }
}

void anim_complete_cb(lv_anim_t *a) {
  app_state.is_animating = false;
  update_template_dots(app_state.current_option);
  DEBUG_PRINTF("Template animation complete - option: %d\n", app_state.current_option);
}

void animate_to_option(int target_option) {
  if (app_state.current_state == STATE_INTERVAL) {
    return; // Interval has no animation
  }
  
  if (app_state.is_animating || target_option == app_state.current_option) return;
  
  app_state.is_animating = true;
  app_state.current_option = target_option;
  
  int btn1_target_x = (target_option == 0) ? 5 : -145;
  int btn2_target_x = (target_option == 0) ? 155 : 5;
  
  DEBUG_PRINTF("Animating to option %d (Timer/T-Lapse only)\n", target_option);
  
  lv_anim_t anim1;
  lv_anim_init(&anim1);
  lv_anim_set_var(&anim1, template_option1_btn);
  lv_anim_set_values(&anim1, lv_obj_get_x(template_option1_btn), btn1_target_x);
  lv_anim_set_time(&anim1, ANIMATION_TIME_MS);
  lv_anim_set_exec_cb(&anim1, (lv_anim_exec_xcb_t)lv_obj_set_x);
  lv_anim_set_path_cb(&anim1, lv_anim_path_ease_out);
  lv_anim_start(&anim1);
  
  lv_anim_t anim2;
  lv_anim_init(&anim2);
  lv_anim_set_var(&anim2, template_option2_btn);
  lv_anim_set_values(&anim2, lv_obj_get_x(template_option2_btn), btn2_target_x);
  lv_anim_set_time(&anim2, ANIMATION_TIME_MS);
  lv_anim_set_exec_cb(&anim2, (lv_anim_exec_xcb_t)lv_obj_set_x);
  lv_anim_set_path_cb(&anim2, lv_anim_path_ease_out);
  lv_anim_set_ready_cb(&anim2, anim_complete_cb);
  lv_anim_start(&anim2);
}

static lv_coord_t start_x = 0;
static bool touch_started = false;

void template_swipe_cb(lv_event_t *e) {
  if (app_state.current_state == STATE_INTERVAL) {
    return; // Interval has no swipe functionality
  }
  
  lv_event_code_t code = lv_event_get_code(e);
  lv_indev_t *indev = lv_indev_get_act();
  lv_point_t point;
  
  if (app_state.is_animating) return;
  
  lv_indev_get_point(indev, &point);
  
  if (code == LV_EVENT_PRESSED) {
    start_x = point.x;
    touch_started = true;
  }
  else if (code == LV_EVENT_RELEASED && touch_started) {
    lv_coord_t end_x = point.x;
    lv_coord_t diff = end_x - start_x;
    
    if (abs(diff) > SWIPE_THRESHOLD) {
      if (diff > 0 && app_state.current_option == 1) {
        animate_to_option(0);
      }
      else if (diff < 0 && app_state.current_option == 0) {
        animate_to_option(1);
      }
    }
    
    touch_started = false;
  }
}

void create_template_page() {
  template_page = lv_obj_create(lv_scr_act());
  lv_obj_set_size(template_page, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(template_page, lv_color_hex(COLOR_BG_TEMPLATE), 0);
  lv_obj_set_style_border_width(template_page, 0, 0);
  lv_obj_set_style_pad_all(template_page, 0, 0);
  lv_obj_add_flag(template_page, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *template_header = create_page_header(template_page, "", true);
  template_header_label = lv_obj_get_child(template_header, 1);
  template_battery_widget = lv_obj_get_child(template_header, 2);

  template_button_container = lv_obj_create(template_page);
  lv_obj_set_size(template_button_container, 150, 158);
  lv_obj_align(template_button_container, LV_ALIGN_TOP_LEFT, 11, 60);
  lv_obj_set_style_bg_opa(template_button_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(template_button_container, 0, 0);
  lv_obj_set_style_pad_all(template_button_container, 0, 0);
  lv_obj_clear_flag(template_button_container, LV_OBJ_FLAG_SCROLLABLE);

  template_option1_btn = lv_btn_create(template_button_container);
  lv_obj_set_size(template_option1_btn, 142, 146);
  lv_obj_set_pos(template_option1_btn, 0, 5);
  lv_obj_set_style_bg_color(template_option1_btn, lv_color_hex(COLOR_BTN_SECONDARY), 0);
  lv_obj_set_style_radius(template_option1_btn, 8, 0);
  // REMOVED: No event callback - cards are no longer clickable
  lv_obj_clear_flag(template_option1_btn, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *option1_container = lv_obj_create(template_option1_btn);
  lv_obj_set_size(option1_container, lv_pct(100), lv_pct(100));
  lv_obj_center(option1_container);
  lv_obj_set_style_bg_opa(option1_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(option1_container, 0, 0);
  lv_obj_clear_flag(option1_container, LV_OBJ_FLAG_SCROLLABLE);

  template_option1_label = lv_label_create(option1_container);
  lv_label_set_text(template_option1_label, "Option 1");
  lv_obj_set_style_text_font(template_option1_label, &lv_font_montserrat_28, 0);
  lv_obj_align(template_option1_label, LV_ALIGN_TOP_LEFT, -10, 0);

  template_option1_time = lv_label_create(option1_container);
  lv_label_set_text(template_option1_time, "00:00");
  lv_obj_set_style_text_font(template_option1_time, &lv_font_montserrat_40, 0);
  lv_obj_align(template_option1_time, LV_ALIGN_TOP_MID, 0, 60);

  template_option2_btn = lv_btn_create(template_button_container);
  lv_obj_set_size(template_option2_btn, 142, 146);
  lv_obj_set_pos(template_option2_btn, 155, 10);
  lv_obj_set_style_bg_color(template_option2_btn, lv_color_hex(COLOR_BTN_SECONDARY), 0);
  lv_obj_set_style_radius(template_option2_btn, 8, 0);
  // REMOVED: No event callback - cards are no longer clickable
  lv_obj_clear_flag(template_option2_btn, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *option2_container = lv_obj_create(template_option2_btn);
  lv_obj_set_size(option2_container, lv_pct(100), lv_pct(100));
  lv_obj_center(option2_container);
  lv_obj_set_style_bg_opa(option2_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(option2_container, 0, 0);
  lv_obj_clear_flag(option2_container, LV_OBJ_FLAG_SCROLLABLE);

  template_option2_label = lv_label_create(option2_container);
  lv_label_set_text(template_option2_label, "Option 2");
  lv_obj_set_style_text_font(template_option2_label, &lv_font_montserrat_28, 0);
  lv_obj_align(template_option2_label, LV_ALIGN_TOP_LEFT, -10, 0);

  template_option2_time = lv_label_create(option2_container);
  lv_label_set_text(template_option2_time, "00:00");
  lv_obj_set_style_text_font(template_option2_time, &lv_font_montserrat_40, 0);
  lv_obj_align(template_option2_time, LV_ALIGN_TOP_MID, 0, 60);

  template_swipe_area = lv_obj_create(template_page);
  lv_obj_set_size(template_swipe_area, lv_pct(90), 60);
  lv_obj_align(template_swipe_area, LV_ALIGN_CENTER, 0, 65);
  lv_obj_set_style_bg_opa(template_swipe_area, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(template_swipe_area, 0, 0);
  lv_obj_add_event_cb(template_swipe_area, template_swipe_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(template_swipe_area, template_swipe_cb, LV_EVENT_RELEASED, NULL);

  lv_obj_t *dots_container = lv_obj_create(template_page);
  lv_obj_set_size(dots_container, 60, 20);
  lv_obj_align(dots_container, LV_ALIGN_TOP_MID, 0, 230);
  lv_obj_set_style_bg_opa(dots_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(dots_container, 0, 0);
  lv_obj_set_style_pad_all(dots_container, 0, 0);

  template_dot1 = lv_obj_create(dots_container);
  lv_obj_set_size(template_dot1, 8, 8);
  lv_obj_set_pos(template_dot1, 20, 6);
  lv_obj_set_style_bg_color(template_dot1, lv_color_hex(COLOR_DOT_ACTIVE), 0);
  lv_obj_set_style_border_width(template_dot1, 0, 0);
  lv_obj_set_style_radius(template_dot1, 4, 0);

  template_dot2 = lv_obj_create(dots_container);
  lv_obj_set_size(template_dot2, 8, 8);
  lv_obj_set_pos(template_dot2, 32, 6);
  lv_obj_set_style_bg_color(template_dot2, lv_color_hex(COLOR_DOT_INACTIVE), 0);
  lv_obj_set_style_border_width(template_dot2, 0, 0);
  lv_obj_set_style_radius(template_dot2, 4, 0);

  template_start_btn = lv_btn_create(template_page);
  lv_obj_set_size(template_start_btn, 150, 46);
  lv_obj_align(template_start_btn, LV_ALIGN_BOTTOM_MID, 0, -16);
  lv_obj_set_style_bg_color(template_start_btn, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_add_event_cb(template_start_btn, template_start_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_t *start_label = lv_label_create(template_start_btn);
  lv_label_set_text(start_label, "Start");
  lv_obj_set_style_text_color(start_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_set_style_text_font(start_label, &lv_font_montserrat_20, 0);
  lv_obj_center(start_label);
}

void create_interval_page() {
  interval_page = lv_obj_create(lv_scr_act());
  lv_obj_set_size(interval_page, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(interval_page, lv_color_hex(COLOR_BG_TEMPLATE), 0);
  lv_obj_set_style_border_width(interval_page, 0, 0);
  lv_obj_set_style_pad_all(interval_page, 0, 0);
  lv_obj_add_flag(interval_page, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *interval_header = create_page_header(interval_page, "", true);
  interval_header_label = lv_obj_get_child(interval_header, 1);
  interval_battery_widget = lv_obj_get_child(interval_header, 2);

  lv_obj_t *single_card_container = lv_obj_create(interval_page);
  lv_obj_set_size(single_card_container, 150, 158);
  lv_obj_align(single_card_container, LV_ALIGN_CENTER, 0, -20);
  lv_obj_set_style_bg_opa(single_card_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(single_card_container, 0, 0);
  lv_obj_set_style_pad_all(single_card_container, 0, 0);
  lv_obj_clear_flag(single_card_container, LV_OBJ_FLAG_SCROLLABLE);

  interval_single_btn = lv_btn_create(single_card_container);
  lv_obj_set_size(interval_single_btn, 142, 146);
  lv_obj_set_pos(interval_single_btn, 4, 6);
  lv_obj_set_style_bg_color(interval_single_btn, lv_color_hex(COLOR_BTN_SECONDARY), 0);
  lv_obj_set_style_radius(interval_single_btn, 8, 0);
  // REMOVED: No event callback - card is no longer clickable
  lv_obj_clear_flag(interval_single_btn, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *single_container = lv_obj_create(interval_single_btn);
  lv_obj_set_size(single_container, lv_pct(100), lv_pct(100));
  lv_obj_center(single_container);
  lv_obj_set_style_bg_opa(single_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(single_container, 0, 0);
  lv_obj_clear_flag(single_container, LV_OBJ_FLAG_SCROLLABLE);

  interval_single_label = lv_label_create(single_container);
  lv_label_set_text(interval_single_label, "Interval");
  lv_obj_set_style_text_font(interval_single_label, &lv_font_montserrat_28, 0);
  lv_obj_align(interval_single_label, LV_ALIGN_TOP_LEFT, -10, 0);

  interval_single_time = lv_label_create(single_container);
  lv_label_set_text(interval_single_time, "00:00");
  lv_obj_set_style_text_font(interval_single_time, &lv_font_montserrat_40, 0);
  lv_obj_align(interval_single_time, LV_ALIGN_TOP_MID, 0, 60);

  interval_start_btn = lv_btn_create(interval_page);
  lv_obj_set_size(interval_start_btn, 150, 46);
  lv_obj_align(interval_start_btn, LV_ALIGN_BOTTOM_MID, 0, -16);
  lv_obj_set_style_bg_color(interval_start_btn, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_add_event_cb(interval_start_btn, interval_start_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_t *interval_start_label = lv_label_create(interval_start_btn);
  lv_label_set_text(interval_start_label, "Start");
  lv_obj_set_style_text_color(interval_start_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_set_style_text_font(interval_start_label, &lv_font_montserrat_20, 0);
  lv_obj_center(interval_start_label);
}

void create_popup() {
  popup_overlay = lv_obj_create(lv_scr_act());
  lv_obj_set_size(popup_overlay, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(popup_overlay, lv_color_hex(COLOR_POPUP_OVERLAY), 0);
  lv_obj_set_style_bg_opa(popup_overlay, LV_OPA_70, 0);
  lv_obj_set_style_border_width(popup_overlay, 0, 0);
  lv_obj_add_flag(popup_overlay, LV_OBJ_FLAG_HIDDEN);

  popup_modal = lv_obj_create(popup_overlay);
  lv_obj_set_size(popup_modal, 200, 150);
  lv_obj_center(popup_modal);
  lv_obj_set_style_bg_color(popup_modal, lv_color_hex(COLOR_BG_TEMPLATE), 0);
  lv_obj_set_style_border_color(popup_modal, lv_color_hex(COLOR_POPUP_BORDER), 0);
  lv_obj_set_style_border_width(popup_modal, 2, 0);
  lv_obj_set_style_radius(popup_modal, 10, 0);

  lv_obj_t *popup_title = lv_label_create(popup_modal);
  lv_label_set_text(popup_title, "Start Process");
  lv_obj_set_style_text_font(popup_title, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(popup_title, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(popup_title, LV_ALIGN_TOP_MID, 0, 20);

  lv_obj_t *popup_text = lv_label_create(popup_modal);
  lv_label_set_text(popup_text, "Process started\nsuccessfully!");
  lv_obj_set_style_text_align(popup_text, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(popup_text, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_align(popup_text, LV_ALIGN_CENTER, 0, -10);

  lv_obj_t *popup_close_btn = lv_btn_create(popup_modal);
  lv_obj_set_size(popup_close_btn, 80, 30);
  lv_obj_align(popup_close_btn, LV_ALIGN_BOTTOM_MID, 0, -15);
  lv_obj_set_style_bg_color(popup_close_btn, lv_color_hex(COLOR_BTN_SUCCESS), 0);
  lv_obj_add_event_cb(popup_close_btn, popup_close_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_t *close_label = lv_label_create(popup_close_btn);
  lv_label_set_text(close_label, "Close");
  lv_obj_set_style_text_color(close_label, lv_color_hex(COLOR_TEXT_DARK), 0);
  lv_obj_center(close_label);
}

void create_settings_page() {
  settings_page = lv_obj_create(lv_scr_act());
  lv_obj_set_size(settings_page, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(settings_page, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_style_border_width(settings_page, 0, 0);
  lv_obj_set_style_pad_all(settings_page, 0, 0);
  lv_obj_add_flag(settings_page, LV_OBJ_FLAG_HIDDEN);

  // Header with back button and battery
  lv_obj_t *settings_header = create_page_header(settings_page, "", true);
  settings_header_label = lv_obj_get_child(settings_header, 1);
  settings_battery_widget = lv_obj_get_child(settings_header, 2);

  // Wire Button (like in Figma)
  settings_wire_btn = lv_btn_create(settings_page);
  lv_obj_set_size(settings_wire_btn, 150, 46);
  lv_obj_align(settings_wire_btn, LV_ALIGN_TOP_MID, 0, 80);
  lv_obj_set_style_bg_color(settings_wire_btn, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_set_style_radius(settings_wire_btn, 8, 0);
  lv_obj_add_event_cb(settings_wire_btn, settings_wire_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *wire_label = lv_label_create(settings_wire_btn);
  lv_label_set_text(wire_label, "Wire");
  lv_obj_set_style_text_color(wire_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_set_style_text_font(wire_label, &lv_font_montserrat_20, 0);
  lv_obj_center(wire_label);

  // LED Toggle Row
  lv_obj_t *led_container = lv_obj_create(settings_page);
  lv_obj_set_size(led_container, 150, 50);
  lv_obj_align(led_container, LV_ALIGN_TOP_MID, 0, 140);
  lv_obj_set_style_bg_color(led_container, lv_color_hex(COLOR_BTN_SECONDARY), 0);
  lv_obj_set_style_radius(led_container, 8, 0);
  lv_obj_set_style_border_width(led_container, 0, 0);
  lv_obj_set_style_pad_all(led_container, 15, 0);
  lv_obj_set_scrollbar_mode(led_container, LV_SCROLLBAR_MODE_OFF);
  
  lv_obj_t *led_label = lv_label_create(led_container);
  lv_label_set_text(led_label, "Led");
  lv_obj_set_style_text_font(led_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(led_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(led_label, LV_ALIGN_LEFT_MID, 0, 0);
  
  settings_led_switch = lv_switch_create(led_container);
  lv_obj_align(settings_led_switch, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_add_event_cb(settings_led_switch, settings_led_switch_cb, LV_EVENT_VALUE_CHANGED, NULL);
  if (app_state.led_enabled) {
    lv_obj_add_state(settings_led_switch, LV_STATE_CHECKED);
  }

  // BT (Bluetooth) Toggle Row
  lv_obj_t *bt_container = lv_obj_create(settings_page);
  lv_obj_set_size(bt_container, 150, 50);
  lv_obj_align(bt_container, LV_ALIGN_TOP_MID, 0, 200);
  lv_obj_set_style_bg_color(bt_container, lv_color_hex(COLOR_BTN_SECONDARY), 0);
  lv_obj_set_style_radius(bt_container, 8, 0);
  lv_obj_set_style_border_width(bt_container, 0, 0);
  lv_obj_set_style_pad_all(bt_container, 15, 0);
  lv_obj_set_scrollbar_mode(bt_container, LV_SCROLLBAR_MODE_OFF);  

  lv_obj_t *bt_label = lv_label_create(bt_container);
  lv_label_set_text(bt_label, "BT");
  lv_obj_set_style_text_font(bt_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(bt_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(bt_label, LV_ALIGN_LEFT_MID, 0, 0);
  
  settings_bt_switch = lv_switch_create(bt_container);
  lv_obj_align(settings_bt_switch, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_add_event_cb(settings_bt_switch, settings_bt_switch_cb, LV_EVENT_VALUE_CHANGED, NULL);
  if (app_state.bluetooth_enabled) {
    lv_obj_add_state(settings_bt_switch, LV_STATE_CHECKED);
  }
}

void create_wire_settings_page() {
  wire_settings_page = lv_obj_create(lv_scr_act());
  lv_obj_set_size(wire_settings_page, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(wire_settings_page, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_style_border_width(wire_settings_page, 0, 0);
  lv_obj_set_style_pad_all(wire_settings_page, 0, 0);
  lv_obj_add_flag(wire_settings_page, LV_OBJ_FLAG_HIDDEN);

  // Header with back button and battery
  lv_obj_t *wire_header = create_page_header(wire_settings_page, "", true);
  wire_header_label = lv_obj_get_child(wire_header, 1);
  wire_battery_widget = lv_obj_get_child(wire_header, 2);

  // Percentage display container (gray background like Figma)
  lv_obj_t *percentage_container = lv_obj_create(wire_settings_page);
  lv_obj_set_size(percentage_container, 120, 120);
  lv_obj_align(percentage_container, LV_ALIGN_CENTER, 0, -10);
  lv_obj_set_style_bg_color(percentage_container, lv_color_hex(COLOR_BTN_SECONDARY), 0);
  lv_obj_set_style_radius(percentage_container, 15, 0);
  lv_obj_set_style_border_width(percentage_container, 0, 0);
  lv_obj_clear_flag(percentage_container, LV_OBJ_FLAG_SCROLLABLE);

  // Large percentage number
  wire_percentage_label = lv_label_create(percentage_container);
  lv_label_set_text(wire_percentage_label, "20");
  lv_obj_set_style_text_font(wire_percentage_label, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(wire_percentage_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(wire_percentage_label, LV_ALIGN_CENTER, 0, -10);

  // "percent" text below
  wire_percent_text_label = lv_label_create(percentage_container);
  lv_label_set_text(wire_percent_text_label, "percent");
  lv_obj_set_style_text_font(wire_percent_text_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(wire_percent_text_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(wire_percent_text_label, LV_ALIGN_CENTER, 0, 25);

  // Save Button
  wire_save_btn = lv_btn_create(wire_settings_page);
  lv_obj_set_size(wire_save_btn, 150, 46);
  lv_obj_align(wire_save_btn, LV_ALIGN_BOTTOM_MID, 0, -16);
  lv_obj_set_style_bg_color(wire_save_btn, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_add_event_cb(wire_save_btn, wire_save_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *save_label = lv_label_create(wire_save_btn);
  lv_label_set_text(save_label, "Save");
  lv_obj_set_style_text_color(save_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_set_style_text_font(save_label, &lv_font_montserrat_20, 0);
  lv_obj_center(save_label);

  // Initialize display
  update_wire_percentage_display();

  // WICHTIG: Initialize switch states with LOADED values
  if (app_state.led_enabled) {
    lv_obj_add_state(settings_led_switch, LV_STATE_CHECKED);
  } else {
    lv_obj_clear_state(settings_led_switch, LV_STATE_CHECKED);
  }
  
  if (app_state.bluetooth_enabled) {
    lv_obj_add_state(settings_bt_switch, LV_STATE_CHECKED);
  } else {
    lv_obj_clear_state(settings_bt_switch, LV_STATE_CHECKED);
  }
}

void update_wire_percentage_display() {
  if (wire_percentage_label) {
    lv_label_set_text(wire_percentage_label, String(app_state.servo_wire_percentage).c_str());
  }
}

// =============================================================================
// EVENT CALLBACKS
// =============================================================================

// Event callbacks - SIMPLIFIED (removed detail navigation)
void main_card_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED && !main_is_animating) {
  lv_obj_t *clicked_card = lv_event_get_target(e);
  
  for (int i = 0; i < main_total_cards; i++) {
    if (main_card_objects[i] == clicked_card) {
      MainCard current_card = main_cards[i];
      DEBUG_PRINTF("Main card clicked: %s\n", current_card.title.c_str());

      // Egal ob Settings oder nicht → State wechseln
      change_state(current_card.target_state);
      show_current_page();

      break;
    }
  }
}
}

void template_back_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Template back pressed");
    go_back();
  }
}

void template_start_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Start button pressed");
    
    // Start timer based on current state
    switch (app_state.current_state) {
      case STATE_TIMER:
        start_timer_execution();
        break;
      case STATE_TLAPSE:
        start_tlapse_execution();
        break;
      default:
        DEBUG_PRINTLN("Invalid state for timer start");
        break;
    }
  }
}

void interval_back_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Interval back pressed");
    go_back();
  }
}

void interval_start_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Interval start button pressed");
    start_interval_execution();
  }
}

void popup_close_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Popup closed");
    lv_obj_add_flag(popup_overlay, LV_OBJ_FLAG_HIDDEN);
  }
}

void settings_back_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Settings back pressed");
    go_back();
  }
}

void settings_wire_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Wire settings opened");
    change_state(STATE_WIRE_SETTINGS);
    show_current_page();
  }
}

void settings_led_switch_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
    app_state.led_enabled = lv_obj_has_state(settings_led_switch, LV_STATE_CHECKED);
    DEBUG_PRINTF("LED toggled: %s\n", app_state.led_enabled ? "ON" : "OFF");
  }
}


void settings_bt_switch_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
    app_state.bluetooth_enabled = lv_obj_has_state(settings_bt_switch, LV_STATE_CHECKED);
    
    // BLE System entsprechend aktivieren/deaktivieren
    if (app_state.bluetooth_enabled) {
      bluetooth_enable();
    } else {
      bluetooth_disable();
    }
    
    if (settings_initialized) {
      save_app_state();
    }
    
    // BT-Indikator System aktualisieren
    update_bt_indicator_system();
    
    DEBUG_PRINTF("Bluetooth toggled: %s\n", app_state.bluetooth_enabled ? "ON" : "OFF");
  }
}

void wire_settings_back_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Wire settings back pressed");
    go_back();
  }
}

void wire_save_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    // Verwende die Settings-Funktion statt Direct Access
    save_app_state();
    DEBUG_PRINTF("Wire settings saved via button: %d%%\n", app_state.servo_wire_percentage);
    go_back();
  }
}

// Page management functions

void hide_all_pages() {
  lv_obj_add_flag(main_page, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(template_page, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(interval_page, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(loading_page, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(settings_page, LV_OBJ_FLAG_HIDDEN);        // NEW
  lv_obj_add_flag(wire_settings_page, LV_OBJ_FLAG_HIDDEN);   // NEW
}

void update_template_content(PageContent content) {
  // Update labels but keep current option unchanged for encoder functionality
  lv_label_set_text(template_option1_label, content.option1_text.c_str());
  lv_label_set_text(template_option2_label, content.option2_text.c_str());
  lv_label_set_text(template_option1_time, content.option1_time.c_str());
  lv_label_set_text(template_option2_time, content.option2_time.c_str());
  
  // Update dots to reflect current state
  update_template_dots(app_state.current_option);
  
  DEBUG_PRINTF("Template content updated - keeping current option: %d\n", app_state.current_option);
}

void init_template_content(PageContent content) {
  // Initialize page with reset positions
  lv_label_set_text(template_option1_label, content.option1_text.c_str());
  lv_label_set_text(template_option2_label, content.option2_text.c_str());
  lv_label_set_text(template_option1_time, content.option1_time.c_str());
  lv_label_set_text(template_option2_time, content.option2_time.c_str());
  
  // Reset to first option when initially showing page
  app_state.current_option = 0;
  lv_obj_set_pos(template_option1_btn, 5, 10);
  lv_obj_set_pos(template_option2_btn, 155, 10);
  update_template_dots(0);
  
  DEBUG_PRINTLN("Template page initialized - reset to option 0");
}

void update_interval_content(PageContent content) {
  lv_label_set_text(interval_single_label, content.option1_text.c_str());
  lv_label_set_text(interval_single_time, content.option1_time.c_str());
}


void show_current_page() {
  hide_all_pages();
 
  switch (app_state.current_state) {
    case STATE_LOADING:
      lv_obj_clear_flag(loading_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing loading page");
      break;
     
    case STATE_MAIN:
      if (LOADING_SCREEN_ENABLED) {
        cleanup_loading_screen();
      }
      lv_obj_clear_flag(main_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing main page");
      break;
     
    case STATE_TIMER:
      init_template_content(timer_content);
      lv_obj_clear_flag(template_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing timer template");
      break;
     
    case STATE_TLAPSE:
      init_template_content(tlapse_content);
      lv_obj_clear_flag(template_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing time-lapse template");
      break;
     
    case STATE_INTERVAL:
      app_state.current_option = 0;
      update_interval_content(interval_content);
      lv_obj_clear_flag(interval_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing interval page");
      break;
     
    case STATE_SETTINGS:
      lv_obj_clear_flag(settings_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing settings page");
      break;
     
    case STATE_WIRE_SETTINGS:
      lv_obj_clear_flag(wire_settings_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing wire settings page");
      break;
  }
 
  // Update battery widgets and BT indicators
  if (app_state.current_state != STATE_LOADING) {
    update_all_battery_widgets();
    recreate_bt_indicators_for_current_page();
    if (app_state.bluetooth_enabled) {
      start_bt_indicator_system();
    }
  }
}

void ui_init() {
  DEBUG_PRINTLN("Initializing UI...");
  
  if (app_state.current_state != STATE_LOADING) {
    battery_init();
  }
  
  if (LOADING_SCREEN_ENABLED) {
    create_loading_page();
  }
  
  create_main_page();
  create_template_page();
  create_interval_page();
  create_settings_page();        // NEW
  create_wire_settings_page();   // NEW
  create_popup();
  timer_system_init();
  
  if (app_state.current_state != STATE_LOADING) {
    battery_init();
  }
  
  show_current_page();
  
  update_bt_indicator_system();

  DEBUG_PRINTLN("UI initialized successfully!");
}

#endif // UI_H