#include <gui/icon_i.h>

// Simple 10x10 WiFi-like icon for ESP flasher
static const uint8_t I_esp_flasher_10px_data[] = {
    0x0E, 0x00,  // ░░░░███░░░
    0x1F, 0x00,  // ░░░█████░░
    0x3F, 0x80,  // ░░███████░
    0x73, 0xC0,  // ░███░░███░
    0x61, 0x80,  // ░██░░░██░░
    0x0C, 0x00,  // ░░░░██░░░░
    0x0C, 0x00,  // ░░░░██░░░░
    0x00, 0x00,  // ░░░░░░░░░░
    0x0C, 0x00,  // ░░░░██░░░░
    0x0C, 0x00,  // ░░░░██░░░░
};

const Icon I_esp_flasher_10px = {
    .width = 10,
    .height = 10,
    .frame_count = 1,
    .frame_rate = 0,
    .frames = {I_esp_flasher_10px_data}
};