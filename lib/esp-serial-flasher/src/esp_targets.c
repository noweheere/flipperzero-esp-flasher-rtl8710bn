#include <stdint.h>

// ESP chip target information for the ESP flasher

// ESP32 chip identification
#define ESP32_CHIP_ID           0x0000
#define ESP32_S2_CHIP_ID        0x0002
#define ESP32_S3_CHIP_ID        0x0009
#define ESP32_C3_CHIP_ID        0x0005
#define ESP32_C2_CHIP_ID        0x000C
#define ESP32_H2_CHIP_ID        0x0010

typedef struct {
    uint32_t chip_id;
    const char* name;
    uint32_t flash_size;
    uint32_t ram_size;
} esp_target_t;

static const esp_target_t esp_targets[] = {
    {ESP32_CHIP_ID, "ESP32", 0x400000, 0x50000},
    {ESP32_S2_CHIP_ID, "ESP32-S2", 0x400000, 0x50000},
    {ESP32_S3_CHIP_ID, "ESP32-S3", 0x400000, 0x80000},
    {ESP32_C3_CHIP_ID, "ESP32-C3", 0x400000, 0x50000},
    {ESP32_C2_CHIP_ID, "ESP32-C2", 0x200000, 0x50000},
    {ESP32_H2_CHIP_ID, "ESP32-H2", 0x200000, 0x50000},
};

const char* esp_get_chip_name(uint32_t chip_id) {
    for(size_t i = 0; i < sizeof(esp_targets) / sizeof(esp_targets[0]); i++) {
        if(esp_targets[i].chip_id == chip_id) {
            return esp_targets[i].name;
        }
    }
    return "Unknown ESP chip";
}

uint32_t esp_get_flash_size(uint32_t chip_id) {
    for(size_t i = 0; i < sizeof(esp_targets) / sizeof(esp_targets[0]); i++) {
        if(esp_targets[i].chip_id == chip_id) {
            return esp_targets[i].flash_size;
        }
    }
    return 0x400000; // Default 4MB
}

uint32_t esp_get_ram_size(uint32_t chip_id) {
    for(size_t i = 0; i < sizeof(esp_targets) / sizeof(esp_targets[0]); i++) {
        if(esp_targets[i].chip_id == chip_id) {
            return esp_targets[i].ram_size;
        }
    }
    return 0x50000; // Default 320KB
}