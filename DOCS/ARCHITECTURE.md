# BMO Display Firmware - Modbus Architecture

## System Overview

A modular, Modbus-enabled firmware system for controlling the BMO e-paper display faces using embedded Rust on ESP32-C3.

## Architecture Principles

### 1. Modular Design
- **Separation of Concerns**: Each module has a single, well-defined responsibility
- **Hardware Abstraction**: Display hardware details isolated from business logic
- **Protocol Agnostic Core**: Face rendering independent of communication protocol

### 2. Modbus Integration
- **Register-Based Control**: Face expressions mapped to Modbus holding registers
- **Real-time Updates**: Low-latency face changes via Modbus commands
- **Network Ready**: Multi-device deployment on industrial networks

### 3. Safety & Reliability
- **Rust Memory Safety**: No undefined behavior, no buffer overflows
- **Error Handling**: Comprehensive Result types throughout
- **Watchdog Protection**: System reset on hang conditions

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                        │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ Face Manager │  │ Animation    │  │ System       │     │
│  │              │  │ Controller   │  │ Monitor      │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
                            │
┌─────────────────────────────────────────────────────────────┐
│                   Communication Layer                        │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ Modbus RTU   │  │ Modbus TCP   │  │ MQTT Bridge  │     │
│  │ Handler      │  │ Handler      │  │ (optional)   │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
                            │
┌─────────────────────────────────────────────────────────────┐
│                   Business Logic Layer                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ Face         │  │ Expression   │  │ Asset        │     │
│  │ Renderer     │  │ State Machine│  │ Manager      │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
                            │
┌─────────────────────────────────────────────────────────────┐
│                Hardware Abstraction Layer                    │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ Display HAL  │  │ Storage HAL  │  │ Timer HAL    │     │
│  │ (E-Paper)    │  │ (SPIFFS)     │  │              │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
                            │
┌─────────────────────────────────────────────────────────────┐
│                      Driver Layer                            │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ SPI Driver   │  │ GPIO Driver  │  │ UART Driver  │     │
│  │ (bit-bang)   │  │              │  │ (Modbus)     │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

## Modbus Register Map

### Holding Registers (Read/Write)

| Address | Name                | Type   | Description                           |
|---------|---------------------|--------|---------------------------------------|
| 0x0000  | FACE_ID             | u16    | Current face expression ID (0-15)     |
| 0x0001  | ANIMATION_MODE      | u16    | 0=Static, 1=Cycle, 2=Random           |
| 0x0002  | REFRESH_INTERVAL    | u16    | Milliseconds between updates          |
| 0x0003  | DISPLAY_BRIGHTNESS  | u16    | Reserved (e-paper: 0=normal, 1=invert)|
| 0x0004  | SYSTEM_STATUS       | u16    | Bit flags: [ready, busy, error, ...]  |
| 0x0005  | ERROR_CODE          | u16    | Last error code (0=no error)          |
| 0x0006  | PARTIAL_REFRESH_CNT | u16    | Counter for partial refreshes         |
| 0x0007  | FULL_REFRESH_TRIG   | u16    | Write 1 to trigger full refresh       |
| 0x0008  | FACE_TRANSITION     | u16    | Transition effect (0=instant, 1=fade) |
| 0x0009  | WATCHDOG_TIMEOUT    | u16    | Watchdog timeout in seconds           |
| 0x000A-0x000F | RESERVED       | u16    | Future use                            |

### Input Registers (Read-Only)

| Address | Name                | Type   | Description                           |
|---------|---------------------|--------|---------------------------------------|
| 0x0000  | FIRMWARE_VERSION_HI | u16    | Firmware version major.minor          |
| 0x0001  | FIRMWARE_VERSION_LO | u16    | Firmware version patch                |
| 0x0002  | UPTIME_HI           | u16    | System uptime (high word)             |
| 0x0003  | UPTIME_LO           | u16    | System uptime (low word)              |
| 0x0004  | FREE_HEAP           | u16    | Free heap memory (KB)                 |
| 0x0005  | DISPLAY_WIDTH       | u16    | Display width in pixels               |
| 0x0006  | DISPLAY_HEIGHT      | u16    | Display height in pixels              |
| 0x0007  | TOTAL_FACES         | u16    | Number of available face assets       |
| 0x0008  | TEMPERATURE         | u16    | Display temperature (°C * 10)         |

### Coils (Read/Write Bits)

| Address | Name                | Description                              |
|---------|---------------------|------------------------------------------|
| 0x0000  | DISPLAY_ENABLE      | Enable/disable display updates           |
| 0x0001  | DEBUG_MODE          | Enable debug output                      |
| 0x0002  | FORCE_FULL_REFRESH  | Force next update to be full refresh     |
| 0x0003  | INVERT_DISPLAY      | Invert black/white                       |

## Face ID Mapping

| Face ID | Expression    | File                   | Description              |
|---------|---------------|------------------------|--------------------------|
| 0       | Neutral       | 0_neutral.bin          | Default resting face     |
| 1       | Happy         | 1_happy.bin            | Happy expression         |
| 2       | Sad           | 2_sad.bin              | Sad expression           |
| 3       | Angry         | 3_angry.bin            | Angry expression         |
| 4       | Surprised     | 4_surprised.bin        | Surprised expression     |
| 5       | Confused      | 5_confused.bin         | Confused expression      |
| 6       | Sleeping      | 6_sleeping.bin         | Sleep/idle state         |
| 7       | Excited       | 7_excited.bin          | Excited expression       |
| 8-15    | Custom        | custom_N.bin           | User-defined faces       |

