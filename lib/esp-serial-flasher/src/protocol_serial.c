// Protocol serial implementation for ESP flasher
// Basic serial protocol wrapper functions

#include <stdint.h>

int serial_init(void) {
    // Serial initialization is handled by the platform-specific UART code
    return 0;
}

int serial_write(const uint8_t* data, int size) {
    // This would be implemented by the platform port layer
    // For Flipper Zero, this is handled in esp_flasher_worker.c
    (void)data;
    (void)size;
    return size;
}

int serial_read(uint8_t* data, int size, int timeout_ms) {
    // This would be implemented by the platform port layer  
    // For Flipper Zero, this is handled in esp_flasher_worker.c
    (void)data;
    (void)size;
    (void)timeout_ms;
    return 0;
}

void serial_flush(void) {
    // Flush any pending data
}

void serial_close(void) {
    // Close serial connection
}