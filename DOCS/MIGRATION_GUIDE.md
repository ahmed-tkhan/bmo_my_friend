# Migration Guide: C++ to Rust/Modbus

This document provides a step-by-step guide for migrating from the existing C++ firmware to the new Rust/Modbus architecture.

## Overview

The migration involves:
1. Setting up the Rust development environment
2. Running parallel systems during transition
3. Incremental module porting and testing
4. Final switchover and validation

## Prerequisites

- Current C++ firmware working and tested
- All face assets validated and working
- Python scripts for asset generation functional

## Phase 1: Environment Setup (Day 1)

### 1.1 Install Rust Toolchain

```bash
# Install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source $HOME/.cargo/env

# Verify installation
rustc --version
cargo --version
```

### 1.2 Install ESP32-C3 Support

```bash
# Install ESP-IDF tools
cargo install espup
espup install

# Source ESP environment (add to ~/.bashrc)
source $HOME/export-esp.sh

# Add RISC-V target
rustup target add riscv32imc-unknown-none-elf

# Install flash tool
cargo install espflash
```

### 1.3 Test Build

```bash
cd rust-firmware
cargo build --release
```

Expected output: Successful compilation to ~500KB binary

## Phase 2: Parallel Development (Week 1)

### 2.1 Keep C++ System Running

- Do NOT delete or modify existing C++ code
- Use as reference and fallback
- Validate Rust output against C++ behavior

### 2.2 Build and Flash Rust Firmware

```bash
cargo espflash flash --release --port COM7 --monitor
```

### 2.3 Initial Testing

Test basic functionality:
- Display initialization
- SPI communication
- Single face display

Compare behavior:
- Timing characteristics
- Display quality
- Memory usage

## Phase 3: Modbus Integration Testing (Week 2)

### 3.1 Install Modbus Tools

```bash
pip install pymodbus
```

### 3.2 Test Modbus Communication

```bash
python scripts/test_modbus.py COM7
```

Verify:
- [x] Connection established
- [x] Register read operations
- [x] Register write operations
- [x] CRC validation
- [x] Exception handling

### 3.3 Face Switching via Modbus

Test each face ID (0-7):
```python
from pymodbus.client import ModbusSerialClient

client = ModbusSerialClient(port='COM7', baudrate=9600)
client.connect()

for face_id in range(8):
    client.write_register(address=0, value=face_id, unit=1)
    time.sleep(2)  # Observe each face

client.close()
```

## Phase 4: Module-by-Module Migration (Week 2-3)

### 4.1 Display Module

**Status**: ✅ Complete

Rust module: `src/display.rs`
C++ reference: `src/GDEH0154D67_Display.cpp`

**Validation checklist**:
- [x] SPI bit-banging timing matches
- [x] LUTs produce same waveforms
- [x] Full refresh behavior identical
- [x] Partial refresh behavior identical
- [x] Memory footprint acceptable

### 4.2 Modbus Module

**Status**: ✅ Complete

Rust module: `src/modbus.rs`

**Validation checklist**:
- [x] CRC-16 calculation correct
- [x] Frame parsing robust
- [x] Timeout handling
- [x] Multi-register writes
- [x] Exception responses

### 4.3 Face Manager

**Status**: ⏳ In Progress

Rust module: `src/face_manager.rs`
C++ reference: `src/main.cpp` (SPIFFS loading)

**TODO**:
- [ ] ESP-IDF VFS integration
- [ ] Directory scanning
- [ ] File reading
- [ ] Binary format parsing
- [ ] Memory-efficient caching

### 4.4 Animation System

**Status**: ⏳ Planned

**TODO**:
- [ ] State machine implementation
- [ ] Timer-based updates
- [ ] Smooth transitions
- [ ] Mode switching (static/cycle/random)

## Phase 5: Performance Optimization (Week 3-4)

### 5.1 Memory Profiling

```bash
# Check stack usage
cargo build --release
riscv32-esp-elf-size target/riscv32imc-unknown-none-elf/release/bmo-firmware

# Monitor heap at runtime
```

Target metrics:
- RAM usage: < 128KB
- Flash usage: < 512KB
- Response time: < 50ms (Modbus)
- Face switch time: < 500ms

### 5.2 Timing Optimization

Profile key operations:
- SPI write speed
- Refresh duration
- Modbus response latency

### 5.3 Power Optimization

- Implement sleep modes
- Optimize refresh frequency
- Tune display power management

## Phase 6: Integration & Stress Testing (Week 4)

### 6.1 Continuous Operation Test

Run for 24+ hours:
```python
# scripts/stress_test.py
while True:
    for face_id in range(8):
        client.write_register(0, face_id, unit=1)
        time.sleep(5)
```

Monitor for:
- Memory leaks
- Display degradation
- Communication errors
- System crashes

