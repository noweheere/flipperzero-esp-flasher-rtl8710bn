#include "rtl_loader.h"
#include <string.h>

// RTL8710BN download mode command sequences
static const uint8_t RTL_SYNC_CMD[] = {0x7E, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E};
static const uint8_t RTL_CHIP_ID_CMD[] = {0x7E, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E};
static const uint8_t RTL_FLASH_BEGIN_CMD[] = {0x7E, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E};

#define RTL_RESPONSE_TIMEOUT_MS 1000
#define RTL_SYNC_RETRIES 10
#define RTL_MAX_PACKET_SIZE 1024

static uint32_t detected_chip_id = 0;

// Calculate simple checksum for RTL packets
static uint8_t rtl_calculate_checksum(const uint8_t* data, uint16_t len) {
    uint8_t checksum = 0;
    for(uint16_t i = 0; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

// Send RTL command packet
static rtl_loader_error_t rtl_send_command(const uint8_t* cmd, uint16_t cmd_len, uint8_t* response, uint16_t* response_len) {
    rtl_loader_error_t err;
    
    // Send command
    err = rtl_loader_port_write(cmd, cmd_len, RTL_RESPONSE_TIMEOUT_MS);
    if(err != RTL_LOADER_SUCCESS) {
        return err;
    }
    
    // Read response if expected
    if(response && response_len && *response_len > 0) {
        err = rtl_loader_port_read(response, *response_len, RTL_RESPONSE_TIMEOUT_MS);
        if(err != RTL_LOADER_SUCCESS) {
            return err;
        }
    }
    
    return RTL_LOADER_SUCCESS;
}

rtl_loader_error_t rtl_loader_detect_chip(uint32_t* chip_id) {
    rtl_loader_error_t err;
    uint8_t response[16];
    uint16_t response_len = sizeof(response);
    
    if(!chip_id) {
        return RTL_LOADER_ERROR_FAIL;
    }
    
    // Send chip ID command
    err = rtl_send_command(RTL_CHIP_ID_CMD, sizeof(RTL_CHIP_ID_CMD), response, &response_len);
    if(err != RTL_LOADER_SUCCESS) {
        return err;
    }
    
    // Parse chip ID from response (assuming it's in bytes 4-7)
    if(response_len >= 8) {
        *chip_id = (response[4] << 24) | (response[5] << 16) | (response[6] << 8) | response[7];
        
        // Validate RTL8710BN chip ID
        if(*chip_id == RTL8710BN_CHIP_ID) {
            detected_chip_id = *chip_id;
            rtl_loader_port_debug_print("RTL8710BN chip detected\n");
            return RTL_LOADER_SUCCESS;
        } else {
            rtl_loader_port_debug_print("Unknown RTL chip detected\n");
            return RTL_LOADER_ERROR_UNSUPPORTED_CHIP;
        }
    }
    
    return RTL_LOADER_ERROR_INVALID_RESPONSE;
}

rtl_loader_error_t rtl_loader_connect(void) {
    rtl_loader_error_t err;
    uint8_t response[16];
    uint16_t response_len = sizeof(response);
    
    rtl_loader_port_debug_print("Attempting to connect to RTL chip...\n");
    
    // Try to sync with the chip multiple times
    for(int retry = 0; retry < RTL_SYNC_RETRIES; retry++) {
        err = rtl_send_command(RTL_SYNC_CMD, sizeof(RTL_SYNC_CMD), response, &response_len);
        if(err == RTL_LOADER_SUCCESS) {
            rtl_loader_port_debug_print("RTL chip sync successful\n");
            
            // Detect chip type after successful sync
            uint32_t chip_id;
            err = rtl_loader_detect_chip(&chip_id);
            if(err == RTL_LOADER_SUCCESS) {
                return RTL_LOADER_SUCCESS;
            }
        }
        
        rtl_loader_port_delay_ms(100);
    }
    
    rtl_loader_port_debug_print("Failed to sync with RTL chip\n");
    return RTL_LOADER_ERROR_TIMEOUT;
}

rtl_loader_error_t rtl_loader_enter_download_mode(void) {
    rtl_loader_port_debug_print("Entering RTL download mode...\n");
    rtl_loader_port_enter_download_mode();
    rtl_loader_port_delay_ms(RTL_DOWNLOAD_MODE_HOLD_MS);
    return RTL_LOADER_SUCCESS;
}

rtl_loader_error_t rtl_loader_reset_chip(void) {
    rtl_loader_port_debug_print("Resetting RTL chip...\n");
    rtl_loader_port_reset_target();
    rtl_loader_port_delay_ms(RTL_RESET_HOLD_MS);
    return RTL_LOADER_SUCCESS;
}

rtl_loader_error_t rtl_loader_flash_begin(uint32_t offset, uint32_t size) {
    rtl_loader_error_t err;
    uint8_t cmd[16];
    uint8_t response[16];
    uint16_t response_len = sizeof(response);
    
    // Prepare flash begin command with offset and size
    cmd[0] = 0x7E;
    cmd[1] = 0x00;
    cmd[2] = 0x02; // Flash begin command
    cmd[3] = 0x00;
    cmd[4] = (offset >> 24) & 0xFF;
    cmd[5] = (offset >> 16) & 0xFF;
    cmd[6] = (offset >> 8) & 0xFF;
    cmd[7] = offset & 0xFF;
    cmd[8] = (size >> 24) & 0xFF;
    cmd[9] = (size >> 16) & 0xFF;
    cmd[10] = (size >> 8) & 0xFF;
    cmd[11] = size & 0xFF;
    cmd[12] = rtl_calculate_checksum(cmd, 12);
    cmd[13] = 0x7E;
    
    err = rtl_send_command(cmd, 14, response, &response_len);
    if(err != RTL_LOADER_SUCCESS) {
        rtl_loader_port_debug_print("Flash begin command failed\n");
        return err;
    }
    
    rtl_loader_port_debug_print("Flash begin successful\n");
    return RTL_LOADER_SUCCESS;
}

rtl_loader_error_t rtl_loader_flash_write(const uint8_t* data, uint32_t size) {
    rtl_loader_error_t err;
    uint32_t remaining = size;
    uint32_t offset = 0;
    
    while(remaining > 0) {
        uint32_t chunk_size = (remaining > RTL_MAX_PACKET_SIZE) ? RTL_MAX_PACKET_SIZE : remaining;
        
        // Create flash write packet
        uint8_t packet[RTL_MAX_PACKET_SIZE + 16];
        packet[0] = 0x7E;
        packet[1] = 0x00;
        packet[2] = 0x03; // Flash write command
        packet[3] = 0x00;
        packet[4] = (chunk_size >> 8) & 0xFF;
        packet[5] = chunk_size & 0xFF;
        
        // Copy data
        memcpy(&packet[6], &data[offset], chunk_size);
        
        // Add checksum and end marker
        packet[6 + chunk_size] = rtl_calculate_checksum(packet, 6 + chunk_size);
        packet[7 + chunk_size] = 0x7E;
        
        err = rtl_loader_port_write(packet, 8 + chunk_size, RTL_RESPONSE_TIMEOUT_MS);
        if(err != RTL_LOADER_SUCCESS) {
            rtl_loader_port_debug_print("Flash write failed\n");
            return err;
        }
        
        offset += chunk_size;
        remaining -= chunk_size;
    }
    
    return RTL_LOADER_SUCCESS;
}

rtl_loader_error_t rtl_loader_flash_finish(void) {
    static const uint8_t finish_cmd[] = {0x7E, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x7E};
    uint8_t response[16];
    uint16_t response_len = sizeof(response);
    
    rtl_loader_error_t err = rtl_send_command(finish_cmd, sizeof(finish_cmd), response, &response_len);
    if(err != RTL_LOADER_SUCCESS) {
        rtl_loader_port_debug_print("Flash finish command failed\n");
        return err;
    }
    
    rtl_loader_port_debug_print("Flash finish successful\n");
    return RTL_LOADER_SUCCESS;
}

rtl_loader_error_t rtl_loader_flash_verify(uint32_t offset, const uint8_t* data, uint32_t size) {
    // For now, just return success - verification can be implemented later
    (void)offset;
    (void)data;
    (void)size;
    rtl_loader_port_debug_print("Flash verification skipped (not implemented)\n");
    return RTL_LOADER_SUCCESS;
}