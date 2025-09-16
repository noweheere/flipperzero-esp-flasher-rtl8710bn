#pragma once

#include "esp_flasher_app.h"

#ifdef __cplusplus
extern "C" {
#endif

void esp_flasher_worker_start_thread(EspFlasherApp* app);
void esp_flasher_worker_stop_thread(EspFlasherApp* app);
void esp_flasher_worker_handle_rx_data_cb(uint8_t* buf, size_t len, void* context);

#ifdef __cplusplus
}
#endif