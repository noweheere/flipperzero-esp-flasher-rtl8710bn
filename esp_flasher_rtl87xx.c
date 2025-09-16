#include "esp_flasher_rtl87xx.h"
#include <furi_hal_serial.h>
#include <furi_hal_gpio.h>
#include <storage/storage.h>
#include <furi_hal_resources.h>

#define RTL_SYNC_TIMEOUT_MS 5000
#define RTL_RESPONSE_TIMEOUT_MS 1000
#define RTL_FLASH_BLOCK_SIZE 1024

static bool rtl8710bn_wait_for_response(RTL8710BNContext* ctx, uint32_t timeout_ms) {
    uint32_t start_time = furi_get_tick();
    while((furi_get_tick() - start_time) < timeout_ms) {
        if(furi_hal_serial_rx_bytes_available(ctx->serial_handle) > 0) {
            return true;
        }
        furi_delay_ms(1);
    }
    return false;
}

bool rtl8710bn_init(RTL8710BNContext* ctx) {
    if(!ctx) return false;
    
    memset(ctx, 0, sizeof(RTL8710BNContext));
    
    // Initialize serial communication
    ctx->serial_handle = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
    if(!ctx->serial_handle) {
        return false;
    }
    
    furi_hal_serial_init(ctx->serial_handle, RTL8710BN_DOWNLOAD_BAUDRATE);
    
    // Configure GPIO pins for download mode control
    furi_hal_gpio_init_simple(RTL8710BN_CEN_PIN, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(RTL8710BN_TX2_PIN, GpioModeOutputPushPull);
    
    // Set default pin states (not in download mode)
    furi_hal_gpio_write(RTL8710BN_CEN_PIN, true);  // CEN high
    furi_hal_gpio_write(RTL8710BN_TX2_PIN, true);  // TX2 high
    
    ctx->flash_size = RTL8710BN_FLASH_SIZE;
    ctx->sector_size = RTL8710BN_SECTOR_SIZE;
    
    return true;
}

void rtl8710bn_deinit(RTL8710BNContext* ctx) {
    if(!ctx) return;
    
    if(ctx->serial_handle) {
        furi_hal_serial_deinit(ctx->serial_handle);
        furi_hal_serial_control_release(ctx->serial_handle);
        ctx->serial_handle = NULL;
    }
    
    // Reset GPIO pins
    furi_hal_gpio_init_simple(RTL8710BN_CEN_PIN, GpioModeAnalog);
    furi_hal_gpio_init_simple(RTL8710BN_TX2_PIN, GpioModeAnalog);
    
    ctx->connected = false;
}

bool rtl8710bn_enter_download_mode(RTL8710BNContext* ctx) {
    if(!ctx) return false;
    
    FURI_LOG_I("RTL8710BN", "Entering download mode");
    
    // RTL8710BN download mode sequence: CEN + TX2 to GND
    furi_hal_gpio_write(RTL8710BN_CEN_PIN, false);  // CEN to GND
    furi_hal_gpio_write(RTL8710BN_TX2_PIN, false);  // TX2 to GND
    furi_delay_ms(100);
    
    // Release CEN while keeping TX2 low
    furi_hal_gpio_write(RTL8710BN_CEN_PIN, true);   // Release CEN
    furi_delay_ms(100);
    
    // Release TX2
    furi_hal_gpio_write(RTL8710BN_TX2_PIN, true);   // Release TX2
    furi_delay_ms(100);
    
    return true;
}

bool rtl8710bn_exit_download_mode(RTL8710BNContext* ctx) {
    if(!ctx) return false;
    
    FURI_LOG_I("RTL8710BN", "Exiting download mode");
    
    // Reset the chip
    furi_hal_gpio_write(RTL8710BN_CEN_PIN, false);
    furi_delay_ms(100);
    furi_hal_gpio_write(RTL8710BN_CEN_PIN, true);
    
    return true;
}

bool rtl8710bn_sync(RTL8710BNContext* ctx) {
    if(!ctx || !ctx->serial_handle) return false;
    
    FURI_LOG_I("RTL8710BN", "Syncing with chip");
    
    // Send sync command multiple times
    for(int i = 0; i < 5; i++) {
        uint8_t sync_cmd = RTL8710BNCommandSync;
        furi_hal_serial_tx(ctx->serial_handle, &sync_cmd, 1);
        
        if(rtl8710bn_wait_for_response(ctx, RTL_SYNC_TIMEOUT_MS)) {
            uint8_t response;
            size_t received = furi_hal_serial_rx(ctx->serial_handle, &response, 1);
            if(received > 0 && response == 0x01) {
                FURI_LOG_I("RTL8710BN", "Sync successful");
                return true;
            }
        }
        furi_delay_ms(100);
    }
    
    FURI_LOG_E("RTL8710BN", "Sync failed");
    return false;
}

bool rtl8710bn_detect_chip(RTL8710BNContext* ctx) {
    if(!ctx || !ctx->serial_handle) return false;
    
    FURI_LOG_I("RTL8710BN", "Detecting chip");
    
    // Send chip ID command
    uint8_t cmd = RTL8710BNCommandChipId;
    furi_hal_serial_tx(ctx->serial_handle, &cmd, 1);
    
    if(rtl8710bn_wait_for_response(ctx, RTL_RESPONSE_TIMEOUT_MS)) {
        uint8_t response[4];
        size_t received = furi_hal_serial_rx(ctx->serial_handle, response, sizeof(response));
        if(received >= 2) {
            ctx->chip_id = (response[1] << 8) | response[0];
            FURI_LOG_I("RTL8710BN", "Chip ID: 0x%04X", ctx->chip_id);
            
            if(ctx->chip_id == RTL8710BN_CHIP_ID) {
                ctx->connected = true;
                return true;
            }
        }
    }
    
    FURI_LOG_E("RTL8710BN", "Chip detection failed");
    return false;
}

bool rtl8710bn_send_command(RTL8710BNContext* ctx, RTL8710BNCommand cmd, const uint8_t* data, size_t data_len) {
    if(!ctx || !ctx->serial_handle) return false;
    
    // Send command byte
    uint8_t cmd_byte = (uint8_t)cmd;
    furi_hal_serial_tx(ctx->serial_handle, &cmd_byte, 1);
    
    // Send data if provided
    if(data && data_len > 0) {
        furi_hal_serial_tx(ctx->serial_handle, data, data_len);
    }
    
    return true;
}

bool rtl8710bn_receive_response(RTL8710BNContext* ctx, uint8_t* buffer, size_t buffer_size, size_t* received_len) {
    if(!ctx || !ctx->serial_handle || !buffer || !received_len) return false;
    
    if(!rtl8710bn_wait_for_response(ctx, RTL_RESPONSE_TIMEOUT_MS)) {
        return false;
    }
    
    *received_len = furi_hal_serial_rx(ctx->serial_handle, buffer, buffer_size);
    return *received_len > 0;
}

uint32_t rtl8710bn_calculate_checksum(const uint8_t* data, size_t len) {
    uint32_t checksum = 0;
    for(size_t i = 0; i < len; i++) {
        checksum += data[i];
    }
    return checksum;
}

bool rtl8710bn_flash_firmware(RTL8710BNContext* ctx, const char* firmware_path, void(*progress_callback)(size_t current, size_t total)) {
    if(!ctx || !firmware_path) return false;
    
    FURI_LOG_I("RTL8710BN", "Starting firmware flash: %s", firmware_path);
    
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    
    bool success = false;
    
    if(!storage_file_open(file, firmware_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E("RTL8710BN", "Failed to open firmware file");
        goto cleanup;
    }
    
    uint64_t file_size = storage_file_size(file);
    if(file_size == 0 || file_size > ctx->flash_size) {
        FURI_LOG_E("RTL8710BN", "Invalid firmware size: %llu", file_size);
        goto cleanup;
    }
    
    // Switch to flash baudrate for faster transfer
    furi_hal_serial_set_br(ctx->serial_handle, RTL8710BN_FLASH_BAUDRATE);
    
    // Begin flash operation
    uint8_t flash_begin_data[16];
    memset(flash_begin_data, 0, sizeof(flash_begin_data));
    *(uint32_t*)&flash_begin_data[0] = (uint32_t)file_size;
    *(uint32_t*)&flash_begin_data[4] = (file_size + RTL_FLASH_BLOCK_SIZE - 1) / RTL_FLASH_BLOCK_SIZE;
    *(uint32_t*)&flash_begin_data[8] = RTL_FLASH_BLOCK_SIZE;
    *(uint32_t*)&flash_begin_data[12] = 0x00000000; // Flash offset
    
    if(!rtl8710bn_send_command(ctx, RTL8710BNCommandFlashBegin, flash_begin_data, sizeof(flash_begin_data))) {
        FURI_LOG_E("RTL8710BN", "Failed to send flash begin command");
        goto cleanup;
    }
    
    // Wait for response
    uint8_t response;
    size_t received;
    if(!rtl8710bn_receive_response(ctx, &response, 1, &received) || response != 0x00) {
        FURI_LOG_E("RTL8710BN", "Flash begin command failed");
        goto cleanup;
    }
    
    // Flash data in blocks
    uint8_t* block_buffer = malloc(RTL_FLASH_BLOCK_SIZE);
    if(!block_buffer) {
        FURI_LOG_E("RTL8710BN", "Failed to allocate block buffer");
        goto cleanup;
    }
    
    size_t total_written = 0;
    uint32_t block_number = 0;
    
    while(total_written < file_size) {
        size_t to_read = RTL_FLASH_BLOCK_SIZE;
        if(total_written + to_read > file_size) {
            to_read = file_size - total_written;
        }
        
        size_t bytes_read = storage_file_read(file, block_buffer, to_read);
        if(bytes_read == 0) {
            FURI_LOG_E("RTL8710BN", "Failed to read from firmware file");
            break;
        }
        
        // Pad block to full size if needed
        if(bytes_read < RTL_FLASH_BLOCK_SIZE) {
            memset(block_buffer + bytes_read, 0xFF, RTL_FLASH_BLOCK_SIZE - bytes_read);
        }
        
        // Prepare flash data packet
        uint8_t flash_data_header[16];
        *(uint32_t*)&flash_data_header[0] = RTL_FLASH_BLOCK_SIZE;
        *(uint32_t*)&flash_data_header[4] = block_number;
        *(uint32_t*)&flash_data_header[8] = 0; // Reserved
        *(uint32_t*)&flash_data_header[12] = 0; // Reserved
        
        // Send flash data command
        if(!rtl8710bn_send_command(ctx, RTL8710BNCommandFlashData, flash_data_header, sizeof(flash_data_header))) {
            FURI_LOG_E("RTL8710BN", "Failed to send flash data header");
            break;
        }
        
        // Send block data
        furi_hal_serial_tx(ctx->serial_handle, block_buffer, RTL_FLASH_BLOCK_SIZE);
        
        // Send checksum
        uint32_t checksum = rtl8710bn_calculate_checksum(block_buffer, RTL_FLASH_BLOCK_SIZE);
        furi_hal_serial_tx(ctx->serial_handle, (uint8_t*)&checksum, 4);
        
        // Wait for response
        if(!rtl8710bn_receive_response(ctx, &response, 1, &received) || response != 0x00) {
            FURI_LOG_E("RTL8710BN", "Flash data block %lu failed", block_number);
            break;
        }
        
        total_written += bytes_read;
        block_number++;
        
        // Update progress
        if(progress_callback) {
            progress_callback(total_written, file_size);
        }
    }
    
    free(block_buffer);
    
    if(total_written == file_size) {
        // Send flash end command
        uint8_t flash_end_data[4] = {0x01, 0x00, 0x00, 0x00}; // Reboot flag
        if(rtl8710bn_send_command(ctx, RTL8710BNCommandFlashEnd, flash_end_data, sizeof(flash_end_data))) {
            if(rtl8710bn_receive_response(ctx, &response, 1, &received) && response == 0x00) {
                FURI_LOG_I("RTL8710BN", "Firmware flash completed successfully");
                success = true;
            }
        }
    }
    
cleanup:
    if(file) {
        storage_file_close(file);
        storage_file_free(file);
    }
    furi_record_close(RECORD_STORAGE);
    
    // Reset baudrate
    furi_hal_serial_set_br(ctx->serial_handle, RTL8710BN_DOWNLOAD_BAUDRATE);
    
    return success;
}