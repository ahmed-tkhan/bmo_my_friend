# InkLink DEBUG Project - Waveshare 1.54" E-ink Display

## 🎯 Project Purpose

This is a debugging project to test e-ink display functionality using:
- **ESP32 Dev Kit V1** (easier to debug than XIAO ESP32-C3)
- **Waveshare 1.54" E-ink HAT** (known-good hardware configuration)
- **Black/White display** (simpler than 4-grayscale)

## 📋 Hardware Configuration

### ESP32 Dev Kit V1 + Waveshare 1.54" HAT

**Display Specs:**
- Model: Waveshare 1.54" E-ink Display
- Resolution: 200 x 200 pixels
- Colors: Black/White (no grayscale in this debug setup)
- Interface: 4-wire SPI (BS pin = 0)
- HAT: Pre-configured and attached

**Pin Configuration:**
```
Display Pin -> ESP32 Dev Kit V1
VCC         -> 3.3V
GND         -> GND
DIN (MOSI)  -> GPIO23 (Hardware SPI)
CLK (SCK)   -> GPIO18 (Hardware SPI)
CS          -> GPIO15
DC          -> GPIO2
RST         -> GPIO4
BUSY        -> GPIO5
```

## 🔧 Development Strategy

### Phase 1: Debug Project (Current)
- ✅ Use ESP32 Dev Kit V1 (easier debugging, more pins, stable power)
- ✅ Use Waveshare HAT (known-good configuration)
- ✅ Test Black/White display first (simpler than grayscale)
- ✅ Verify GxEPD2 library compatibility
- ✅ Establish working baseline

### Phase 2: Production Project (Later)
- Transfer working code to SEEED XIAO ESP32-C3
- Adapt to GDEM0154I61 (4-grayscale display)
- Use custom wiring/HAT configuration
- Implement advanced features

## 🎯 Test Sequence

The debug program runs these tests:

1. **System Information** - Verify ESP32 and memory
2. **Pin Configuration** - Display GPIO assignments
3. **Display Initialization** - Test GxEPD2 setup
4. **Minimal Test** - Simple border and cross pattern
5. **Welcome Screen** - Text and graphics combination
6. **System Info Screen** - Dynamic system data
7. **Test Pattern** - Comprehensive graphics test
8. **Live Updates** - 30-second refresh cycle

## ✅ Success Criteria

**If working correctly, you should see:**

1. **Serial Output:**
   ```
   [INFO] Display width: 200 pixels  
   [INFO] Display height: 200 pixels
   [INFO] Display initialized successfully
   ```

2. **Display Activity:**
   - Screen "flashes" during updates (normal for e-ink)
   - Clear black/white graphics appear
   - Text is readable and properly positioned
   - Update timing is consistent

3. **Test Results:**
   - Border and cross pattern visible
   - Welcome screen with centered text
   - System info with real data
   - Graphics test with shapes and text
   - Live updates every 30 seconds

## 🔍 Troubleshooting

**If display doesn't work:**

1. **Check Serial Output** - Look for initialization errors
2. **Verify Wiring** - Ensure HAT is properly connected
3. **Try Different Models** - GxEPD2 has multiple 1.54" variants:
   ```cpp
   // Try these alternatives in main.cpp:
   GxEPD2_154_M09  // Alternative model 1
   GxEPD2_154_M10  // Alternative model 2  
   GxEPD2_154      // Generic model
   ```
4. **Power Supply** - Ensure ESP32 has sufficient power
5. **SPI Settings** - Hardware SPI should work automatically

**Common Issues:**
- Wrong GxEPD2 display model (try alternatives above)
- Loose connections on breadboard
- Power supply issues
- Wrong pin assignments

## 📁 File Structure

```
InkLink-DEBUG/
├── platformio.ini          # ESP32 Dev Kit V1 config
├── include/
│   ├── config.h            # Display pins and settings
│   └── debug.h             # Debug macros
├── src/
│   ├── main.cpp            # Main test program
│   └── debug.cpp           # Debug functions
└── README.md               # This file
```

## 🚀 Next Steps

1. **Test the Debug Setup** - Upload and verify display works
2. **Document Working Configuration** - Note exact model/settings that work
3. **Transfer to Production** - Adapt working code to XIAO ESP32-C3
4. **Add Advanced Features** - Network, image fetching, etc.

The goal is to establish a **known-working baseline** with easier hardware before tackling the more complex XIAO ESP32-C3 + GDEM0154I61 setup.

## ⚠️ Important Notes

- **Black/White Only**: This debug setup doesn't use grayscale
- **Different Display**: Final project will use GDEM0154I61 with SSD1681
- **Different MCU**: Final project will use XIAO ESP32-C3
- **Proven Hardware**: Waveshare HAT eliminates wiring issues

Success here proves the software approach works, then we can adapt to the production hardware!
