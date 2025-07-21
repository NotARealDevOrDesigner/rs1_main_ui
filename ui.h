/*
=============================================================================
ui.h - Clean User Interface System with Loading Screen & Battery Header
=============================================================================
*/

#ifndef UI_H
#define UI_H

#include <lvgl.h>
#include "config.h"
#include "state_machine.h"
#include "images.h"  // Verbesserte Images mit Battery System

// =============================================================================
// FORWARD DECLARATIONS & STRUCTURES
// =============================================================================
struct MainCard {
  String title;
  const lv_img_dsc_t* icon_img;
  AppState target_state;
};

// =============================================================================
// GLOBAL VARIABLES (defined only once)
// =============================================================================
#ifndef UI_GLOBALS_DEFINED
#define UI_GLOBALS_DEFINED

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

// Main page card data - mit verbesserten Icons
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

// Battery widgets für alle Pages
lv_obj_t *main_battery_widget;
lv_obj_t *template_battery_widget;
lv_obj_t *detail_battery_widget;

// Main page state
int main_current_card = 0;
int main_total_cards = 4;
bool main_is_animating = false;

// Template page objects
lv_obj_t *template_page;
lv_obj_t *template_header_label;
lv_obj_t *template_button_container;
lv_obj_t *template_option1_btn, *template_option2_btn;
lv_obj_t *template_option1_label, *template_option2_label;
lv_obj_t *template_option1_time, *template_option2_time;
lv_obj_t *template_swipe_area;
lv_obj_t *template_start_btn;
lv_obj_t *template_dot1, *template_dot2;

// Detail page objects
lv_obj_t *detail_page;
lv_obj_t *detail_header_label;
lv_obj_t *detail_content_label;

// Popup objects
lv_obj_t *popup_overlay;
lv_obj_t *popup_modal;

#endif // UI_GLOBALS_DEFINED

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================
void ui_init();
void show_current_page();

// Page creation functions
void create_loading_page();        // New loading page function
void create_main_page();
void create_template_page();
void create_detail_page();
void create_popup();

// Loading screen functions
void loading_spinner_timer_cb(lv_timer_t *timer);
//void loading_dots_timer_cb(lv_timer_t *timer);
void cleanup_loading_screen();

// Header management functions
lv_obj_t* create_page_header(lv_obj_t *parent,const char* title, bool show_back_btn);
void update_all_battery_widgets();

// Page management functions
void hide_all_pages();
void update_template_content(PageContent content);

// Main page functions
void update_main_dots(int active_index);
void animate_main_to_card(int target_index);
void main_anim_complete_cb(lv_anim_t *a);
lv_obj_t* create_main_card(lv_obj_t *parent, MainCard card_data, int initial_x);

// Template page swipe functionality
void update_template_dots(int active_index);
void animate_to_option(int target_option);
void anim_complete_cb(lv_anim_t *a);

// Event callbacks
void main_card_cb(lv_event_t *e);
void main_swipe_cb(lv_event_t *e);

void template_back_cb(lv_event_t *e);
void template_option1_cb(lv_event_t *e);
void template_option2_cb(lv_event_t *e);
void template_start_cb(lv_event_t *e);
void template_swipe_cb(lv_event_t *e);

void detail_back_cb(lv_event_t *e);
void popup_close_cb(lv_event_t *e);

// =============================================================================
// LOADING SCREEN FUNCTIONS
// =============================================================================

// Loading screen spinner animation callback
void loading_spinner_timer_cb(lv_timer_t *timer) {
  loading_spinner_angle += 30;
  if (loading_spinner_angle >= 360) {
    loading_spinner_angle = 0;
  }
  
  if (loading_spinner) {
    lv_img_set_angle(loading_spinner, loading_spinner_angle * 10); // LVGL uses 0.1 degree units
  }
}

