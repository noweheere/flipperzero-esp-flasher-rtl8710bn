# Flipper Zero ESP/RTL Flasher

Extended Flipper Zero ESP Flasher with comprehensive RTL8710BN/WR3 support for Tuya IoT modules.

## Features

- **RTL8710BN Support**: Full support for Realtek RTL8710BN WiFi chips commonly found in Tuya devices
- **ESP32/ESP8266 Support**: Compatible with original ESP flasher functionality  
- **Automatic Chip Detection**: Automatically detects chip type (RTL8710BN ID: 0x8710)
- **Download Mode Automation**: Handles boot sequence automatically
- **Progress Indicators**: Real-time flash progress display
- **Error Handling**: Comprehensive error reporting and troubleshooting guidance

## RTL8710BN/WR3 Specifications

- **Chip ID**: 0x8710
- **Flash Size**: 2MB (0x200000)
- **Sector Size**: 4KB (0x1000)
- **Download Baudrate**: 115200 bps
- **Flash Baudrate**: 460800 bps

## Wiring Instructions

### RTL8710BN/WR3 Modules

```
Flipper Zero -> RTL Module
5V          -> VCC (requires external 3.3V!)
GND         -> GND  
A7          -> TX
A6          -> RX
C0          -> CEN (Reset)
C1          -> TX2 (Boot)
```

⚠️ **IMPORTANT**: RTL8710BN modules require external 3.3V power supply. The Flipper Zero's 3.3V output is insufficient for reliable operation.

### Download Mode Sequence

The app automatically handles the RTL8710BN download mode sequence:
1. CEN + TX2 pulled to GND
2. Release CEN while keeping TX2 low  
3. Release TX2
4. Begin communication

## Usage

1. **Select Chip Type**: Choose "Flash RTL8710BN" from the main menu
2. **Select Firmware**: Browse and select a .bin firmware file from SD card
3. **Check Wiring**: Review the wiring diagram and connections
4. **Flash**: Connect external power and start flashing process

## Firmware Files

Place `.bin` firmware files in `/ext/esp_flasher/` on your SD card. The app supports:
- LibreTiny compiled firmware
- ESPHome RTL8710BN builds
- Custom RTL8710BN firmware binaries

## Error Troubleshooting

Common issues and solutions:

- **Chip not detected**: Check wiring and external power supply
- **Sync failed**: Ensure proper download mode sequence and stable power
- **Flash failed**: Verify firmware file compatibility and size (max 2MB)
- **Connection timeout**: Check UART wiring (A6/A7 pins)

## Technical Details

### Flash Process
1. Initialize UART communication (115200 bps)
2. Enter download mode via GPIO sequence
3. Sync with RTL8710BN bootloader
4. Detect chip ID (0x8710)
5. Switch to high-speed mode (460800 bps)
6. Erase and flash firmware in 1KB blocks
7. Verify and reboot

### Compatibility
- LibreTiny framework
- ESPHome RTL platform  
- Arduino RTL8710BN cores
- Raw binary images

## Development

Based on the original ESP Flasher by 0xchocolate, extended with comprehensive RTL8710BN support following Realtek's download protocol specifications.

### Build Requirements
- Flipper Zero firmware SDK
- Standard Flipper build environment

### File Structure
```
esp_flasher.c/.h           - Main application
esp_flasher_rtl87xx.c/.h   - RTL8710BN chip support
scenes/                    - UI scenes and navigation
application.fam            - App configuration
```
