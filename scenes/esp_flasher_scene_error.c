#include "../esp_flasher.h"

typedef struct {
    EspFlasherApp* app;
    size_t progress_current;
    size_t progress_total;
    bool success;
    FuriString* error_message;
} FlashWorkerContext;

void esp_flasher_scene_error_on_enter(void* context) {
    EspFlasherApp* app = context;
    
    // Get error message from worker context if available
    FlashWorkerContext* worker_ctx = (FlashWorkerContext*)scene_manager_get_scene_state(
        app->scene_manager, EspFlasherSceneFlashing);
    
    FuriString* error_text = furi_string_alloc();
    
    if(worker_ctx && furi_string_size(worker_ctx->error_message) > 0) {
        furi_string_printf(error_text,
            "Flash operation failed!\n\n"
            "Error: %s\n\n"
            "Please check:\n"
            "• Wiring connections\n"
            "• Power supply (3.3V for RTL)\n"
            "• Firmware file validity\n"
            "• Chip compatibility",
            furi_string_get_cstr(worker_ctx->error_message));
    } else {
        furi_string_set(error_text,
            "Flash operation failed!\n\n"
            "Unknown error occurred.\n\n"
            "Please check:\n"
            "• Wiring connections\n"
            "• Power supply\n"
            "• Firmware file\n"
            "• Chip compatibility");
    }
    
    dialog_ex_reset(app->dialog_ex);
    dialog_ex_set_header(app->dialog_ex, "Error", 64, 10, AlignCenter, AlignCenter);
    dialog_ex_set_text(app->dialog_ex, furi_string_get_cstr(error_text), 64, 32, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(app->dialog_ex, "Back");
    dialog_ex_set_right_button_text(app->dialog_ex, "Retry");
    dialog_ex_set_context(app->dialog_ex, app);
    dialog_ex_set_result_callback(app->dialog_ex, NULL);
    
    // Show error notification
    notification_message(app->notifications, &sequence_error);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, EspFlasherViewDialogEx);
    
    furi_string_free(error_text);
}

bool esp_flasher_scene_error_on_event(void* context, SceneManagerEvent event) {
    EspFlasherApp* app = context;
    bool consumed = false;
    
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultLeft:
            scene_manager_search_and_switch_to_previous_scene(app->scene_manager, EspFlasherSceneStart);
            consumed = true;
            break;
        case DialogExResultRight:
            scene_manager_search_and_switch_to_previous_scene(app->scene_manager, EspFlasherSceneWiring);
            consumed = true;
            break;
        }
    }
    
    return consumed;
}

void esp_flasher_scene_error_on_exit(void* context) {
    EspFlasherApp* app = context;
    dialog_ex_reset(app->dialog_ex);
}