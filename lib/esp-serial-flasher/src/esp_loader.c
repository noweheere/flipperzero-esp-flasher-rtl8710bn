#include "esp_loader.h"
#include <string.h>

// Simple ESP loader implementation for Flipper Zero
// This implements a basic ESP32 ROM bootloader protocol

#define ESP_SLIP_END     0xC0
#define ESP_SLIP_ESC     0xDB
#define ESP_SLIP_ESC_END 0xDC
#define ESP_SLIP_ESC_ESC 0xDD

#define ESP_FLASH_BEGIN   0x02
#define ESP_FLASH_DATA    0x03
#define ESP_FLASH_END     0x04
#define ESP_SYNC          0x08

#define ESP_CHECKSUM_MAGIC 0xEF

static uint32_t s_flash_write_size = 0;

static void slip_encode(uint8_t* dst, const uint8_t* src, size_t size, size_t* encoded_size) {
    size_t j = 0;
    dst[j++] = ESP_SLIP_END;
    
    for(size_t i = 0; i < size; i++) {
        if(src[i] == ESP_SLIP_END) {
            dst[j++] = ESP_SLIP_ESC;
            dst[j++] = ESP_SLIP_ESC_END;
        } else if(src[i] == ESP_SLIP_ESC) {
            dst[j++] = ESP_SLIP_ESC;
            dst[j++] = ESP_SLIP_ESC_ESC;
        } else {
            dst[j++] = src[i];
        }
    }
    
    dst[j++] = ESP_SLIP_END;
    *encoded_size = j;
}

static esp_loader_error_t send_command(uint8_t command, const uint8_t* data, uint16_t size, uint32_t checksum) {
    uint8_t header[8];
    uint8_t packet[1024];
    size_t encoded_size = 0;
    
    // Build packet header
    header[0] = 0x00; // direction
    header[1] = command;
    header[2] = size & 0xFF;
    header[3] = (size >> 8) & 0xFF;
    header[4] = checksum & 0xFF;
    header[5] = (checksum >> 8) & 0xFF;
    header[6] = (checksum >> 16) & 0xFF;
    header[7] = (checksum >> 24) & 0xFF;
    
    // SLIP encode header
    slip_encode(packet, header, 8, &encoded_size);
    
    // Send header
    esp_loader_error_t err = loader_port_write(packet, encoded_size, 100);
    if(err != ESP_LOADER_SUCCESS) return err;
    
    // Send data if present
    if(data && size > 0) {
        slip_encode(packet, data, size, &encoded_size);
        err = loader_port_write(packet, encoded_size, 100);
        if(err != ESP_LOADER_SUCCESS) return err;
    }
    
    return ESP_LOADER_SUCCESS;
}

esp_loader_error_t esp_loader_connect(esp_loader_connect_args_t* config) {
    uint8_t sync_data[36] = {
        0x07, 0x07, 0x12, 0x20,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55
    };
    
    for(uint32_t trial = 0; trial < config->trials; trial++) {
        loader_port_debug_print("Connecting...\n");
        
        // Send sync command
        esp_loader_error_t err = send_command(ESP_SYNC, sync_data, sizeof(sync_data), 0);
        if(err != ESP_LOADER_SUCCESS) continue;
        
        // Wait for response
        uint8_t response[16];
        err = loader_port_read(response, 1, config->sync_timeout);
        if(err == ESP_LOADER_SUCCESS) {
            loader_port_debug_print("ESP connected\n");
            return ESP_LOADER_SUCCESS;
        }
        
        loader_port_delay_ms(100);
    }
    
    return ESP_LOADER_ERROR_TIMEOUT;
}

esp_loader_error_t esp_loader_flash_start(uint32_t offset, uint32_t image_size, uint32_t block_size) {
    uint8_t flash_begin_data[16];
    
    // Prepare flash begin parameters
    flash_begin_data[0] = image_size & 0xFF;
    flash_begin_data[1] = (image_size >> 8) & 0xFF;
    flash_begin_data[2] = (image_size >> 16) & 0xFF;
    flash_begin_data[3] = (image_size >> 24) & 0xFF;
    
    flash_begin_data[4] = 0; // num_blocks - will be calculated by ROM
    flash_begin_data[5] = 0;
    flash_begin_data[6] = 0;
    flash_begin_data[7] = 0;
    
    flash_begin_data[8] = block_size & 0xFF;
    flash_begin_data[9] = (block_size >> 8) & 0xFF;
    flash_begin_data[10] = (block_size >> 16) & 0xFF;
    flash_begin_data[11] = (block_size >> 24) & 0xFF;
    
    flash_begin_data[12] = offset & 0xFF;
    flash_begin_data[13] = (offset >> 8) & 0xFF;
    flash_begin_data[14] = (offset >> 16) & 0xFF;
    flash_begin_data[15] = (offset >> 24) & 0xFF;
    
    s_flash_write_size = block_size;
    
    return send_command(ESP_FLASH_BEGIN, flash_begin_data, 16, 0);
}

esp_loader_error_t esp_loader_flash_write(const void* payload, uint32_t size) {
    if(!payload || size == 0) return ESP_LOADER_ERROR_FAIL;
    
    // Calculate checksum
    uint32_t checksum = ESP_CHECKSUM_MAGIC;
    const uint8_t* data = (const uint8_t*)payload;
    for(uint32_t i = 0; i < size; i++) {
        checksum ^= data[i];
    }
    
    // Build flash data packet with size prefix
    uint8_t flash_data[4];
    flash_data[0] = size & 0xFF;
    flash_data[1] = (size >> 8) & 0xFF;
    flash_data[2] = (size >> 16) & 0xFF;
    flash_data[3] = (size >> 24) & 0xFF;
    
    // Send size first
    esp_loader_error_t err = send_command(ESP_FLASH_DATA, flash_data, 4, 0);
    if(err != ESP_LOADER_SUCCESS) return err;
    
    // Send data with checksum
    return send_command(ESP_FLASH_DATA, payload, size, checksum);
}

esp_loader_error_t esp_loader_change_transmission_rate(uint32_t transmission_rate) {
    (void)transmission_rate;
    // Stub - would implement baud rate change
    return ESP_LOADER_SUCCESS;
}