## Module Descriptions

### 1. Modbus Handler (`modbus_handler.rs`)
- Implements Modbus RTU protocol
- Register read/write operations
- CRC validation
- Frame parsing and response generation

### 2. Face Manager (`face_manager.rs`)
- Maps face IDs to binary assets
- Loads face data from SPIFFS
- Caches frequently used faces
- Validates face format

### 3. Display HAL (`display_hal.rs`)
- Abstract interface for e-paper display
- SPI communication primitives
- Partial/full refresh control
- Hardware-agnostic display operations

### 4. Expression State Machine (`expression_fsm.rs`)
- Manages face transitions
- Animation timing
- Expression sequences
- Interrupt-safe state updates

### 5. Asset Manager (`asset_manager.rs`)
- SPIFFS file system interface
- Binary asset loading
- Format validation (header parsing)
- Memory-efficient streaming

## Data Flow: Modbus Command to Display Update

```
1. UART RX Interrupt → Modbus Frame Buffer
2. Frame Complete → Parse Modbus PDU
3. Register Write (FACE_ID=4) → Face Manager
4. Face Manager → Load Asset (4_surprised.bin)
5. Asset Manager → Read from SPIFFS
6. Face Manager → Decode Binary Format
7. Display HAL → Write to E-Paper RAM
8. Display HAL → Trigger Partial Refresh
9. Modbus Response → Confirm Write Success
```

## Error Handling Strategy

### Rust Result Types
```rust
pub enum DisplayError {
    SpiTimeout,
    InvalidFaceId,
    AssetNotFound,
    MemoryExhausted,
    HardwareFault,
}

pub type Result<T> = core::result::Result<T, DisplayError>;
```

### Graceful Degradation
1. **Asset Load Fail**: Revert to last known good face
2. **SPI Timeout**: Retry with exponential backoff
3. **Memory Exhausted**: Clear cache, reload essential assets only
4. **Hardware Fault**: Log error to Modbus register, trigger watchdog

## Performance Targets

- **Modbus Response Time**: < 50ms
- **Face Transition Time**: < 500ms (partial refresh)
- **Full Refresh Time**: < 2s
- **Memory Footprint**: < 128KB RAM
- **Flash Usage**: < 512KB (excluding assets)

## Security Considerations

1. **Input Validation**: All Modbus values validated against safe ranges
2. **Memory Safety**: Rust ownership prevents buffer overflows
3. **Watchdog**: Hardware watchdog resets system on hang
4. **Read-Only Assets**: SPIFFS mounted read-only after boot

## Development Phases

### Phase 1: Core Infrastructure (Week 1)
- [ ] Rust embedded project setup (esp-idf-hal)
- [ ] Hardware abstraction layer
- [ ] Basic SPI bit-banging driver
- [ ] Display initialization sequence

### Phase 2: Modbus Protocol (Week 2)
- [ ] Modbus RTU frame parser
- [ ] Register map implementation
- [ ] CRC-16 calculation
- [ ] UART interrupt handler

### Phase 3: Face Management (Week 3)
- [ ] Asset manager (SPIFFS integration)
- [ ] Face binary format parser
- [ ] Face rendering to display buffer
- [ ] Partial refresh optimization

### Phase 4: Integration & Testing (Week 4)
- [ ] End-to-end face switching via Modbus
- [ ] Animation state machine
- [ ] Error handling and recovery
- [ ] Performance optimization

### Phase 5: Advanced Features (Future)
- [ ] Modbus TCP over WiFi
- [ ] MQTT bridge for IoT integration
- [ ] OTA firmware updates
- [ ] Multi-language support

## Testing Strategy

### Unit Tests
- Modbus CRC calculation
- Register bounds checking
- Face ID validation
- Asset header parsing

### Integration Tests
- Modbus command → face change
- Error recovery scenarios
- Memory leak detection
- Performance benchmarks

### Hardware-in-Loop Tests
- Actual display refresh
- SPI timing verification
- Power consumption measurement
- Temperature stress testing

## Tools & Dependencies

- **Rust Toolchain**: nightly-2024-12-01
- **Target**: riscv32imc-unknown-none-elf (ESP32-C3)
- **Crates**:
  - `esp-idf-hal`: ESP32 hardware abstraction
  - `embedded-hal`: Generic embedded traits
  - `heapless`: Static data structures
  - `modbus-core`: Modbus protocol library
  - `defmt`: Efficient logging

## Migration Path from C++

1. **Keep existing Python scripts**: Asset generation unchanged
2. **Parallel development**: Run Rust and C++ side-by-side during transition
3. **Incremental modules**: Port one module at a time, test thoroughly
4. **Use C FFI if needed**: Call legacy C++ code from Rust during transition
5. **Validation**: Ensure bit-identical display output

## Conclusion

This architecture provides:
- **Industrial-grade reliability** via Modbus protocol
- **Memory safety** through Rust
- **Modularity** for easy maintenance and extension
- **Performance** optimized for embedded systems
- **Scalability** for future features (WiFi, MQTT, etc.)

The system is designed to be production-ready for industrial automation environments while maintaining the playful personality of BMO.
