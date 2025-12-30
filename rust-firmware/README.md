# BMO Firmware - Rust/Modbus Implementation

This directory contains the Rust-based, Modbus-enabled firmware for the BMO e-paper display system.

## Architecture

See `../DOCS/ARCHITECTURE.md` for detailed system architecture and design decisions.

## Project Structure

```
rust-firmware/
├── Cargo.toml              # Rust package manifest
├── build.rs                # Build script for ESP-IDF
├── src/
│   ├── main.rs             # Main application entry point
│   ├── config.rs           # System configuration
│   ├── error.rs            # Error types and handling
│   ├── display.rs          # Display hardware abstraction
│   ├── modbus.rs           # Modbus RTU protocol implementation
│   └── face_manager.rs     # Face asset management
└── README.md               # This file
```

## Prerequisites

### Install Rust
```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

### Install ESP-IDF Toolchain
```bash
cargo install espup
espup install
. $HOME/export-esp.sh
```

### Add ESP32-C3 Target
```bash
rustup target add riscv32imc-unknown-none-elf
```

## Building

```bash
cd rust-firmware
cargo build --release
```

## Flashing

```bash
cargo espflash flash --release --monitor
```

Or with specific port:
```bash
cargo espflash flash --release --port COM7 --monitor
```

## Modbus Testing

Use a Modbus master tool or Python script to test:

```python
from pymodbus.client import ModbusSerialClient

client = ModbusSerialClient(
    port='COM7',
    baudrate=9600,
    parity='N',
    stopbits=1,
    bytesize=8,
    timeout=1
)

# Connect
client.connect()

# Read holding registers (status)
result = client.read_holding_registers(address=0, count=10, unit=1)
print(f"Registers: {result.registers}")

# Write single register (change face)
client.write_register(address=0, value=4, unit=1)  # Face ID 4 (surprised)

# Close
client.close()
```

## Register Map

See `../DOCS/ARCHITECTURE.md` for complete Modbus register map.

### Quick Reference

| Register | Name    | R/W | Description              |
|----------|---------|-----|--------------------------|
| 0x0000   | FACE_ID | RW  | Current face ID (0-15)   |
| 0x0001   | ANI_MODE| RW  | Animation mode           |
| 0x0002   | INTERVAL| RW  | Refresh interval (ms)    |

## Development

### Enable Debug Logging
Set environment variable before building:
```bash
export ESP_LOG_LEVEL=debug
cargo build --release
```

### Code Formatting
```bash
cargo fmt
```

### Linting
```bash
cargo clippy
```

## Features

- ✅ Modbus RTU protocol over UART
- ✅ Register-based face control
- ✅ Memory-safe Rust implementation
- ✅ Hardware abstraction layer
- ✅ Custom LUTs for optimal refresh
- ⏳ File system integration (SPIFFS)
- ⏳ Animation state machine
- ⏳ Modbus TCP over WiFi
- ⏳ MQTT bridge

## Troubleshooting

### Build Errors
- Ensure ESP-IDF environment is sourced: `. $HOME/export-esp.sh`
- Check Rust version: `rustc --version` (should be 1.70+)

### Flash Errors
- Check USB cable and port permissions
- Try holding BOOT button while connecting

### Runtime Errors
- Monitor serial output: `cargo espflash monitor --port COM7`
- Check Modbus slave ID matches (default: 1)
- Verify baudrate (default: 9600)

## License

See LICENSE file in repository root.

## Contributors

- BMO Team

## References

- [ESP-IDF HAL Documentation](https://docs.esp-rs.org/esp-idf-hal/)
- [Modbus Protocol Specification](https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf)
- [Embedded Rust Book](https://docs.rust-embedded.org/book/)