// Loading screen dots animation callback
void loading_dots_timer_cb(lv_timer_t *timer) {
  // Reset all dots to inactive
  for (int i = 0; i < 3; i++) {
    if (loading_progress_dots[i]) {
      lv_obj_set_style_bg_color(loading_progress_dots[i], lv_color_hex(COLOR_DOT_INACTIVE), 0);
    }
  }
  
  // Activate current dot
  if (loading_progress_dots[loading_current_dot]) {
    lv_obj_set_style_bg_color(loading_progress_dots[loading_current_dot], lv_color_hex(COLOR_LOADING_SPINNER), 0);
  }
  
  loading_current_dot = (loading_current_dot + 1) % 3;
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

void create_loading_page() {
  loading_page = lv_obj_create(lv_scr_act());
  lv_obj_set_size(loading_page, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(loading_page, lv_color_hex(COLOR_BG_LOADING), 0);
  lv_obj_set_style_border_width(loading_page, 0, 0);
  lv_obj_set_style_pad_all(loading_page, 0, 0);
  lv_obj_set_scrollbar_mode(loading_page, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(loading_page, LV_OBJ_FLAG_SCROLLABLE);

  // Main title - RS1
  loading_title = lv_label_create(loading_page);
  lv_label_set_text(loading_title, "RS1");
  lv_obj_set_style_text_font(loading_title, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(loading_title, lv_color_hex(COLOR_TEXT_LOADING), 0);
  lv_obj_align(loading_title, LV_ALIGN_CENTER, 0, -60);

  /*
  // Subtitle
  loading_subtitle = lv_label_create(loading_page);
  lv_label_set_text(loading_subtitle, "Camera Control System");
  lv_obj_set_style_text_font(loading_subtitle, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(loading_subtitle, lv_color_hex(COLOR_LOADING_ACCENT), 0);
  lv_obj_align(loading_subtitle, LV_ALIGN_CENTER, 0, -10);
  */


  /*
  // Spinner (simple circle that rotates)
  loading_spinner = lv_obj_create(loading_page);
  lv_obj_set_size(loading_spinner, 40, 40);
  lv_obj_align(loading_spinner, LV_ALIGN_CENTER, 0, 30);
  lv_obj_set_style_bg_opa(loading_spinner, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(loading_spinner, 3, 0);
  lv_obj_set_style_border_color(loading_spinner, lv_color_hex(COLOR_LOADING_SPINNER), 0);
  lv_obj_set_style_border_opa(loading_spinner, LV_OPA_30, 0);
  lv_obj_set_style_radius(loading_spinner, LV_RADIUS_CIRCLE, 0);
  
  // Add a bright segment to the circle for spinning effect
  lv_obj_t *spinner_segment = lv_obj_create(loading_spinner);
  lv_obj_set_size(spinner_segment, 8, 8);
  lv_obj_align(spinner_segment, LV_ALIGN_TOP_MID, 0, -4);
  lv_obj_set_style_bg_color(spinner_segment, lv_color_hex(COLOR_LOADING_SPINNER), 0);
  lv_obj_set_style_border_width(spinner_segment, 0, 0);
  lv_obj_set_style_radius(spinner_segment, LV_RADIUS_CIRCLE, 0);
  */
  /*
  // Progress dots
  lv_obj_t *dots_container = lv_obj_create(loading_page);
  lv_obj_set_size(dots_container, 60, 20);
  lv_obj_align(dots_container, LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_bg_opa(dots_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(dots_container, 0, 0);
  lv_obj_set_style_pad_all(dots_container, 0, 0);

  for (int i = 0; i < 3; i++) {
    loading_progress_dots[i] = lv_obj_create(dots_container);
    lv_obj_set_size(loading_progress_dots[i], 6, 6);
    lv_obj_set_pos(loading_progress_dots[i], 15 + i * 15, 7);
    lv_obj_set_style_bg_color(loading_progress_dots[i], lv_color_hex(COLOR_DOT_INACTIVE), 0);
    lv_obj_set_style_border_width(loading_progress_dots[i], 0, 0);
    lv_obj_set_style_radius(loading_progress_dots[i], LV_RADIUS_CIRCLE, 0);
  }
  */
  // Version/Status text at bottom
  lv_obj_t *version_label = lv_label_create(loading_page);
  lv_label_set_text(version_label, "v1.0 - Initializing...");
  lv_obj_set_style_text_font(version_label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(version_label, lv_color_hex(0x808080), 0);
  lv_obj_align(version_label, LV_ALIGN_BOTTOM_MID, 0, -10);

  // Start animations
  loading_spinner_timer = lv_timer_create(loading_spinner_timer_cb, LOADING_SPINNER_SPEED, NULL);
  loading_dots_timer = lv_timer_create(loading_dots_timer_cb, 500, NULL); // Slower dot animation

  DEBUG_PRINTLN("Loading screen created with animations");
}

// =============================================================================
// HEADER MANAGEMENT FUNCTIONS
// =============================================================================

// Universeller Header für alle Pages
lv_obj_t* create_page_header(lv_obj_t *parent, const char* title, bool show_back_btn) {
  // Header Container
  lv_obj_t *header = lv_obj_create(parent);
  lv_obj_set_size(header, lv_pct(100), 50);
  lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 8);
  lv_obj_set_style_bg_color(header, lv_color_hex(COLOR_BG_HEADER), 0);
  lv_obj_set_style_border_width(header, 0, 0);
  lv_obj_set_style_pad_all(header, 0, 0);

  // Back Button (wenn benötigt)
  if (show_back_btn) {
    lv_obj_t *back_btn = lv_btn_create(header);
    lv_obj_set_size(back_btn, 60, 30);
    lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(COLOR_BTN_DANGER), 0);
    lv_obj_set_style_radius(back_btn, 5, 0);
    
    // Event Handler basierend auf current state
    if (strcmp(title, "Detail") == 0) {
      lv_obj_add_event_cb(back_btn, detail_back_cb, LV_EVENT_CLICKED, NULL);
    } else {
      lv_obj_add_event_cb(back_btn, template_back_cb, LV_EVENT_CLICKED, NULL);
    }
    
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(back_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
    lv_obj_center(back_label);
  }
  //*
  
  // Title Label (zentriert)
  lv_obj_t *title_label = lv_label_create(header);
  lv_label_set_text(title_label, title);
  lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(title_label, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);
  //*
  
  // Battery Widget (oben rechts)
  lv_obj_t *battery_widget = create_battery_widget(header, 118, 12); // Position für 172px breites Display
  
  return header;
}

// Battery Widgets in allen Pages updaten
void update_all_battery_widgets() {
  if (main_battery_widget) {
    update_battery_widget(main_battery_widget);
  }
  if (template_battery_widget) {
    update_battery_widget(template_battery_widget);
  }
  if (detail_battery_widget) {
    update_battery_widget(detail_battery_widget);
  }
}

// =============================================================================
// MAIN PAGE SWIPE FUNCTIONALITY
// =============================================================================
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
  DEBUG_PRINTF("Main card animation complete - showing card: %d\n", main_current_card);
}

void animate_main_to_card(int target_index) {
  if (main_is_animating || target_index == main_current_card || target_index < 0 || target_index >= main_total_cards) return;
  
  main_is_animating = true;
  main_current_card = target_index;
  
  DEBUG_PRINTF("Animating to main card %d: %s\n", target_index, main_cards[target_index].title.c_str());
  
  // Calculate target positions for all cards
  int card_positions[4];
  for (int i = 0; i < main_total_cards; i++) {
    if (i == target_index) {
      card_positions[i] = 5; // Center position (visible)
    } else if (i < target_index) {
      card_positions[i] = -150; // Off-screen left
    } else {
      card_positions[i] = 155; // Off-screen right
    }
  }
  
  // Animate all cards to their target positions
  for (int i = 0; i < main_total_cards; i++) {
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, main_card_objects[i]);
    lv_anim_set_values(&anim, lv_obj_get_x(main_card_objects[i]), card_positions[i]);
    lv_anim_set_time(&anim, ANIMATION_TIME_MS);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
    
    // Only set callback on the last animation
    if (i == main_total_cards - 1) {
      lv_anim_set_ready_cb(&anim, main_anim_complete_cb);
    }
    
    lv_anim_start(&anim);
  }
}

// Touch-based swipe detection for main page
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
        // Swipe right: previous card
        animate_main_to_card(main_current_card - 1);
      }
      else if (diff < 0 && main_current_card < main_total_cards - 1) {
        // Swipe left: next card
        animate_main_to_card(main_current_card + 1);
      }
    }
    
    main_touch_started = false;
  }
}

