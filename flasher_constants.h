#pragma once

// ESP memory addresses
#define ESP_ADDR_BOOT_C5     0x0
#define ESP_ADDR_BOOT_S3     0x0
#define ESP_ADDR_BOOT        0x1000
#define ESP_ADDR_PART        0x8000
#define ESP_ADDR_NVS         0x9000
#define ESP_ADDR_BOOT_APP0   0xe000
#define ESP_ADDR_APP_A       0x10000
#define ESP_ADDR_APP_B       0x100000

#define ESP_ADDR_OTADATA_OFFSET_APP_A   0x0
#define ESP_ADDR_OTADATA_OFFSET_APP_B   0x1000

// Serial flasher timing constants
#define SERIAL_FLASHER_RESET_HOLD_TIME_MS   100
#define SERIAL_FLASHER_BOOT_HOLD_TIME_MS    50

// Include esp-serial-flasher if building with ESP support
#ifndef RTL_ONLY_BUILD
#include "esp_loader.h"
#endif