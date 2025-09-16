#include "rtl_loader.h"

// RTL8710BN target information
typedef struct {
    uint32_t chip_id;
    const char* name;
    uint32_t flash_size;
    uint32_t block_size;
    uint32_t sector_size;
} rtl_target_info_t;

static const rtl_target_info_t rtl_targets[] = {
    {
        .chip_id = RTL8710BN_CHIP_ID,
        .name = "RTL8710BN",
        .flash_size = RTL8710BN_FLASH_SIZE,
        .block_size = RTL8710BN_BLOCK_SIZE,
        .sector_size = RTL8710BN_SECTOR_SIZE,
    },
    // Future RTL chips can be added here
};

const rtl_target_info_t* rtl_get_target_info(uint32_t chip_id) {
    for(size_t i = 0; i < sizeof(rtl_targets) / sizeof(rtl_targets[0]); i++) {
        if(rtl_targets[i].chip_id == chip_id) {
            return &rtl_targets[i];
        }
    }
    return NULL;
}

const char* rtl_get_chip_name(uint32_t chip_id) {
    const rtl_target_info_t* target = rtl_get_target_info(chip_id);
    return target ? target->name : "Unknown RTL chip";
}

uint32_t rtl_get_flash_size(uint32_t chip_id) {
    const rtl_target_info_t* target = rtl_get_target_info(chip_id);
    return target ? target->flash_size : 0;
}

uint32_t rtl_get_block_size(uint32_t chip_id) {
    const rtl_target_info_t* target = rtl_get_target_info(chip_id);
    return target ? target->block_size : 0;
}

uint32_t rtl_get_sector_size(uint32_t chip_id) {
    const rtl_target_info_t* target = rtl_get_target_info(chip_id);
    return target ? target->sector_size : 0;
}