// Create individual card with content
lv_obj_t* create_main_card(lv_obj_t *parent, MainCard card_data, int initial_x) {
  lv_obj_t *card = lv_btn_create(parent);
  lv_obj_set_size(card, 140, 196);
  lv_obj_set_pos(card, initial_x, 7);
  lv_obj_set_style_bg_color(card, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_set_style_radius(card, 15, 0);
  
  // Icon circle background
  lv_obj_t *icon_bg = lv_obj_create(card);
  lv_obj_set_size(icon_bg, 86, 86);
  lv_obj_align(icon_bg, LV_ALIGN_CENTER, 0, -30);
  lv_obj_set_style_bg_color(icon_bg, lv_color_hex(0x505050), 0);
  lv_obj_set_style_border_width(icon_bg, 0, 0);
  lv_obj_set_style_radius(icon_bg, 43, 0);

  // Bitmap Icon
  lv_obj_t *icon = lv_img_create(icon_bg);
  lv_img_set_src(icon, card_data.icon_img);
  lv_obj_center(icon);
  
  // Icon einfärben (für monochrome Icons)
  lv_obj_set_style_img_recolor(icon, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_set_style_img_recolor_opa(icon, LV_OPA_COVER, 0);

  // Title
  lv_obj_t *title = lv_label_create(card);
  lv_label_set_text(title, card_data.title.c_str());
  lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -18);
  
  return card;
}

// =============================================================================
// TEMPLATE PAGE SWIPE FUNCTIONALITY
// =============================================================================
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
  DEBUG_PRINTF("Template animation complete - current option: %d\n", app_state.current_option);
}

