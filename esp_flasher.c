#include "esp_flasher.h"
#include "esp_flasher_rtl87xx.h"
#include "scenes/esp_flasher_scene.h"

#define TAG "EspFlasher"

static bool esp_flasher_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    EspFlasherApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool esp_flasher_back_event_callback(void* context) {
    furi_assert(context);
    EspFlasherApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static EspFlasherApp* esp_flasher_app_alloc() {
    EspFlasherApp* app = malloc(sizeof(EspFlasherApp));
    
    app->gui = furi_record_open(RECORD_GUI);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->expansion = furi_record_open(RECORD_EXPANSION);
    
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&esp_flasher_scene_handlers, app);
    
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, esp_flasher_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, esp_flasher_back_event_callback);
    
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    
    // Initialize views
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(app->view_dispatcher, EspFlasherViewSubmenu, submenu_get_view(app->submenu));
    
    app->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(app->view_dispatcher, EspFlasherViewDialogEx, dialog_ex_get_view(app->dialog_ex));
    
    app->text_input = text_input_alloc();
    view_dispatcher_add_view(app->view_dispatcher, EspFlasherViewTextInput, text_input_get_view(app->text_input));
    
    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, EspFlasherViewPopup, popup_get_view(app->popup));
    
    app->loading = loading_alloc();
    view_dispatcher_add_view(app->view_dispatcher, EspFlasherViewLoading, loading_get_view(app->loading));
    
    // Initialize app state
    app->firmware_path = furi_string_alloc();
    app->selected_chip = EspFlasherChipTypeUnknown;
    app->worker_thread = NULL;
    app->worker_running = false;
    
    return app;
}

static void esp_flasher_app_free(EspFlasherApp* app) {
    furi_assert(app);
    
    // Stop worker if running
    if(app->worker_running && app->worker_thread) {
        app->worker_running = false;
        furi_thread_join(app->worker_thread);
        furi_thread_free(app->worker_thread);
    }
    
    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, EspFlasherViewLoading);
    loading_free(app->loading);
    
    view_dispatcher_remove_view(app->view_dispatcher, EspFlasherViewPopup);
    popup_free(app->popup);
    
    view_dispatcher_remove_view(app->view_dispatcher, EspFlasherViewTextInput);
    text_input_free(app->text_input);
    
    view_dispatcher_remove_view(app->view_dispatcher, EspFlasherViewDialogEx);
    dialog_ex_free(app->dialog_ex);
    
    view_dispatcher_remove_view(app->view_dispatcher, EspFlasherViewSubmenu);
    submenu_free(app->submenu);
    
    // Free managers
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);
    
    // Free strings
    furi_string_free(app->firmware_path);
    
    // Close records
    furi_record_close(RECORD_EXPANSION);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_GUI);
    
    free(app);
}

int32_t esp_flasher_app(void* p) {
    UNUSED(p);
    
    // Ensure ESP_FLASHER_APP_FOLDER exists
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(storage, ESP_FLASHER_APP_FOLDER);
    furi_record_close(RECORD_STORAGE);
    
    EspFlasherApp* app = esp_flasher_app_alloc();
    
    scene_manager_next_scene(app->scene_manager, EspFlasherSceneStart);
    view_dispatcher_run(app->view_dispatcher);
    
    esp_flasher_app_free(app);
    
    return 0;
}