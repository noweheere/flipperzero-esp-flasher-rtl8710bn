#include "../esp_flasher.h"

void esp_flasher_scene_start_on_enter(void* context) {
    EspFlasherApp* app = context;
    
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "ESP/RTL Flasher");
    
    submenu_add_item(
        app->submenu,
        "Flash ESP32",
        EspFlasherCustomEventSelectESP32,
        NULL,
        app);
        
    submenu_add_item(
        app->submenu,
        "Flash ESP8266", 
        EspFlasherCustomEventSelectESP8266,
        NULL,
        app);
        
    submenu_add_item(
        app->submenu,
        "Flash RTL8710BN",
        EspFlasherCustomEventSelectRTL8710BN,
        NULL,
        app);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, EspFlasherViewSubmenu);
}

bool esp_flasher_scene_start_on_event(void* context, SceneManagerEvent event) {
    EspFlasherApp* app = context;
    bool consumed = false;
    
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case EspFlasherCustomEventSelectESP32:
            app->selected_chip = EspFlasherChipTypeESP32;
            scene_manager_next_scene(app->scene_manager, EspFlasherSceneSelectFirmware);
            consumed = true;
            break;
        case EspFlasherCustomEventSelectESP8266:
            app->selected_chip = EspFlasherChipTypeESP8266;
            scene_manager_next_scene(app->scene_manager, EspFlasherSceneSelectFirmware);
            consumed = true;
            break;
        case EspFlasherCustomEventSelectRTL8710BN:
            app->selected_chip = EspFlasherChipTypeRTL8710BN;
            scene_manager_next_scene(app->scene_manager, EspFlasherSceneSelectFirmware);
            consumed = true;
            break;
        }
    }
    
    return consumed;
}

void esp_flasher_scene_start_on_exit(void* context) {
    EspFlasherApp* app = context;
    submenu_reset(app->submenu);
}