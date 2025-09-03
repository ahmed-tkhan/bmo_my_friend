# BMO My Friend
A cute little BMO from Adventure Time on an e-ink display!

░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░

░░░░▄██████████████████████▄░░░░

░░░░█░░░░░░░░░░░░░░░░░░░░░░█░░░░

░░░░█░▄██████████████████▄░█░░░░

░░░░█░█░░░░░░░░░░░░░░░░░░█░█░░░░

░░░░█░█░░░░░░░░░░░░░░░░░░█░█░░░░

░░░░█░█░░█░░░░░░░░░░░░█░░█░█░░░░

░░░░█░█░░░░░▄▄▄▄▄▄▄▄░░░░░█░█░░░░

░░░░█░█░░░░░▀▄░░░░▄▀░░░░░█░█░░░░

░░░░█░█░░░░░░░▀▀▀▀░░░░░░░█░█░░░░

░░░░█░█░░░░░░░░░░░░░░░░░░█░█░░░░

░█▌░█░▀██████████████████▀░█░▐█░

░█░░█░░░░░░░░░░░░░░░░░░░░░░█░░█░

░█░░█░████████████░░░░░██░░█░░█░

░█░░█░░░░░░░░░░░░░░░░░░░░░░█░░█░

░█░░█░░░░░░░░░░░░░░░▄░░░░░░█░░█░

░▀█▄█░░░▐█▌░░░░░░░▄███▄░██░█▄█▀░

░░░▀█░░█████░░░░░░░░░░░░░░░█▀░░░

░░░░█░░░▐█▌░░░░░░░░░▄██▄░░░█░░░░

░░░░█░░░░░░░░░░░░░░▐████▌░░█░░░░

░░░░█░▄▄▄░▄▄▄░░░░░░░▀██▀░░░█░░░░

░░░░█░░░░░░░░░░░░░░░░░░░░░░█░░░░

░░░░▀██████████████████████▀░░░░

░░░░░░░░██░░░░░░░░░░░░██░░░░░░░░

░░░░░░░░██░░░░░░░░░░░░██░░░░░░░░

░░░░░░░░██░░░░░░░░░░░░██░░░░░░░░

░░░░░░░░██░░░░░░░░░░░░██░░░░░░░░

░░░░░░░▐██░░░░░░░░░░░░██▌░░░░░░░

░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░




# BMO My Friend - E-ink Display Project

## 🎯 Project Purpose

This is BMO My Friend - a cute little BMO from Adventure Time displayed on an e-ink screen! The project uses:
- **ESP32 Dev Kit V1** (easier to debug than XIAO ESP32-C3)  
- **Waveshare 1.54" E-ink HAT** (known-good hardware configuration)
- **4-level grayscale display** for better BMO expressions

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

## 🤖 BMO Development Strategy

### Phase 1: Current BMO Implementation
- ✅ Use ESP32 Dev Kit V1 (stable, easy debugging)
- ✅ Use Waveshare HAT (reliable connection)
- ✅ Implement 4-level grayscale for BMO expressions
- ✅ Display BMO face patterns and animations
- ✅ Load BMO graphics from SPIFFS

### Phase 2: Enhanced BMO Features (Future)
- Add BMO voice samples via I2S/DAC
- Implement interactive BMO responses
- Add WiFi for BMO updates
- Create multiple BMO expressions
- Add Adventure Time themed animations

## 🎯 BMO Display Features

BMO displays these awesome features:

1. **BMO Face Display** - Shows BMO's adorable face
2. **4-Level Grayscale** - Smooth BMO expressions  
3. **Pattern Showcase** - Geometric patterns like BMO's games
4. **Image Loading** - Loads BMO graphics from SPIFFS
5. **Display Updates** - Smooth e-ink transitions
6. **Adventure Time Style** - True to the show's aesthetic

## ✅ BMO Success Indicators

**If BMO is working correctly, you should see:**

1. **Serial Output:**
   ```
   [INFO] BMO My Friend initializing...
   [INFO] Display width: 200 pixels  
   [INFO] Display height: 200 pixels
   [INFO] BMO display ready!
   ```

2. **BMO Display Activity:**
   - Screen updates with BMO-style patterns
   - 4-level grayscale creates smooth shading
   - Clear geometric patterns (like BMO's games)
   - Adventure Time aesthetic maintained
   - Proper e-ink refresh timing

3. **BMO Features Working:**
   - Face patterns visible and cute
   - Grayscale gradients smooth  
   - Image loading from SPIFFS successful
   - Graphics rendering properly
   - Display updates without artifacts

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
BMO-My-Friend/
├── platformio.ini          # ESP32 Dev Kit V1 config
├── include/
│   ├── config_gooddisplay.h # Display pins and settings
│   └── debug.h             # Debug macros
├── src/
│   ├── main.cpp            # Main BMO program
│   └── debug.cpp           # Debug functions
├── data/
│   └── noise_200x200_2bpp.bin # BMO graphics data
└── README.md               # This file
```

## 🚀 BMO's Next Adventures

1. **Perfect BMO Display** - Fine-tune the cute expressions  
2. **Add BMO Sounds** - Implement "BMO!" voice samples
3. **Interactive BMO** - Respond to button presses
4. **Adventure Time Updates** - WiFi updates with new BMO content
5. **BMO Games** - Simple games like in the show

The goal is to make the most adorable BMO friend possible! 💚

## 🎮 BMO Technical Notes

- **4-Level Grayscale**: Perfect for BMO's subtle expressions
- **GDEM0154I61 Display**: Uses SSD1681 controller for smooth updates  
- **ESP32 Power**: Plenty of processing for BMO's personality
- **Adventure Time Authentic**: True to BMO's character design

**BMO says:** "I am a real boy!" And now BMO lives in your e-ink display! 🤖✨

