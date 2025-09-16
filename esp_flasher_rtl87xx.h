#pragma once

#include <furi.h>
#include <furi_hal.h>

#define RTL8710BN_CHIP_ID 0x8710
#define RTL8710BN_FLASH_SIZE 0x200000  // 2MB
#define RTL8710BN_SECTOR_SIZE 0x1000   // 4KB

// RTL8710BN UART configuration
#define RTL8710BN_UART_TX_PIN &gpio_ext_pa7
#define RTL8710BN_UART_RX_PIN &gpio_ext_pa6
#define RTL8710BN_DOWNLOAD_BAUDRATE 115200
#define RTL8710BN_FLASH_BAUDRATE 460800

// Download mode control pins
#define RTL8710BN_CEN_PIN &gpio_ext_pc0
#define RTL8710BN_TX2_PIN &gpio_ext_pc1

typedef enum {
    RTL8710BNCommandSync = 0x08,
    RTL8710BNCommandFlashBegin = 0x02,
    RTL8710BNCommandFlashData = 0x03,
    RTL8710BNCommandFlashEnd = 0x04,
    RTL8710BNCommandMemBegin = 0x05,
    RTL8710BNCommandMemEnd = 0x06,
    RTL8710BNCommandMemData = 0x07,
    RTL8710BNCommandChipId = 0x09,
} RTL8710BNCommand;

typedef struct {
    bool connected;
    uint32_t chip_id;
    uint32_t flash_size;
    uint32_t sector_size;
    FuriHalSerialHandle* serial_handle;
} RTL8710BNContext;

// Core functions
bool rtl8710bn_init(RTL8710BNContext* ctx);
void rtl8710bn_deinit(RTL8710BNContext* ctx);
bool rtl8710bn_enter_download_mode(RTL8710BNContext* ctx);
bool rtl8710bn_exit_download_mode(RTL8710BNContext* ctx);
bool rtl8710bn_detect_chip(RTL8710BNContext* ctx);
bool rtl8710bn_sync(RTL8710BNContext* ctx);
bool rtl8710bn_flash_firmware(RTL8710BNContext* ctx, const char* firmware_path, void(*progress_callback)(size_t current, size_t total));

// Utility functions
bool rtl8710bn_send_command(RTL8710BNContext* ctx, RTL8710BNCommand cmd, const uint8_t* data, size_t data_len);
bool rtl8710bn_receive_response(RTL8710BNContext* ctx, uint8_t* buffer, size_t buffer_size, size_t* received_len);
uint32_t rtl8710bn_calculate_checksum(const uint8_t* data, size_t len);