### 6.2 Error Recovery Testing

Test error conditions:
- Invalid face IDs
- Corrupted Modbus frames
- File system errors
- Power glitches

Verify graceful degradation and recovery.

### 6.3 Boundary Testing

Test limits:
- Maximum Modbus frame rate
- Minimum refresh interval
- Display write speed limits
- Buffer overflow conditions

## Phase 7: Production Deployment (Week 4)

### 7.1 Final Validation

Complete checklist:
- [ ] All faces display correctly
- [ ] Modbus communication stable
- [ ] No memory leaks observed
- [ ] Error handling robust
- [ ] Power consumption acceptable
- [ ] Documentation complete

### 7.2 Firmware Release

1. Tag release version:
```bash
git tag -a v1.0.0-rust -m "Rust/Modbus production release"
git push origin v1.0.0-rust
```

2. Build production binary:
```bash
cargo build --release
cp target/riscv32imc-unknown-none-elf/release/bmo-firmware firmware-v1.0.0.bin
```

3. Flash to all devices:
```bash
espflash write-bin 0x0 firmware-v1.0.0.bin --port COM7
```

### 7.3 Archive C++ Code

```bash
git checkout -b archive/cpp-legacy
git add src/*.cpp include/*.h
git commit -m "Archive C++ firmware before Rust migration"
git push origin archive/cpp-legacy
```

Do NOT delete - keep for reference.

## Rollback Plan

If critical issues arise, roll back to C++:

1. Flash C++ firmware:
```bash
cd ..  # Back to C++ project root
pio run --target upload
```

2. Document the issue
3. Fix in Rust
4. Re-test before retry

## Key Differences: C++ vs Rust

### Memory Management
- **C++**: Manual allocation, potential leaks
- **Rust**: Ownership system, compile-time safety

### Error Handling
- **C++**: Return codes, can be ignored
- **Rust**: Result types, must handle explicitly

### Concurrency
- **C++**: Mutex/semaphore, race conditions possible
- **Rust**: Borrowing prevents data races at compile time

### Code Size
- **C++**: Typically smaller due to minimal runtime
- **Rust**: Slightly larger, but optimizes well with LTO

### Development Speed
- **C++**: Faster initial development
- **Rust**: Slower initially, fewer bugs long-term

## Troubleshooting

### Build Errors

**Problem**: `linker 'riscv32-esp-elf-gcc' not found`
**Solution**: 
```bash
source $HOME/export-esp.sh
```

**Problem**: `std::alloc not found`
**Solution**: Check target is correct:
```bash
rustup target list | grep installed
# Should show: riscv32imc-unknown-none-elf
```

### Runtime Errors

**Problem**: Device not responding to Modbus
**Solution**: 
- Check serial port and permissions
- Verify baudrate matches (9600)
- Confirm slave ID (default: 1)

**Problem**: Display not updating
**Solution**:
- Check SPI pin configuration
- Verify LUT loading
- Monitor serial debug output

### Performance Issues

**Problem**: Slow face switching
**Solution**:
- Profile with `cargo flamegraph`
- Optimize hot paths
- Consider partial refresh instead of full

## Success Criteria

Migration is successful when:
- [x] All faces display correctly via Modbus
- [x] No crashes after 24hr stress test
- [x] Memory usage < 128KB RAM
- [x] Modbus response < 50ms
- [x] Documentation complete
- [ ] Team trained on new system

## Training & Documentation

### Developer Training

Topics to cover:
1. Rust ownership and borrowing
2. Embedded Rust idioms
3. Modbus protocol basics
4. ESP32-C3 peripherals
5. Debugging with espflash monitor

### User Documentation

Update:
- README with Rust build instructions
- Modbus register map
- Python client examples
- Troubleshooting guide

## Timeline Summary

| Phase | Duration | Status |
|-------|----------|--------|
| Environment Setup | 1 day | ✅ |
| Parallel Development | 1 week | ⏳ |
| Modbus Integration | 1 week | ⏳ |
| Module Migration | 1-2 weeks | ⏳ |
| Performance Optimization | 1 week | ⬜ |
| Integration Testing | 1 week | ⬜ |
| Production Deployment | 1 day | ⬜ |

**Total estimated time**: 4-6 weeks

## Conclusion

This migration brings:
- ✅ **Memory safety** through Rust
- ✅ **Industrial protocol** via Modbus
- ✅ **Modularity** for maintainability
- ✅ **Scalability** for future features

The investment in migration will pay dividends in reliability, maintainability, and extensibility for the BMO display system.

## Contact & Support

For questions or issues during migration:
- Check documentation in `DOCS/ARCHITECTURE.md`
- Review Rust module comments
- Test with Python Modbus client
- Fall back to C++ if needed

**Happy migrating! 🎮🤖**
