#include "../esp_flasher.h"

void esp_flasher_scene_success_on_enter(void* context) {
    EspFlasherApp* app = context;
    
    const char* chip_name;
    switch(app->selected_chip) {
        case EspFlasherChipTypeESP32:
            chip_name = "ESP32";
            break;
        case EspFlasherChipTypeESP8266:
            chip_name = "ESP8266";
            break;
        case EspFlasherChipTypeRTL8710BN:
            chip_name = "RTL8710BN";
            break;
        default:
            chip_name = "Unknown";
            break;
    }
    
    FuriString* success_text = furi_string_alloc();
    furi_string_printf(success_text, 
        "%s flashed successfully!\n\n"
        "Firmware: %s\n\n"
        "The chip should now boot\n"
        "with the new firmware.\n\n"
        "You can disconnect the\n"
        "Flipper Zero now.",
        chip_name,
        app->firmware_name);
    
    dialog_ex_reset(app->dialog_ex);
    dialog_ex_set_header(app->dialog_ex, "Success!", 64, 10, AlignCenter, AlignCenter);
    dialog_ex_set_text(app->dialog_ex, furi_string_get_cstr(success_text), 64, 32, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(app->dialog_ex, "Back");
    dialog_ex_set_right_button_text(app->dialog_ex, "Flash Again");
    dialog_ex_set_context(app->dialog_ex, app);
    dialog_ex_set_result_callback(app->dialog_ex, NULL);
    
    // Show notification
    notification_message(app->notifications, &sequence_success);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, EspFlasherViewDialogEx);
    
    furi_string_free(success_text);
}

bool esp_flasher_scene_success_on_event(void* context, SceneManagerEvent event) {
    EspFlasherApp* app = context;
    bool consumed = false;
    
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultLeft:
            scene_manager_search_and_switch_to_previous_scene(app->scene_manager, EspFlasherSceneStart);
            consumed = true;
            break;
        case DialogExResultRight:
            scene_manager_search_and_switch_to_previous_scene(app->scene_manager, EspFlasherSceneSelectFirmware);
            consumed = true;
            break;
        }
    }
    
    return consumed;
}

void esp_flasher_scene_success_on_exit(void* context) {
    EspFlasherApp* app = context;
    dialog_ex_reset(app->dialog_ex);
}