void animate_to_option(int target_option) {
  if (app_state.is_animating || target_option == app_state.current_option) return;
  
  app_state.is_animating = true;
  app_state.current_option = target_option;
  
  int btn1_target_x, btn2_target_x;
  
  if (target_option == 0) {
    btn1_target_x = 5;
    btn2_target_x = 155;
  } else {
    btn1_target_x = -145;
    btn2_target_x = 5;
  }
  
  DEBUG_PRINTF("Animating to option %d, btn1_x: %d, btn2_x: %d\n", target_option, btn1_target_x, btn2_target_x);
  
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

// =============================================================================
// EVENT CALLBACKS
// =============================================================================
// Main Page Events
void main_card_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED && !main_is_animating) {
    // Find which card was clicked
    lv_obj_t *clicked_card = lv_event_get_target(e);
    
    for (int i = 0; i < main_total_cards; i++) {
      if (main_card_objects[i] == clicked_card) {
        MainCard current_card = main_cards[i];
        DEBUG_PRINTF("Main card clicked: %s\n", current_card.title.c_str());
        
        if (current_card.target_state != STATE_SETTINGS) {
          change_state(current_card.target_state);
          show_current_page();
        } else {
          DEBUG_PRINTLN("Settings not implemented yet");
        }
        break;
      }
    }
  }
}

// Template Page Events
void template_back_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Template back pressed");
    go_back();
  }
}

void template_option1_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED && !app_state.is_animating) {
    DEBUG_PRINTLN("Option 1 clicked");
    app_state.detail_context = get_detail_context();
    change_state(STATE_DETAIL);
    show_current_page();
  }
}

