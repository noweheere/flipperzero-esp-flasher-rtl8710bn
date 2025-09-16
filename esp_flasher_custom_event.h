#pragma once

typedef enum EspFlasherCustomEvent {
    // Reserve first 100 events for button types and indexes, starting from 0
    EspFlasherCustomEventReserved = 100,

    EspFlasherCustomEventTextEditResult,
    EspFlasherCustomEventByteInputDone,
    EspFlasherCustomEventVarItemListIndexChanged,
} EspFlasherCustomEvent;