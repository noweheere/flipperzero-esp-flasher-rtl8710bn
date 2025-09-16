#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/text_input.h>
#include <gui/modules/popup.h>
#include <gui/modules/loading.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>
#include <notification/notification_messages.h>
#include <expansion/expansion.h>
#include <toolbox/path.h>

#define ESP_FLASHER_APP_FOLDER "/ext/esp_flasher"
#define ESP_FLASHER_MAX_FILE_NAME_LEN 255

typedef enum {
    EspFlasherChipTypeESP32,
    EspFlasherChipTypeESP8266,
    EspFlasherChipTypeRTL8710BN,
    EspFlasherChipTypeUnknown,
} EspFlasherChipType;

typedef enum {
    EspFlasherViewSubmenu,
    EspFlasherViewDialogEx,
    EspFlasherViewTextInput,
    EspFlasherViewPopup,
    EspFlasherViewLoading,
    EspFlasherViewWiring,
} EspFlasherView;

typedef enum {
    EspFlasherSceneStart,
    EspFlasherSceneSelectFirmware,
    EspFlasherSceneWiring,
    EspFlasherSceneFlashing,
    EspFlasherSceneSuccess,
    EspFlasherSceneError,
    EspFlasherSceneCount,
} EspFlasherScene;

typedef enum {
    EspFlasherCustomEventSelectESP32,
    EspFlasherCustomEventSelectESP8266,
    EspFlasherCustomEventSelectRTL8710BN,
    EspFlasherCustomEventSelectFirmware,
    EspFlasherCustomEventStartFlashing,
    EspFlasherCustomEventFlashingSuccess,
    EspFlasherCustomEventFlashingError,
    EspFlasherCustomEventBack,
} EspFlasherCustomEvent;

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    Submenu* submenu;
    DialogEx* dialog_ex;
    TextInput* text_input;
    Popup* popup;
    Loading* loading;
    View* wiring_view;
    
    NotificationApp* notifications;
    DialogsApp* dialogs;
    Storage* storage;
    Expansion* expansion;
    
    EspFlasherChipType selected_chip;
    FuriString* firmware_path;
    char firmware_name[ESP_FLASHER_MAX_FILE_NAME_LEN];
    
    FuriThread* worker_thread;
    bool worker_running;
} EspFlasherApp;

typedef struct {
    EspFlasherApp* app;
    bool success;
    FuriString* error_message;
} EspFlasherWorkerResult;