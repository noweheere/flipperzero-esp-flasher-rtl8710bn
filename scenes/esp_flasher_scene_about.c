#include "../esp_flasher_app_i.h"

void esp_flasher_scene_about_on_enter(void* context) {
    EspFlasherApp* app = context;
    Widget* widget = app->widget;

    FuriString* temp_str;
    temp_str = furi_string_alloc();
    furi_string_printf(temp_str, "\e#%s\n", "ESP/RTL Flasher");
    furi_string_cat_printf(temp_str, "Version: %s\n", "1.0");
    furi_string_cat_printf(temp_str, "Developed by: %s\n", "Community");
    furi_string_cat_printf(temp_str, "\e#%s\n", "Description");
    furi_string_cat_printf(
        temp_str,
        "Flash ESP32 chips and RTL8710BN/WR3\n"
        "Tuya modules from your Flipper Zero!\n\n");
    furi_string_cat_printf(temp_str, "\e#%s\n", "Supported chips:");
    furi_string_cat_printf(temp_str, "• ESP32, ESP32-S2/S3/C3\n");
    furi_string_cat_printf(temp_str, "• RTL8710BN (Tuya WR3)\n\n");
    furi_string_cat_printf(temp_str, "\e#%s\n", "RTL8710BN Wiring:");
    furi_string_cat_printf(temp_str, "• VCC -> 3.3V (external power)\n");
    furi_string_cat_printf(temp_str, "• GND -> GND\n");
    furi_string_cat_printf(temp_str, "• TX (PA29) -> Pin 13 (RX)\n");
    furi_string_cat_printf(temp_str, "• RX (PA30) -> Pin 14 (TX)\n");
    furi_string_cat_printf(temp_str, "• CEN -> Pin 15 (control)\n\n");
    furi_string_cat_printf(temp_str, "Download mode: CEN+TX2 sequence\n");

    widget_add_text_scroll_element(widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(app->view_dispatcher, EspFlasherAppViewWidget);
}

bool esp_flasher_scene_about_on_event(void* context, SceneManagerEvent event) {
    EspFlasherApp* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);
    return consumed;
}

void esp_flasher_scene_about_on_exit(void* context) {
    EspFlasherApp* app = context;
    widget_reset(app->widget);
}