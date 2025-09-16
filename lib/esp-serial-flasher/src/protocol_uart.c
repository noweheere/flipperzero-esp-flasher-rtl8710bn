// Protocol UART implementation for ESP flasher
// Platform-specific UART handling

#include <stdint.h>

// UART configuration structure
typedef struct {
    int baudrate;
    int data_bits;
    int stop_bits;
    int parity;
} uart_config_t;

int uart_init(uart_config_t* config) {
    // UART initialization 
    // For Flipper Zero, this is handled in esp_flasher_uart.c
    (void)config;
    return 0;
}

int uart_write(const uint8_t* data, int size) {
    // Write data to UART
    // For Flipper Zero, this is handled by the loader port functions
    (void)data;
    (void)size;
    return size;
}

int uart_read(uint8_t* data, int size, int timeout_ms) {
    // Read data from UART with timeout
    // For Flipper Zero, this is handled by the loader port functions
    (void)data;
    (void)size;
    (void)timeout_ms;
    return 0;
}

void uart_flush(void) {
    // Flush UART buffers
}

void uart_set_baudrate(int baudrate) {
    // Change UART baudrate
    (void)baudrate;
}

void uart_close(void) {
    // Close UART
}