#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ESP_LOADER_SUCCESS = 0,
    ESP_LOADER_ERROR_TIMEOUT,
    ESP_LOADER_ERROR_INVALID_RESPONSE,
    ESP_LOADER_ERROR_FAIL = -1
} esp_loader_error_t;

typedef struct {
    uint32_t sync_timeout;
    uint32_t trials;
} esp_loader_connect_args_t;

#define ESP_LOADER_CONNECT_DEFAULT() {.sync_timeout = 1000, .trials = 10}

// Function declarations
esp_loader_error_t esp_loader_connect(esp_loader_connect_args_t* config);
esp_loader_error_t esp_loader_flash_start(uint32_t offset, uint32_t image_size, uint32_t block_size);
esp_loader_error_t esp_loader_flash_write(const void* payload, uint32_t size);
esp_loader_error_t esp_loader_change_transmission_rate(uint32_t transmission_rate);

// Port functions (implemented by platform)
esp_loader_error_t loader_port_read(uint8_t* data, uint16_t size, uint32_t timeout);
esp_loader_error_t loader_port_write(const uint8_t* data, uint16_t size, uint32_t timeout);
void loader_port_reset_target(void);
void loader_port_enter_bootloader(void);
void loader_port_delay_ms(uint32_t ms);
void loader_port_start_timer(uint32_t ms);
uint32_t loader_port_remaining_time(void);
void loader_port_debug_print(const char* str);
void loader_port_spi_set_cs(uint32_t level);

#ifdef __cplusplus
}
#endif