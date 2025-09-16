#include "../esp_flasher.h"
#include "../esp_flasher_rtl87xx.h"

typedef struct {
    EspFlasherApp* app;
    size_t progress_current;
    size_t progress_total;
    bool success;
    FuriString* error_message;
} FlashWorkerContext;

static void flash_progress_callback(size_t current, size_t total) {
    // This will be called from the worker thread
    // Progress updates will be handled via the worker context
}

static int32_t esp_flasher_worker_thread(void* context) {
    FlashWorkerContext* worker_ctx = context;
    EspFlasherApp* app = worker_ctx->app;
    
    worker_ctx->success = false;
    furi_string_reset(worker_ctx->error_message);
    
    FURI_LOG_I("EspFlasher", "Starting flash operation for chip type: %d", app->selected_chip);
    
    switch(app->selected_chip) {
        case EspFlasherChipTypeRTL8710BN: {
            RTL8710BNContext rtl_ctx;
            
            if(!rtl8710bn_init(&rtl_ctx)) {
                furi_string_set(worker_ctx->error_message, "Failed to initialize RTL8710BN");
                break;
            }
            
            // Enter download mode
            if(!rtl8710bn_enter_download_mode(&rtl_ctx)) {
                furi_string_set(worker_ctx->error_message, "Failed to enter download mode");
                rtl8710bn_deinit(&rtl_ctx);
                break;
            }
            
            furi_delay_ms(500); // Give time for chip to enter download mode
            
            // Sync with chip
            if(!rtl8710bn_sync(&rtl_ctx)) {
                furi_string_set(worker_ctx->error_message, "Failed to sync with RTL8710BN");
                rtl8710bn_exit_download_mode(&rtl_ctx);
                rtl8710bn_deinit(&rtl_ctx);
                break;
            }
            
            // Detect chip
            if(!rtl8710bn_detect_chip(&rtl_ctx)) {
                furi_string_set(worker_ctx->error_message, "RTL8710BN chip not detected");
                rtl8710bn_exit_download_mode(&rtl_ctx);
                rtl8710bn_deinit(&rtl_ctx);
                break;
            }
            
            // Flash firmware
            if(!rtl8710bn_flash_firmware(&rtl_ctx, furi_string_get_cstr(app->firmware_path), flash_progress_callback)) {
                furi_string_set(worker_ctx->error_message, "Failed to flash firmware");
                rtl8710bn_exit_download_mode(&rtl_ctx);
                rtl8710bn_deinit(&rtl_ctx);
                break;
            }
            
            // Exit download mode
            rtl8710bn_exit_download_mode(&rtl_ctx);
            rtl8710bn_deinit(&rtl_ctx);
            
            worker_ctx->success = true;
            break;
        }
        
        case EspFlasherChipTypeESP32:
        case EspFlasherChipTypeESP8266:
            // ESP chips not implemented yet - placeholder for future expansion
            furi_string_set(worker_ctx->error_message, "ESP chip support not implemented yet");
            break;
            
        default:
            furi_string_set(worker_ctx->error_message, "Unknown chip type");
            break;
    }
    
    // Send completion event to main thread
    if(worker_ctx->success) {
        view_dispatcher_send_custom_event(app->view_dispatcher, EspFlasherCustomEventFlashingSuccess);
    } else {
        view_dispatcher_send_custom_event(app->view_dispatcher, EspFlasherCustomEventFlashingError);
    }
    
    return 0;
}

void esp_flasher_scene_flashing_on_enter(void* context) {
    EspFlasherApp* app = context;
    
    // Show loading screen
    loading_set_context(app->loading, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, EspFlasherViewLoading);
    
    // Setup worker context
    FlashWorkerContext* worker_ctx = malloc(sizeof(FlashWorkerContext));
    worker_ctx->app = app;
    worker_ctx->progress_current = 0;
    worker_ctx->progress_total = 0;
    worker_ctx->success = false;
    worker_ctx->error_message = furi_string_alloc();
    
    // Start worker thread
    app->worker_thread = furi_thread_alloc_ex(
        "EspFlasherWorker",
        8192,
        esp_flasher_worker_thread,
        worker_ctx);
    
    app->worker_running = true;
    furi_thread_start(app->worker_thread);
    
    // Store worker context for cleanup
    scene_manager_set_scene_state(app->scene_manager, EspFlasherSceneFlashing, (uint32_t)worker_ctx);
}

bool esp_flasher_scene_flashing_on_event(void* context, SceneManagerEvent event) {
    EspFlasherApp* app = context;
    bool consumed = false;
    
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case EspFlasherCustomEventFlashingSuccess:
            scene_manager_next_scene(app->scene_manager, EspFlasherSceneSuccess);
            consumed = true;
            break;
        case EspFlasherCustomEventFlashingError:
            scene_manager_next_scene(app->scene_manager, EspFlasherSceneError);
            consumed = true;
            break;
        }
    }
    
    return consumed;
}

void esp_flasher_scene_flashing_on_exit(void* context) {
    EspFlasherApp* app = context;
    
    // Stop worker thread if still running
    if(app->worker_running && app->worker_thread) {
        app->worker_running = false;
        furi_thread_join(app->worker_thread);
        
        // Get worker context for cleanup
        FlashWorkerContext* worker_ctx = (FlashWorkerContext*)scene_manager_get_scene_state(
            app->scene_manager, EspFlasherSceneFlashing);
        
        if(worker_ctx) {
            furi_string_free(worker_ctx->error_message);
            free(worker_ctx);
        }
        
        furi_thread_free(app->worker_thread);
        app->worker_thread = NULL;
    }
    
    scene_manager_set_scene_state(app->scene_manager, EspFlasherSceneFlashing, 0);
}