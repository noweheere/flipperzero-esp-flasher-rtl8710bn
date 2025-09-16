## ESP/RTL Flasher App Flow Demonstration

### Main Menu (Start Scene)
```
┌─ ESP/RTL Flasher ────────┐
│                          │
│ > Flash ESP32            │
│   Flash ESP8266          │
│   Flash RTL8710BN        │
│                          │
│ [Back] Select a chip type│
└──────────────────────────┘
```

### RTL8710BN Selected → Firmware Selection
```
┌─ Select Firmware ────────┐
│                          │
│ Browse SD card for       │
│ .bin firmware files      │
│                          │
│ /ext/esp_flasher/        │
│ > tuya_firmware.bin      │
│   libretiny_build.bin    │
│   custom_rtl.bin         │
│                          │
│ [Back] [Select]          │
└──────────────────────────┘
```

### Wiring Instructions  
```
┌─ RTL8710BN ──────────────┐
│                          │
│ RTL8710BN Wiring:        │
│                          │
│ Flipper -> RTL Module    │
│ 5V -> VCC (3.3V ext!)    │
│ GND -> GND               │
│ A7 -> TX                 │
│ A6 -> RX                 │
│ C0 -> CEN (Reset)        │
│ C1 -> TX2 (Boot)         │
│                          │
│ IMPORTANT: RTL8710BN     │
│ requires external 3.3V   │
│ power supply!            │
│                          │
│ [Back]           [Flash] │
└──────────────────────────┘
```

### Flashing Progress
```
┌─ Flashing RTL8710BN ─────┐
│                          │
│ ●●●●●●●●○○ 80%           │
│                          │
│ Syncing with chip...     │
│ Detected: RTL8710BN      │
│ Flashing firmware...     │
│                          │
│ 1024/1280 KB written     │
│                          │
│ Please wait...           │
└──────────────────────────┘
```

### Success
```
┌─ Success! ───────────────┐
│                          │
│ RTL8710BN flashed        │
│ successfully!            │
│                          │
│ Firmware: tuya_fw.bin    │
│                          │
│ The chip should now boot │
│ with the new firmware.   │
│                          │
│ You can disconnect the   │
│ Flipper Zero now.        │
│                          │
│ [Back]      [Flash Again]│
└──────────────────────────┘
```

### Error Handling
```
┌─ Error ──────────────────┐
│                          │
│ Flash operation failed!  │
│                          │
│ Error: Chip not detected │
│                          │
│ Please check:            │
│ • Wiring connections     │
│ • Power supply (3.3V)    │
│ • Firmware file validity │
│ • Chip compatibility     │
│                          │
│ [Back]             [Retry]│
└──────────────────────────┘
```