void template_option2_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED && !app_state.is_animating) {
    DEBUG_PRINTLN("Option 2 clicked");
    app_state.detail_context = get_detail_context();
    change_state(STATE_DETAIL);
    show_current_page();
  }
}

void template_start_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Start button pressed");
    lv_obj_clear_flag(popup_overlay, LV_OBJ_FLAG_HIDDEN);
  }
}

// Detail Page Events
void detail_back_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Detail back pressed");
    go_back();
  }
}

// Popup Events
void popup_close_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    DEBUG_PRINTLN("Popup closed");
    lv_obj_add_flag(popup_overlay, LV_OBJ_FLAG_HIDDEN);
  }
}

// =============================================================================
// UI CREATION FUNCTIONS
// =============================================================================
void create_main_page() {
  main_page = lv_obj_create(lv_scr_act());
  lv_obj_set_size(main_page, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(main_page, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_style_border_width(main_page, 0, 0);
  lv_obj_set_style_pad_all(main_page, 10, 0);
  lv_obj_set_scrollbar_mode(main_page, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(main_page, LV_OBJ_FLAG_SCROLLABLE);

  // Haupttitel RS1 (links oben)
  lv_obj_t *main_title = lv_label_create(main_page);
  lv_label_set_text(main_title, "RS1");
  lv_obj_set_style_text_font(main_title, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(main_title, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(main_title, LV_ALIGN_TOP_LEFT, 8, 8);

  // Battery Widget (rechts oben) für Main Page
  main_battery_widget = create_battery_widget(main_page,118, 10);

  // Card container - Position nach unten angepasst für Header
  main_card_container = lv_obj_create(main_page);
  lv_obj_set_size(main_card_container, 150, 210);
  lv_obj_align(main_card_container, LV_ALIGN_CENTER, 0, -8);  // Etwas weiter nach unten
  lv_obj_set_style_bg_opa(main_card_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(main_card_container, 0, 0);
  lv_obj_set_style_pad_all(main_card_container, 0, 0);
  lv_obj_clear_flag(main_card_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_clip_corner(main_card_container, false, 0);

  // Create all 4 physical cards
  main_card_timer = create_main_card(main_card_container, main_cards[0], 5);
  main_card_tlapse = create_main_card(main_card_container, main_cards[1], 155);
  main_card_interval = create_main_card(main_card_container, main_cards[2], 155);
  main_card_settings = create_main_card(main_card_container, main_cards[3], 155);
  
  // Store card objects in array
  main_card_objects[0] = main_card_timer;
  main_card_objects[1] = main_card_tlapse;
  main_card_objects[2] = main_card_interval;
  main_card_objects[3] = main_card_settings;
  
  // Add click events to all cards
  for (int i = 0; i < main_total_cards; i++) {
    lv_obj_add_event_cb(main_card_objects[i], main_card_cb, LV_EVENT_CLICKED, NULL);
  }

  // Swipe area
  main_swipe_area = lv_obj_create(main_page);
  lv_obj_set_size(main_swipe_area, lv_pct(90), 60);
  lv_obj_align(main_swipe_area, LV_ALIGN_CENTER, 0, 90);
  lv_obj_set_style_bg_opa(main_swipe_area, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(main_swipe_area, 0, 0);
  lv_obj_add_event_cb(main_swipe_area, main_swipe_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(main_swipe_area, main_swipe_cb, LV_EVENT_RELEASED, NULL);

  // Dot indicators
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

  // Initialize dots
  update_main_dots(0);
}

void create_template_page() {
  template_page = lv_obj_create(lv_scr_act());
  lv_obj_set_size(template_page, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(template_page, lv_color_hex(COLOR_BG_TEMPLATE), 0);
  lv_obj_set_style_border_width(template_page, 0, 0);
  lv_obj_set_style_pad_all(template_page, 0, 0);
  lv_obj_add_flag(template_page, LV_OBJ_FLAG_HIDDEN);

  // Header mit Battery (Template wird dynamisch gesetzt)
  lv_obj_t *template_header = create_page_header(template_page, "Template", true);
  template_header_label = lv_obj_get_child(template_header, 1); // Title label ist das 2. Child
  template_battery_widget = lv_obj_get_child(template_header, 1); // Battery widget ist das 3. Child

  // Button container für swipe functionality
  template_button_container = lv_obj_create(template_page);
  lv_obj_set_size(template_button_container, 150, 158);
  lv_obj_align(template_button_container, LV_ALIGN_TOP_LEFT, 11, 60);
  lv_obj_set_style_bg_opa(template_button_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(template_button_container, 0, 0);
  lv_obj_set_style_pad_all(template_button_container, 0, 0);
  lv_obj_clear_flag(template_button_container, LV_OBJ_FLAG_SCROLLABLE);

  // Option buttons
  template_option1_btn = lv_btn_create(template_button_container);
  lv_obj_set_size(template_option1_btn, 142, 146);
  lv_obj_set_pos(template_option1_btn, 0, 5);
  lv_obj_set_style_bg_color(template_option1_btn, lv_color_hex(COLOR_BTN_SECONDARY), 0);
  lv_obj_set_style_radius(template_option1_btn, 8, 0);
  lv_obj_add_event_cb(template_option1_btn, template_option1_cb, LV_EVENT_CLICKED, NULL);
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
  lv_obj_align(template_option1_time, LV_ALIGN_TOP_MID, 0,60);

  template_option2_btn = lv_btn_create(template_button_container);
  lv_obj_set_size(template_option2_btn, 142, 146);
  lv_obj_set_pos(template_option2_btn, 155, 10);
  lv_obj_set_style_bg_color(template_option2_btn, lv_color_hex(COLOR_BTN_SECONDARY), 0);
  lv_obj_set_style_radius(template_option2_btn, 8, 0);
  lv_obj_add_event_cb(template_option2_btn, template_option2_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_clear_flag( template_option2_btn, LV_OBJ_FLAG_SCROLLABLE);


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

  // Swipe area
  template_swipe_area = lv_obj_create(template_page);
  lv_obj_set_size(template_swipe_area, lv_pct(90), 60);
  lv_obj_align(template_swipe_area, LV_ALIGN_CENTER, 0, 65);
  lv_obj_set_style_bg_opa(template_swipe_area, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(template_swipe_area, 0, 0);
  lv_obj_add_event_cb(template_swipe_area, template_swipe_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(template_swipe_area, template_swipe_cb, LV_EVENT_RELEASED, NULL);

  // Dot indicators
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

  // Start button
  template_start_btn = lv_btn_create(template_page);
  lv_obj_set_size(template_start_btn, 150, 46);
  lv_obj_align(template_start_btn, LV_ALIGN_BOTTOM_MID, 0, -16);
  lv_obj_set_style_bg_color(template_start_btn, lv_color_hex(COLOR_BTN_PRIMARY), 0);
  lv_obj_add_event_cb(template_start_btn, template_start_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_t *start_label = lv_label_create(template_start_btn);
  lv_label_set_text(start_label, "Start");
  lv_obj_set_style_text_color(start_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_set_style_text_font(template_start_btn, &lv_font_montserrat_20, 0);
  lv_obj_center(start_label);
}

void create_detail_page() {
  detail_page = lv_obj_create(lv_scr_act());
  lv_obj_set_size(detail_page, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(detail_page, lv_color_hex(COLOR_BG_DETAIL), 0);
  lv_obj_set_style_border_width(detail_page, 0, 0);
  lv_obj_set_style_pad_all(detail_page, 0, 0);
  lv_obj_add_flag(detail_page, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_scrollbar_mode(detail_page, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(detail_page, LV_OBJ_FLAG_SCROLLABLE);

  // Header mit Battery
  lv_obj_t *detail_header = create_page_header(detail_page, "Template", true);
  detail_header_label = lv_obj_get_child(detail_header, 1); // Title label
  detail_battery_widget = lv_obj_get_child(detail_header, 2); // Battery widget

  // Content
  detail_content_label = lv_label_create(detail_page);
  lv_label_set_text(detail_content_label, "Placeholder Content");
  lv_obj_set_style_text_font(detail_content_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(detail_content_label, lv_color_hex(COLOR_TEXT_SECONDARY), 0);
  lv_obj_align(detail_content_label, LV_ALIGN_CENTER, 0, 0);
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

// =============================================================================
// PAGE MANAGEMENT FUNCTIONS
// =============================================================================
void hide_all_pages() {
  lv_obj_add_flag(main_page, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(template_page, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(detail_page, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(loading_page, LV_OBJ_FLAG_HIDDEN);  // Include loading page
}

void update_template_content(PageContent content) {
  lv_label_set_text(template_header_label, content.heading.c_str());
  lv_label_set_text(template_option1_label, content.option1_text.c_str());
  lv_label_set_text(template_option2_label, content.option2_text.c_str());
  lv_label_set_text(template_option1_time, content.option1_time.c_str());
  lv_label_set_text(template_option2_time, content.option2_time.c_str());
  
  // Reset to first option
  app_state.current_option = 0;
  lv_obj_set_pos(template_option1_btn, 5, 10);
  lv_obj_set_pos(template_option2_btn, 155, 10);
  update_template_dots(0);
}

void show_current_page() {
  hide_all_pages();
  
  switch (app_state.current_state) {
    case STATE_LOADING:
      lv_obj_clear_flag(loading_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing loading page");
      break;
      
    case STATE_MAIN:
      // Clean up loading screen when transitioning to main
      if (LOADING_SCREEN_ENABLED) {
        cleanup_loading_screen();
      }
      lv_obj_clear_flag(main_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing main page");
      break;
      
    case STATE_TIMER:
      update_template_content(timer_content);
      lv_obj_clear_flag(template_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing timer template");
      break;
      
    case STATE_TLAPSE:
      update_template_content(tlapse_content);
      lv_obj_clear_flag(template_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing time-lapse template");
      break;
      
    case STATE_INTERVAL:
      update_template_content(interval_content);
      lv_obj_clear_flag(template_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing interval template");
      break;
      
    case STATE_DETAIL: {
      lv_label_set_text(detail_header_label, "Detail");
      String heading = get_detail_heading();
      update_dynamic_text(heading + " - Detail Configuration");
      lv_label_set_text(detail_content_label, app_state.dynamic_text.c_str());
      lv_obj_clear_flag(detail_page, LV_OBJ_FLAG_HIDDEN);
      DEBUG_PRINTLN("Showing detail page for: " + heading);
      break;
    }
      
    case STATE_SETTINGS:
      DEBUG_PRINTLN("Settings not implemented");
      change_state(STATE_MAIN);
      show_current_page();
      break;
  }
  
  // Battery widgets in allen aktiven Pages updaten (außer Loading)
  if (app_state.current_state != STATE_LOADING) {
    update_all_battery_widgets();
  }
}

void ui_init() {
  DEBUG_PRINTLN("Initializing UI...");
  
  // Battery System initialisieren (nur wenn nicht im Loading State)
  if (app_state.current_state != STATE_LOADING) {
    battery_init();
  }
  
  // Create loading page first if enabled
  if (LOADING_SCREEN_ENABLED) {
    create_loading_page();
  }
  
  create_main_page();
  create_template_page();
  create_detail_page();
  create_popup();
  
  // Initialize battery system after UI creation if not in loading
  if (app_state.current_state != STATE_LOADING) {
    battery_init();
  }
  
  show_current_page();
  
  DEBUG_PRINTLN("UI initialized successfully!");
}

#endif // UI_H