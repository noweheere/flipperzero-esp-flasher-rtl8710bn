#include "../esp_flasher.h"

static void esp_flasher_scene_select_firmware_file_browser_callback(void* context) {
    EspFlasherApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, EspFlasherCustomEventSelectFirmware);
}

void esp_flasher_scene_select_firmware_on_enter(void* context) {
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
    
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, ".bin", NULL);
    browser_options.base_path = ESP_FLASHER_APP_FOLDER;
    
    furi_string_set(app->firmware_path, ESP_FLASHER_APP_FOLDER);
    
    if(dialog_file_browser_show(app->dialogs, app->firmware_path, app->firmware_path, &browser_options)) {
        // Extract filename for display
        path_extract_filename(app->firmware_path, app->firmware_name, true);
        
        scene_manager_next_scene(app->scene_manager, EspFlasherSceneWiring);
    } else {
        scene_manager_previous_scene(app->scene_manager);
    }
}

bool esp_flasher_scene_select_firmware_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void esp_flasher_scene_select_firmware_on_exit(void* context) {
    UNUSED(context);
}