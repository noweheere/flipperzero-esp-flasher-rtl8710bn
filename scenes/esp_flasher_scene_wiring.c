#include "../esp_flasher.h"

void esp_flasher_scene_wiring_on_enter(void* context) {
    EspFlasherApp* app = context;
    
    const char* chip_name;
    const char* wiring_text;
    
    switch(app->selected_chip) {
        case EspFlasherChipTypeRTL8710BN:
            chip_name = "RTL8710BN";
            wiring_text = 
                "RTL8710BN Wiring:\n\n"
                "Flipper -> RTL Module\n"
                "5V -> VCC (3.3V external!)\n"
                "GND -> GND\n"
                "A7 -> TX\n" 
                "A6 -> RX\n"
                "C0 -> CEN (Reset)\n"
                "C1 -> TX2 (Boot)\n\n"
                "Download Mode:\n"
                "1. Power OFF module\n"
                "2. Connect wires\n"
                "3. Power ON module\n"
                "4. App will handle boot sequence\n\n"
                "IMPORTANT: RTL8710BN requires\n"
                "external 3.3V power supply!\n"
                "Flipper 3.3V is insufficient.";
            break;
        case EspFlasherChipTypeESP32:
            chip_name = "ESP32";
            wiring_text = 
                "ESP32 Wiring:\n\n"
                "Flipper -> ESP32\n"
                "5V -> VIN\n"
                "GND -> GND\n"
                "A7 -> RX\n"
                "A6 -> TX\n"
                "C0 -> EN (Reset)\n"
                "C1 -> GPIO0 (Boot)\n\n"
                "Download Mode:\n"
                "Connect all wires, then\n"
                "power on or reset ESP32.\n"
                "App will handle boot sequence.";
            break;
        case EspFlasherChipTypeESP8266:
            chip_name = "ESP8266";
            wiring_text = 
                "ESP8266 Wiring:\n\n"
                "Flipper -> ESP8266\n"
                "5V -> VCC\n"
                "GND -> GND\n"
                "A7 -> RX\n"
                "A6 -> TX\n"
                "C0 -> RST (Reset)\n"
                "C1 -> GPIO0 (Boot)\n\n"
                "Download Mode:\n"
                "Connect all wires, then\n"
                "power on or reset ESP8266.\n"
                "App will handle boot sequence.";
            break;
        default:
            chip_name = "Unknown";
            wiring_text = "Unknown chip type selected.";
            break;
    }
    
    dialog_ex_reset(app->dialog_ex);
    dialog_ex_set_header(app->dialog_ex, chip_name, 64, 10, AlignCenter, AlignCenter);
    dialog_ex_set_text(app->dialog_ex, wiring_text, 64, 32, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(app->dialog_ex, "Back");
    dialog_ex_set_right_button_text(app->dialog_ex, "Flash");
    dialog_ex_set_context(app->dialog_ex, app);
    dialog_ex_set_result_callback(app->dialog_ex, NULL);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, EspFlasherViewDialogEx);
}

bool esp_flasher_scene_wiring_on_event(void* context, SceneManagerEvent event) {
    EspFlasherApp* app = context;
    bool consumed = false;
    
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultLeft:
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
            break;
        case DialogExResultRight:
            scene_manager_next_scene(app->scene_manager, EspFlasherSceneFlashing);
            consumed = true;
            break;
        }
    }
    
    return consumed;
}

void esp_flasher_scene_wiring_on_exit(void* context) {
    EspFlasherApp* app = context;
    dialog_ex_reset(app->dialog_ex);
}