# RTL8710BN Support

This flasher now supports RTL8710BN/WR3 Tuya modules in addition to ESP32 chips.

## RTL8710BN Specifications

- **Chip ID**: 0x8710
- **Flash Size**: 2MB
- **UART Pins**: TX (PA29), RX (PA30)
- **Power**: 3.3V external power required
- **Download Mode**: CEN+TX2 sequence

## Wiring Diagram for RTL8710BN

```
RTL8710BN Module    Flipper Zero
================    ============
VCC (3.3V)    ----> External 3.3V Power Supply
GND           ----> Pin 8 (GND)
TX (PA29)     ----> Pin 13 (RX)
RX (PA30)     ----> Pin 14 (TX)  
CEN           ----> Pin 15 (Control for download mode)
TX2           ----> Pin 16 (Control for download sequence)
```

**Important**: RTL8710BN requires external 3.3V power supply. Do not power from Flipper Zero.

## Download Mode Activation

The RTL8710BN enters download mode using a specific sequence:

1. Set CEN pin LOW
2. Set TX2 pin LOW  
3. Hold for 100ms
4. Set CEN pin HIGH
5. Wait 50ms
6. Set TX2 pin HIGH

This sequence is automatically handled by the flasher software.

## Supported File Format

- **.bin files**: Raw binary firmware compiled with LibreTiny or similar RTL8710BN toolchain

## Usage

1. Connect RTL8710BN module according to wiring diagram
2. Ensure external 3.3V power is connected
3. Select "Flash RTL8710BN" from main menu
4. Choose your .bin firmware file
5. Select "RTL Binary" option and set to "Yes"
6. Press "[>] FLASH" to start flashing

## LibreTiny Integration

This flasher is designed to work with firmware compiled using LibreTiny, which provides Arduino-like environment for RTL8710BN chips. The flasher expects standard .bin format files as output by the LibreTiny build process.

## Troubleshooting

### Connection Issues
- Verify external 3.3V power is connected and stable
- Check all wiring connections
- Ensure CEN pin is properly connected for download mode control

### Flash Failures  
- Try power cycling the RTL8710BN module
- Verify .bin file is valid LibreTiny compiled firmware
- Check that firmware size doesn't exceed 2MB limit

### Download Mode Issues
- Verify CEN and TX2 control pins are connected
- Try manual reset: briefly ground CEN pin while powered
- Check for proper timing in download sequence