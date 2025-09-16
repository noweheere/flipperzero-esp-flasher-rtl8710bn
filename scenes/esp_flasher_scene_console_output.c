#include "../esp_flasher_app_i.h"
#include "../esp_flasher_worker.h"

void esp_flasher_console_output_handle_rx_data_cb(uint8_t* buf, size_t len, void* context) {
    furi_assert(context);
    EspFlasherApp* app = context;

    // If text box store gets too big, then truncate it
    app->text_box_store_strlen += len;
    if(app->text_box_store_strlen >= ESP_FLASHER_TEXT_BOX_STORE_SIZE - 1) {
        furi_string_right(app->text_box_store, app->text_box_store_strlen / 2);
        app->text_box_store_strlen = furi_string_size(app->text_box_store) + len;
    }

    // Null-terminate incoming data
    uint8_t* null_terminated_buf = malloc(len + 1);
    memcpy(null_terminated_buf, buf, len);
    null_terminated_buf[len] = '\0';

    furi_string_cat_printf(app->text_box_store, "%s", null_terminated_buf);
    text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));

    free(null_terminated_buf);
}

void esp_flasher_scene_console_output_on_enter(void* context) {
    EspFlasherApp* app = context;

    TextBox* text_box = app->text_box;
    text_box_reset(app->text_box);
    text_box_set_font(text_box, TextBoxFontText);

    if(app->chip_type == ChipTypeESP) {
        if(app->reset) {
            furi_string_cat_printf(app->text_box_store, "Resetting ESP board...\n");
        } else if(app->boot) {
            furi_string_cat_printf(app->text_box_store, "Entering ESP bootloader...\n");
        } else {
            furi_string_cat_printf(app->text_box_store, "Connecting to ESP chip...\n");
        }
    } else {
        if(app->reset) {
            furi_string_cat_printf(app->text_box_store, "Resetting RTL board...\n");
        } else {
            furi_string_cat_printf(app->text_box_store, "Connecting to RTL8710BN chip...\n");
        }
    }

    text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));

    view_dispatcher_switch_to_view(app->view_dispatcher, EspFlasherAppViewConsoleOutput);

    esp_flasher_uart_set_handle_rx_data_cb(app->uart, esp_flasher_worker_handle_rx_data_cb);
    esp_flasher_worker_start_thread(app);
}

bool esp_flasher_scene_console_output_on_event(void* context, SceneManagerEvent event) {
    EspFlasherApp* app = context;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
    }

    return consumed;
}

void esp_flasher_scene_console_output_on_exit(void* context) {
    EspFlasherApp* app = context;

    // Automatically stop worker
    esp_flasher_worker_stop_thread(app);
}