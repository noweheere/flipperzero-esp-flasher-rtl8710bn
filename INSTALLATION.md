# ESP/RTL Flasher - Installation Guide

## Overview
This is the ESP/RTL Flasher application for Flipper Zero with support for both ESP32 family chips and RTL8710BN/WR3 modules commonly found in Tuya IoT devices.

## Installation

### Method 1: Build from Source (Recommended)
1. Install ufbt (uFBT - Flipper Zero build tool):
   ```bash
   pip install ufbt
   ```

2. Clone this repository:
   ```bash
   git clone https://github.com/noweheere/flipperzero-esp-flasher-rtl8710bn.git
   cd flipperzero-esp-flasher-rtl8710bn
   ```

3. Build the application:
   ```bash
   ufbt
   ```

4. The compiled `esp_flasher_rtl.fap` file will be in the `dist/` directory.

5. Copy the .fap file to your Flipper Zero SD card:
   ```
   /ext/apps/GPIO/esp_flasher_rtl.fap
   ```

### Method 2: Pre-built Release
1. Download the latest `esp_flasher_rtl.fap` from the Releases page
2. Copy to your Flipper Zero SD card under `/ext/apps/GPIO/`

## Usage

1. **Launch the app**: Go to Apps > GPIO > ESP/RTL Flasher
2. **Select chip type**: Choose between ESP32 family or RTL8710BN
3. **Wiring**: Follow the built-in wiring guide
4. **Power**: Ensure external 3.3V power for RTL8710BN modules
5. **Browse and select**: Choose your firmware files (.bin format)
6. **Flash**: Start the flashing process

## Wiring Diagrams

### ESP32 Family
```
ESP32 → Flipper Zero
VCC → Pin 1 (3.3V) or external PSU
GND → Pin 8 (GND)
TX → Pin 13 (RX)
RX → Pin 14 (TX)
EN → Pin 15 (Control)
GPIO0 → Pin 16 (Boot)
```

### RTL8710BN/WR3 (Tuya modules)
```
RTL8710BN → Flipper Zero
VCC → External 3.3V PSU (REQUIRED)
GND → Pin 8 (GND)
TX(PA29) → Pin 13 (RX)
RX(PA30) → Pin 14 (TX)
CEN → Pin 15 (Control)
```

⚠️ **Important**: RTL8710BN modules require external 3.3V power supply

## Supported Firmware

### ESP32 Family
- Arduino IDE compiled .bin files
- ESP-IDF compiled .bin files
- Bootloader, partition table, and application binaries

### RTL8710BN
- LibreTiny compiled .bin files
- Arduino IDE with RTL8710BN support
- Direct binary images for RTL8710BN

## Features

- **Dual Chip Support**: Flash both ESP32 and RTL8710BN chips
- **Automatic Detection**: Chip ID detection and validation
- **Download Mode**: Automated boot sequence for reliable flashing
- **Progress Feedback**: Real-time flashing progress and status
- **Error Recovery**: Robust error handling and recovery
- **External Power Support**: Safe handling of external power requirements

## Troubleshooting

### Common Issues
1. **Connection Failed**: Check wiring and power supply
2. **Flash Timeout**: Ensure proper download mode activation
3. **File Not Found**: Use .bin files in the correct directory
4. **RTL Power Issues**: RTL8710BN requires external 3.3V supply

### RTL8710BN Specific
- Always use external 3.3V power (Flipper's 3.3V is insufficient)
- Ensure CEN pin connection for download mode
- Use LibreTiny-compatible firmware

## Development

### Building
```bash
ufbt
```

### Testing
```bash
ufbt launch
```

### Debugging
```bash
ufbt debug
```

## Credits

- Based on [0xchocolate/flipperzero-esp-flasher](https://github.com/0xchocolate/flipperzero-esp-flasher)
- RTL8710BN support developed for the community
- LibreTiny integration for modern RTL development

## License

GPL v3 - Same as original ESP Flasher project

## Contributing

Contributions welcome! Please submit PRs for:
- Additional chip variants
- Protocol improvements
- Bug fixes and enhancements
- Documentation updates