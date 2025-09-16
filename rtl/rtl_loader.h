#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// RTL8710BN chip constants
#define RTL8710BN_CHIP_ID       0x8710
#define RTL8710BN_FLASH_SIZE    (2 * 1024 * 1024)  // 2MB flash
#define RTL8710BN_BLOCK_SIZE    4096
#define RTL8710BN_SECTOR_SIZE   4096

// UART pins for RTL8710BN
#define RTL8710BN_UART_TX_PIN   29  // PA29
#define RTL8710BN_UART_RX_PIN   30  // PA30

// Download mode activation constants
#define RTL_DOWNLOAD_MODE_HOLD_MS   100
#define RTL_RESET_HOLD_MS          50

// RTL-specific error codes
typedef enum {
    RTL_LOADER_SUCCESS = 0,
    RTL_LOADER_ERROR_TIMEOUT,
    RTL_LOADER_ERROR_INVALID_RESPONSE,
    RTL_LOADER_ERROR_CHIP_NOT_DETECTED,
    RTL_LOADER_ERROR_FLASH_FAIL,
    RTL_LOADER_ERROR_VERIFY_FAIL,
    RTL_LOADER_ERROR_UNSUPPORTED_CHIP,
    RTL_LOADER_ERROR_FAIL = -1
} rtl_loader_error_t;

// RTL chip detection and connection
rtl_loader_error_t rtl_loader_detect_chip(uint32_t* chip_id);
rtl_loader_error_t rtl_loader_connect(void);
rtl_loader_error_t rtl_loader_enter_download_mode(void);
rtl_loader_error_t rtl_loader_reset_chip(void);

// RTL flash operations
rtl_loader_error_t rtl_loader_flash_begin(uint32_t offset, uint32_t size);
rtl_loader_error_t rtl_loader_flash_write(const uint8_t* data, uint32_t size);
rtl_loader_error_t rtl_loader_flash_finish(void);
rtl_loader_error_t rtl_loader_flash_verify(uint32_t offset, const uint8_t* data, uint32_t size);

// RTL loader port functions (implemented by platform)
rtl_loader_error_t rtl_loader_port_write(const uint8_t* data, uint16_t size, uint32_t timeout);
rtl_loader_error_t rtl_loader_port_read(uint8_t* data, uint16_t size, uint32_t timeout);
void rtl_loader_port_delay_ms(uint32_t ms);
void rtl_loader_port_enter_download_mode(void);
void rtl_loader_port_reset_target(void);
void rtl_loader_port_debug_print(const char* str);

#ifdef __cplusplus
}
#endif