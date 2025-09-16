#include "esp_flasher_uart.h"
#include <furi.h>
#include <furi_hal.h>

struct EspFlasherUart {
    FuriHalSerialHandle* serial_handle;
    uint8_t rx_buf[RX_BUF_SIZE];
    EspFlasherUartRxCallback callback;
    void* context;
};

static void esp_flasher_uart_on_irq_cb(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context) {
    EspFlasherUart* uart = (EspFlasherUart*)context;

    if(event == FuriHalSerialRxEventData) {
        uint8_t data = furi_hal_serial_async_rx(handle);
        // Simple buffering - in a real implementation you'd want a proper ring buffer
        if(uart->callback) {
            uart->callback(&data, 1, uart->context);
        }
    }
}

EspFlasherUart* esp_flasher_usart_init(void* context) {
    EspFlasherUart* uart = malloc(sizeof(EspFlasherUart));
    uart->callback = NULL;
    uart->context = context;

    uart->serial_handle = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
    furi_check(uart->serial_handle);

    furi_hal_serial_init(uart->serial_handle, BAUDRATE);
    furi_hal_serial_async_rx_start(uart->serial_handle, esp_flasher_uart_on_irq_cb, uart, false);

    return uart;
}

void esp_flasher_uart_free(EspFlasherUart* uart) {
    furi_assert(uart);

    furi_hal_serial_async_rx_stop(uart->serial_handle);
    furi_hal_serial_deinit(uart->serial_handle);
    furi_hal_serial_control_release(uart->serial_handle);

    free(uart);
}

void esp_flasher_uart_set_handle_rx_data_cb(
    EspFlasherUart* uart,
    EspFlasherUartRxCallback callback) {
    furi_assert(uart);
    uart->callback = callback;
}

void esp_flasher_uart_tx(EspFlasherUart* uart, uint8_t* data, size_t len) {
    furi_assert(uart);
    furi_hal_serial_tx(uart->serial_handle, data, len);
}

void esp_flasher_uart_set_br(EspFlasherUart* uart, uint32_t br) {
    furi_assert(uart);
    furi_hal_serial_set_br(uart->serial_handle, br);
}