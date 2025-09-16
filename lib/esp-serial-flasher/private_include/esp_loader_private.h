#pragma once

// Private ESP loader definitions

#include <stdint.h>

// Internal ESP loader constants
#define ESP_LOADER_BLOCK_SIZE_MAX 1024
#define ESP_LOADER_TIMEOUT_DEFAULT 1000

// Internal function declarations
int slip_encode(uint8_t* dst, const uint8_t* src, int size);
int slip_decode(uint8_t* dst, const uint8_t* src, int size);

// MD5 functions
uint32_t md5_hash_buffer(const uint8_t* buffer, uint32_t length);

// ESP target functions
const char* esp_get_chip_name(uint32_t chip_id);
uint32_t esp_get_flash_size(uint32_t chip_id);
uint32_t esp_get_ram_size(uint32_t chip_id);