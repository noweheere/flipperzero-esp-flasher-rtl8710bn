#include "../esp_flasher_app_i.h"

enum SubmenuIndex {
    SubmenuIndexEspFlash,
    SubmenuIndexRtlFlash,
    SubmenuIndexReset,
    SubmenuIndexAbout,
};

void esp_flasher_scene_start_submenu_callback(void* context, uint32_t index) {
    EspFlasherApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void esp_flasher_scene_start_on_enter(void* context) {
    EspFlasherApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu, "Flash ESP chip", SubmenuIndexEspFlash, esp_flasher_scene_start_submenu_callback, app);
    submenu_add_item(
        submenu, "Flash RTL8710BN", SubmenuIndexRtlFlash, esp_flasher_scene_start_submenu_callback, app);
    submenu_add_item(
        submenu, "Reset board", SubmenuIndexReset, esp_flasher_scene_start_submenu_callback, app);
    submenu_add_item(
        submenu, "About", SubmenuIndexAbout, esp_flasher_scene_start_submenu_callback, app);

    submenu_set_selected_item(submenu, scene_manager_get_scene_state(app->scene_manager, EspFlasherSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, EspFlasherAppViewSubmenu);
}

bool esp_flasher_scene_start_on_event(void* context, SceneManagerEvent event) {
    EspFlasherApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexEspFlash) {
            scene_manager_set_scene_state(app->scene_manager, EspFlasherSceneStart, SubmenuIndexEspFlash);
            app->chip_type = ChipTypeESP;
            scene_manager_next_scene(app->scene_manager, EspFlasherSceneBrowse);
            consumed = true;
        } else if(event.event == SubmenuIndexRtlFlash) {
            scene_manager_set_scene_state(app->scene_manager, EspFlasherSceneStart, SubmenuIndexRtlFlash);
            app->chip_type = ChipTypeRTL;
            scene_manager_next_scene(app->scene_manager, EspFlasherSceneBrowse);
            consumed = true;
        } else if(event.event == SubmenuIndexReset) {
            scene_manager_set_scene_state(app->scene_manager, EspFlasherSceneStart, SubmenuIndexReset);
            app->reset = true;
            scene_manager_next_scene(app->scene_manager, EspFlasherSceneConsoleOutput);
            consumed = true;
        } else if(event.event == SubmenuIndexAbout) {
            scene_manager_set_scene_state(app->scene_manager, EspFlasherSceneStart, SubmenuIndexAbout);
            scene_manager_next_scene(app->scene_manager, EspFlasherSceneAbout);
            consumed = true;
        }
    }

    return consumed;
}

void esp_flasher_scene_start_on_exit(void* context) {
    EspFlasherApp* app = context;
    submenu_reset(app->submenu);
}