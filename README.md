# flipperzero-esp-flasher-rtl8710bn

Extended Flipper Zero ESP Flasher with RTL8710BN/WR3 support for Tuya IoT modules.

This project extends the popular ESP Flasher for Flipper Zero to support RTL8710BN (Realtek) chips commonly found in Tuya IoT devices and WR3 modules.

## Features

- **Dual Chip Support**: Flash both ESP32 and RTL8710BN chips
- **RTL8710BN Detection**: Automatic chip ID detection (0x8710)
- **Download Mode**: Automated CEN+TX2 download sequence for RTL8710BN
- **LibreTiny Compatible**: Works with LibreTiny compiled .bin files
- **Wiring Diagrams**: Built-in documentation and pin assignments
- **External Power Support**: Safe 3.3V external power handling

## Supported Chips

### ESP32 Family (Original Support)
- ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C2, ESP32-H2

### RTL8710BN (New Support)  
- RTL8710BN/WR3 (Tuya modules)
- 2MB flash, UART PA29/PA30
- Requires external 3.3V power

## Quick Start

1. **Installation**: Copy `esp_flasher_rtl.fap` to your Flipper Zero SD card under `Apps/GPIO/`
2. **Wiring**: Connect your module according to the built-in wiring guide
3. **Power**: Ensure external 3.3V power for RTL8710BN modules
4. **Flash**: Select chip type and follow on-screen instructions

## RTL8710BN Wiring

```
RTL8710BN → Flipper Zero
VCC → External 3.3V PSU
GND → Pin 8 (GND)
TX(PA29) → Pin 13 (RX) 
RX(PA30) → Pin 14 (TX)
CEN → Pin 15 (Control)
```

⚠️ **Important**: RTL8710BN requires external 3.3V power supply

## Usage

### For ESP32 Chips
1. Select "Flash ESP chip"
2. Choose bootloader, partition table, and firmware files
3. Configure flash options
4. Press "[>] FLASH"

### For RTL8710BN Chips  
1. Select "Flash RTL8710BN"
2. Choose your LibreTiny compiled .bin file
3. Enable "RTL Binary" option
4. Press "[>] FLASH"

## LibreTiny Integration

This flasher works seamlessly with [LibreTiny](https://github.com/libretiny-eu/libretiny), an Arduino-like environment for RTL8710BN chips. Simply compile your project and flash the resulting .bin file.

## Technical Details

- **RTL Chip ID**: 0x8710
- **Flash Size**: 2MB maximum
- **Protocol**: Custom RTL download protocol with SLIP-like framing
- **Download Mode**: CEN+TX2 pin sequence timing
- **Baud Rate**: 115200 (RTL), 115200-921600 (ESP32)

## Building

This project requires the Flipper Zero firmware development environment:

### Prerequisites
- Install ufbt (Flipper Zero build tool):
  ```bash
  pip install ufbt
  ```

### Build Steps
1. Clone the repository:
   ```bash
   git clone https://github.com/noweheere/flipperzero-esp-flasher-rtl8710bn.git
   cd flipperzero-esp-flasher-rtl8710bn
   ```

2. Verify all components (optional):
   ```bash
   ./verify_app.sh
   ```

3. Build the application:
   ```bash
   ufbt
   ```

4. Install to Flipper Zero:
   ```bash
   ufbt launch
   ```

### Manual Installation
Copy the generated `dist/esp_flasher_rtl.fap` file to your Flipper Zero SD card:
```
/ext/apps/GPIO/esp_flasher_rtl.fap
```

## Usage

1. **Launch**: Apps → GPIO → ESP/RTL Flasher
2. **Select Chip Type**: Choose ESP32 family or RTL8710BN
3. **Wiring**: Follow the built-in wiring diagrams
4. **Select Files**: Browse and choose your .bin firmware files
5. **Flash**: Start the flashing process

### File Requirements
- **ESP32**: Use .bin files (bootloader, partition table, application)
- **RTL8710BN**: Use LibreTiny-compatible .bin files

### Power Requirements
- **ESP32**: Can use Flipper's 3.3V or external supply
- **RTL8710BN**: **Requires external 3.3V power supply**

## Documentation

- [RTL8710BN Support Details](docs/RTL8710BN_SUPPORT.md)
- [Wiring Diagrams](docs/WIRING.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)

## Credits

- Based on [0xchocolate/flipperzero-esp-flasher](https://github.com/0xchocolate/flipperzero-esp-flasher)
- RTL8710BN support developed for the community
- LibreTiny integration for modern RTL development

## License

GPL v3 - Same as original ESP Flasher project

## Contributing

Contributions welcome! Please submit PRs for:
- Additional RTL chip variants
- Protocol improvements  
- Bug fixes and enhancements
- Documentation updates
