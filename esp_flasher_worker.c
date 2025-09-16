#include "esp_flasher_worker.h"
#include "flasher_constants.h"

#ifndef RTL_ONLY_BUILD
#include "esp_loader.h"
#endif

#ifdef RTL_FLASHER_SUPPORT
#include "rtl/rtl_loader.h"
#endif

#include <string.h>

FuriStreamBuffer* flash_rx_stream; // TODO make safe
EspFlasherApp* global_app; // TODO make safe
FuriTimer* timer; // TODO make

static uint32_t _remaining_time = 0;
static void _timer_callback(void* context) {
    UNUSED(context);
    if(_remaining_time > 0) {
        _remaining_time--;
    }
}

static esp_loader_error_t _flash_file(EspFlasherApp* app, char* filepath, uint32_t addr) {
    // TODO cleanup
    esp_loader_error_t err;
    static uint8_t payload[1024];
    File* bin_file = storage_file_alloc(app->storage);

    char user_msg[256];

    // open file
    if(!storage_file_open(bin_file, filepath, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_close(bin_file);
        storage_file_free(bin_file);
        dialog_message_show_storage_error(app->dialogs, "Cannot open file");
        return ESP_LOADER_ERROR_FAIL;
    }

    uint64_t size = storage_file_size(bin_file);

    loader_port_debug_print("Erasing flash...this may take a while\n");
    err = esp_loader_flash_start(addr, size, sizeof(payload));
    if(err != ESP_LOADER_SUCCESS) {
        storage_file_close(bin_file);
        storage_file_free(bin_file);
        snprintf(user_msg, sizeof(user_msg), "Erasing flash failed with error %d\n", err);
        loader_port_debug_print(user_msg);
        return err;
    }

    loader_port_debug_print("Start programming\n");
    uint64_t last_updated = size;
    while(size > 0) {
        if((last_updated - size) > 50000) {
            // inform user every 50k bytes
            // TODO: draw a progress bar next update
            snprintf(user_msg, sizeof(user_msg), "%llu bytes left.\n", size);
            loader_port_debug_print(user_msg);
            last_updated = size;
        }
        size_t to_read = MIN(size, sizeof(payload));
        uint16_t num_bytes = storage_file_read(bin_file, payload, to_read);
        err = esp_loader_flash_write(payload, num_bytes);
        if(err != ESP_LOADER_SUCCESS) {
            snprintf(user_msg, sizeof(user_msg), "Packet could not be written! Error: %u\n", err);
            storage_file_close(bin_file);
            storage_file_free(bin_file);
            loader_port_debug_print(user_msg);
            return err;
        }

        size -= num_bytes;
    }

    loader_port_debug_print("Finished programming\n");

    // TODO verify

    storage_file_close(bin_file);
    storage_file_free(bin_file);

    return ESP_LOADER_SUCCESS;
}

#ifdef RTL_FLASHER_SUPPORT
static rtl_loader_error_t _flash_rtl_file(EspFlasherApp* app, char* filepath) {
    rtl_loader_error_t err;
    static uint8_t payload[1024];
    File* bin_file = storage_file_alloc(app->storage);
    char user_msg[256];

    // open file
    if(!storage_file_open(bin_file, filepath, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_close(bin_file);
        storage_file_free(bin_file);
        dialog_message_show_storage_error(app->dialogs, "Cannot open RTL file");
        return RTL_LOADER_ERROR_FAIL;
    }

    uint64_t size = storage_file_size(bin_file);

    rtl_loader_port_debug_print("Starting RTL flash operation\n");
    err = rtl_loader_flash_begin(0x0, size);  // RTL typically flashes to offset 0
    if(err != RTL_LOADER_SUCCESS) {
        storage_file_close(bin_file);
        storage_file_free(bin_file);
        snprintf(user_msg, sizeof(user_msg), "RTL flash begin failed with error %d\n", err);
        rtl_loader_port_debug_print(user_msg);
        return err;
    }

    rtl_loader_port_debug_print("Programming RTL chip\n");
    uint64_t last_updated = size;
    while(size > 0) {
        if((last_updated - size) > 50000) {
            snprintf(user_msg, sizeof(user_msg), "%llu bytes left.\n", size);
            rtl_loader_port_debug_print(user_msg);
            last_updated = size;
        }
        size_t to_read = MIN(size, sizeof(payload));
        uint16_t num_bytes = storage_file_read(bin_file, payload, to_read);
        err = rtl_loader_flash_write(payload, num_bytes);
        if(err != RTL_LOADER_SUCCESS) {
            snprintf(user_msg, sizeof(user_msg), "RTL flash write failed! Error: %u\n", err);
            storage_file_close(bin_file);
            storage_file_free(bin_file);
            rtl_loader_port_debug_print(user_msg);
            return err;
        }

        size -= num_bytes;
    }

    err = rtl_loader_flash_finish();
    if(err != RTL_LOADER_SUCCESS) {
        snprintf(user_msg, sizeof(user_msg), "RTL flash finish failed with error %d\n", err);
        rtl_loader_port_debug_print(user_msg);
    } else {
        rtl_loader_port_debug_print("RTL programming completed successfully\n");
    }

    storage_file_close(bin_file);
    storage_file_free(bin_file);

    return err;
}
#endif

// ... ESP-specific functions from original (omitted for brevity) ...

typedef struct {
    SelectedFlashOptions selected;
    const char* description;
    char* path;
    uint32_t addr;
} FlashItem;

static void _flash_all_files(EspFlasherApp* app) {
    if(app->chip_type == ChipTypeRTL) {
#ifdef RTL_FLASHER_SUPPORT
        if(app->selected_flash_options[SelectedFlashRTLBin]) {
            rtl_loader_port_debug_print("Flashing RTL binary\n");
            _flash_rtl_file(app, app->bin_file_path_rtl);
        }
#endif
        return;
    }

    // Original ESP flashing logic
    esp_loader_error_t err;
    const int num_steps = app->num_selected_flash_options;

#define NUM_FLASH_ITEMS 7
    FlashItem items[NUM_FLASH_ITEMS] = {
        {SelectedFlashBoot,
         "bootloader",
         app->bin_file_path_boot,
         app->selected_flash_options[SelectedFlashC5Mode] ?
             ESP_ADDR_BOOT_C5 :
             (app->selected_flash_options[SelectedFlashS3Mode] ? ESP_ADDR_BOOT_S3 :
                                                                 ESP_ADDR_BOOT)},
        {SelectedFlashPart, "partition table", app->bin_file_path_part, ESP_ADDR_PART},
        {SelectedFlashNvs, "NVS", app->bin_file_path_nvs, ESP_ADDR_NVS},
        {SelectedFlashBootApp0, "boot_app0", app->bin_file_path_boot_app0, ESP_ADDR_BOOT_APP0},
        {SelectedFlashAppA, "firmware A", app->bin_file_path_app_a, ESP_ADDR_APP_A},
        {SelectedFlashAppB, "firmware B", app->bin_file_path_app_b, ESP_ADDR_APP_B},
        {SelectedFlashCustom, "custom data", app->bin_file_path_custom, 0x0},
        /* if you add more entries, update NUM_FLASH_ITEMS above! */
    };

    char user_msg[256];

    int current_step = 1;
    for(FlashItem* item = &items[0]; item < &items[NUM_FLASH_ITEMS]; ++item) {
        if(app->selected_flash_options[item->selected]) {
            snprintf(
                user_msg,
                sizeof(user_msg),
                "Flashing %s (%d/%d) to address 0x%lx\n",
                item->description,
                current_step++,
                num_steps,
                item->addr);
            loader_port_debug_print(user_msg);
            err = _flash_file(app, item->path, item->addr);
            if(err) {
                break;
            }
        }
    }
}

static int32_t esp_flasher_flash_bin(void* context) {
    EspFlasherApp* app = (void*)context;
    esp_loader_error_t err = ESP_LOADER_SUCCESS;

    app->flash_worker_busy = true;

    // alloc global objects
    flash_rx_stream = furi_stream_buffer_alloc(RX_BUF_SIZE, 1);
    timer = furi_timer_alloc(_timer_callback, FuriTimerTypePeriodic, app);

    // turn on flipper blue LED for duration of flash
    notification_message(app->notification, &sequence_set_only_blue_255);

    if(app->chip_type == ChipTypeRTL) {
#ifdef RTL_FLASHER_SUPPORT
        rtl_loader_port_debug_print("Connecting to RTL chip\n");
        rtl_loader_error_t rtl_err = rtl_loader_enter_download_mode();
        if(rtl_err == RTL_LOADER_SUCCESS) {
            rtl_err = rtl_loader_connect();
            if(rtl_err == RTL_LOADER_SUCCESS) {
                rtl_loader_port_debug_print("RTL chip connected successfully\n");
                uint32_t start_time = furi_get_tick();
                _flash_all_files(app);
                FuriString* flash_time = furi_string_alloc_printf(
                    "RTL flash took: %lds\n", (furi_get_tick() - start_time) / 1000);
                rtl_loader_port_debug_print(furi_string_get_cstr(flash_time));
                furi_string_free(flash_time);
                rtl_loader_reset_chip();
            } else {
                rtl_loader_port_debug_print("Failed to connect to RTL chip\n");
            }
        } else {
            rtl_loader_port_debug_print("Failed to enter RTL download mode\n");
        }
#endif
    } else {
        // Original ESP connection and flashing logic
        loader_port_debug_print("Connecting to ESP chip\n");
        esp_loader_connect_args_t connect_config = ESP_LOADER_CONNECT_DEFAULT();
        err = esp_loader_connect(&connect_config);
        if(err != ESP_LOADER_SUCCESS) {
            char err_msg[256];
            snprintf(
                err_msg,
                sizeof(err_msg),
                "Cannot connect to target. Error: %u\nMake sure the device is in bootloader/reflash mode, then try again.\n",
                err);
            loader_port_debug_print(err_msg);
        }

        // higher BR
        if(!err && app->turbospeed) {
            loader_port_debug_print("Increasing speed for faster flash\n");
            err = esp_loader_change_transmission_rate(FAST_BAUDRATE);
            if(err != ESP_LOADER_SUCCESS) {
                char err_msg[256];
                snprintf(
                    err_msg, sizeof(err_msg), "Cannot change transmission rate. Error: %u\n", err);
                loader_port_debug_print(err_msg);
            }
            esp_flasher_uart_set_br(app->uart, FAST_BAUDRATE);
        }

        if(!err) {
            loader_port_debug_print("ESP chip connected\n");
            uint32_t start_time = furi_get_tick();

            _flash_all_files(app);

            FuriString* flash_time = furi_string_alloc_printf(
                "ESP flash took: %lds\n", (furi_get_tick() - start_time) / 1000);
            loader_port_debug_print(furi_string_get_cstr(flash_time));
            furi_string_free(flash_time);

            if(app->turbospeed) {
                loader_port_debug_print("Restoring transmission rate\n");
                esp_flasher_uart_set_br(app->uart, BAUDRATE);
            }

            loader_port_reset_target();
        }
    }

    loader_port_debug_print(
        "Done flashing. Please reset the board manually if it doesn't auto-reset.\n");

    // short buzz to alert user
    notification_message(app->notification, &sequence_set_vibro_on);
    loader_port_delay_ms(50);
    notification_message(app->notification, &sequence_reset_vibro);

    // turn off flipper blue LED
    notification_message(app->notification, &sequence_reset_blue);

    // done
    app->flash_worker_busy = false;

    // cleanup
    furi_stream_buffer_free(flash_rx_stream);
    flash_rx_stream = NULL;
    furi_timer_free(timer);
    return 0;
}

// GPIO control functions for RTL download mode
static void _initDTR(void) {
    furi_hal_gpio_init(&gpio_ext_pc3, GpioModeOutputPushPull, GpioPullDown, GpioSpeedVeryHigh);
}

static void _initRTS(void) {
    furi_hal_gpio_init(&gpio_ext_pb2, GpioModeOutputPushPull, GpioPullDown, GpioSpeedVeryHigh);
}

static void _setDTR(bool state) {
    furi_hal_gpio_write(&gpio_ext_pc3, state);
}

static void _setRTS(bool state) {
    furi_hal_gpio_write(&gpio_ext_pb2, state);
}

static void _initCEN(void) {
    // For RTL8710BN, we need to control CEN pin (pin for download mode)
    // Using different GPIO for RTL-specific control
    furi_hal_gpio_init(&gpio_ext_pa7, GpioModeOutputPushPull, GpioPullDown, GpioSpeedVeryHigh);
}

static void _setCEN(bool state) {
    furi_hal_gpio_write(&gpio_ext_pa7, state);
}

static int32_t esp_flasher_reset(void* context) {
    EspFlasherApp* app = (void*)context;

    app->flash_worker_busy = true;

    if(app->chip_type == ChipTypeRTL) {
        _initCEN();
        _setCEN(false);
        furi_delay_ms(100);
        _setCEN(true);
    } else {
        _setDTR(false);
        _initDTR();
        _setRTS(false);
        _initRTS();

        furi_hal_gpio_init_simple(&gpio_swclk, GpioModeOutputPushPull);
        furi_hal_gpio_write(&gpio_swclk, true);

        if(app->reset) {
            loader_port_debug_print("Resetting board\n");
            loader_port_reset_target();
        } else if(app->boot) {
            loader_port_debug_print("Entering bootloader\n");
            loader_port_enter_bootloader();
        }
    }

    // done
    app->flash_worker_busy = false;
    app->reset = false;
    app->boot = false;

    return 0;
}

void esp_flasher_worker_start_thread(EspFlasherApp* app) {
    global_app = app;

    app->flash_worker = furi_thread_alloc();
    furi_thread_set_name(app->flash_worker, "EspFlasherFlashWorker");
    furi_thread_set_stack_size(app->flash_worker, 2048);
    furi_thread_set_context(app->flash_worker, app);
    if(app->reset || app->boot) {
        furi_thread_set_callback(app->flash_worker, esp_flasher_reset);
    } else {
        furi_thread_set_callback(app->flash_worker, esp_flasher_flash_bin);
    }
    furi_thread_start(app->flash_worker);
}

void esp_flasher_worker_stop_thread(EspFlasherApp* app) {
    furi_thread_join(app->flash_worker);
    furi_thread_free(app->flash_worker);
}

// ESP loader port functions
esp_loader_error_t loader_port_read(uint8_t* data, uint16_t size, uint32_t timeout) {
    size_t read = furi_stream_buffer_receive(flash_rx_stream, data, size, timeout);
    if(read < size) {
        return ESP_LOADER_ERROR_TIMEOUT;
    } else {
        return ESP_LOADER_SUCCESS;
    }
}

esp_loader_error_t loader_port_write(const uint8_t* data, uint16_t size, uint32_t timeout) {
    UNUSED(timeout);
    if(global_app) esp_flasher_uart_tx(global_app->uart, (uint8_t*)data, size);
    return ESP_LOADER_SUCCESS;
}

void loader_port_reset_target(void) {
    _setDTR(true);
    loader_port_delay_ms(SERIAL_FLASHER_RESET_HOLD_TIME_MS);
    _setDTR(false);
}

void loader_port_enter_bootloader(void) {
    // Also support for the Multi-fucc and Xeon boards
    furi_hal_gpio_write(&gpio_swclk, false);
    furi_hal_power_disable_otg();
    loader_port_delay_ms(100);
    furi_hal_power_enable_otg();
    furi_hal_gpio_init_simple(&gpio_swclk, GpioModeAnalog);
    loader_port_delay_ms(100);

    // adapted from custom usb-jtag-serial reset in esptool
    _setDTR(true);
    loader_port_delay_ms(SERIAL_FLASHER_RESET_HOLD_TIME_MS);
    _setRTS(true);
    _setDTR(false);
    loader_port_delay_ms(SERIAL_FLASHER_BOOT_HOLD_TIME_MS);
    _setRTS(false);
}

void loader_port_delay_ms(uint32_t ms) {
    furi_delay_ms(ms);
}

void loader_port_start_timer(uint32_t ms) {
    _remaining_time = ms;
    furi_timer_start(timer, 1);
}

uint32_t loader_port_remaining_time(void) {
    return _remaining_time;
}

extern void esp_flasher_console_output_handle_rx_data_cb(
    uint8_t* buf,
    size_t len,
    void* context); // TODO cleanup
void loader_port_debug_print(const char* str) {
    if(global_app)
        esp_flasher_console_output_handle_rx_data_cb((uint8_t*)str, strlen(str), global_app);
}

void loader_port_spi_set_cs(uint32_t level) {
    UNUSED(level);
    // unimplemented
}

// RTL loader port functions
#ifdef RTL_FLASHER_SUPPORT
rtl_loader_error_t rtl_loader_port_write(const uint8_t* data, uint16_t size, uint32_t timeout) {
    UNUSED(timeout);
    if(global_app) esp_flasher_uart_tx(global_app->uart, (uint8_t*)data, size);
    return RTL_LOADER_SUCCESS;
}

rtl_loader_error_t rtl_loader_port_read(uint8_t* data, uint16_t size, uint32_t timeout) {
    size_t read = furi_stream_buffer_receive(flash_rx_stream, data, size, timeout);
    if(read < size) {
        return RTL_LOADER_ERROR_TIMEOUT;
    } else {
        return RTL_LOADER_SUCCESS;
    }
}

void rtl_loader_port_delay_ms(uint32_t ms) {
    furi_delay_ms(ms);
}

void rtl_loader_port_enter_download_mode(void) {
    // RTL8710BN download mode sequence: CEN low, TX2 low, then CEN high
    _initCEN();
    _initRTS(); // Use RTS as TX2 control for RTL
    
    _setCEN(false);
    _setRTS(false); // TX2 low
    rtl_loader_port_delay_ms(RTL_DOWNLOAD_MODE_HOLD_MS);
    _setCEN(true);
    rtl_loader_port_delay_ms(50);
    _setRTS(true); // TX2 high
}

void rtl_loader_port_reset_target(void) {
    _initCEN();
    _setCEN(false);
    rtl_loader_port_delay_ms(RTL_RESET_HOLD_MS);
    _setCEN(true);
}

void rtl_loader_port_debug_print(const char* str) {
    if(global_app)
        esp_flasher_console_output_handle_rx_data_cb((uint8_t*)str, strlen(str), global_app);
}
#endif

void esp_flasher_worker_handle_rx_data_cb(uint8_t* buf, size_t len, void* context) {
    UNUSED(context);
    if(flash_rx_stream) {
        furi_stream_buffer_send(flash_rx_stream, buf, len, 0);
    } else {
        // done flashing
        if(global_app) esp_flasher_console_output_handle_rx_data_cb(buf, len, global_app);
    }
}