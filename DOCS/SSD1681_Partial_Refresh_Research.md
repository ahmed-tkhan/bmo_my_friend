# SSD1681 Partial Refresh Research 🔬
*For BMO My Friend - Smooth Animation Implementation*

## 🎯 Goal
Implement partial refresh on the SSD1681 controller to enable smooth BMO facial animations without full-screen flicker.

## 📋 SSD1681 Partial Refresh Commands

### Key Commands for Partial Update:

#### **0x44 - Set RAM X Address Start/End Position**
```cpp
EPD_WriteCMD(0x44);
EPD_WriteDATA(x_start);    // Start address (byte boundary)
EPD_WriteDATA(x_end);      // End address (byte boundary)
```
- Controls horizontal update region
- Addresses are in bytes (8 pixels per byte)
- For 200px width: 0x00 to 0x18 (25 bytes total)

#### **0x45 - Set RAM Y Address Start/End Position** 
```cpp
EPD_WriteCMD(0x45);
EPD_WriteDATA(y_start_low);   // Y start LSB
EPD_WriteDATA(y_start_high);  // Y start MSB  
EPD_WriteDATA(y_end_low);     // Y end LSB
EPD_WriteDATA(y_end_high);    // Y end MSB
```
- Controls vertical update region
- For 200px height: 0x0000 to 0x00C7 (199 in hex)

#### **0x4E/0x4F - Set RAM Address Counter**
```cpp
EPD_WriteCMD(0x4E);
EPD_WriteDATA(x_address);     // Set X counter

EPD_WriteCMD(0x4F); 
EPD_WriteDATA(y_low);         // Set Y counter LSB
EPD_WriteDATA(y_high);        // Set Y counter MSB
```
- Sets starting position for writing data
- Must be within the region set by 0x44/0x45

#### **0x22 - Display Update Control 2**
```cpp
EPD_WriteCMD(0x22);
EPD_WriteDATA(0xCF);    // Partial update mode
// vs 0xF7 for full update
```

**Update Control Values:**
- `0xF7` - Full update (what we currently use)
- `0xCF` - Partial update (faster, less flicker)
- `0xFF` - Fast partial update (fastest, but may have ghosting)

## 🧠 BMO Face Region Mapping

### Suggested BMO Face Regions:
```
BMO Display Layout (200x200):
┌─────────────────────────────┐
│        TOP AREA (text)      │  Y: 0-40
├─────────────────────────────┤
│    👁        👁           │  Y: 40-100 (EYES)
│      LEFT    RIGHT         │  X: 40-80, 120-160
├─────────────────────────────┤
│          👄               │  Y: 120-160 (MOUTH)
│        MOUTH               │  X: 80-120
├─────────────────────────────┤
│      BOTTOM AREA           │  Y: 160-200
└─────────────────────────────┘
```

### Region Definitions:
```cpp
// BMO Face Regions for Partial Updates
#define EYES_REGION_Y_START     40
#define EYES_REGION_Y_END       100
#define LEFT_EYE_X_START        5    // Byte 5 (40px)
#define LEFT_EYE_X_END          10   // Byte 10 (80px)
#define RIGHT_EYE_X_START       15   // Byte 15 (120px) 
#define RIGHT_EYE_X_END         20   // Byte 20 (160px)

#define MOUTH_REGION_Y_START    120
#define MOUTH_REGION_Y_END      160
#define MOUTH_X_START           10   // Byte 10 (80px)
#define MOUTH_X_END             15   // Byte 15 (120px)

#define TEXT_REGION_Y_START     0
#define TEXT_REGION_Y_END       40
#define TEXT_X_START            0
#define TEXT_X_END              24   // Full width
```

## 🔄 Partial Update Sequence

### Step-by-Step Implementation:
```cpp
void EPD_PartialUpdate_Region(uint8_t x_start, uint8_t x_end, 
                             uint16_t y_start, uint16_t y_end) {
    // 1. Set region boundaries
    EPD_WriteCMD(0x44);           // Set X region
    EPD_WriteDATA(x_start);
    EPD_WriteDATA(x_end);
    
    EPD_WriteCMD(0x45);           // Set Y region  
    EPD_WriteDATA(y_start & 0xFF);
    EPD_WriteDATA((y_start >> 8) & 0xFF);
    EPD_WriteDATA(y_end & 0xFF);
    EPD_WriteDATA((y_end >> 8) & 0xFF);
    
    // 2. Set RAM pointer to region start
    EPD_WriteCMD(0x4E);
    EPD_WriteDATA(x_start);
    
    EPD_WriteCMD(0x4F);
    EPD_WriteDATA(y_start & 0xFF);
    EPD_WriteDATA((y_start >> 8) & 0xFF);
    
    // 3. Write data for this region only
    EPD_WriteCMD(0x24);           // Write RAM
    // ... write region data here ...
    
    // 4. Trigger partial update
    EPD_WriteCMD(0x22);
    EPD_WriteDATA(0xCF);          // Partial update mode
    EPD_WriteCMD(0x20);           // Execute update
    Epaper_READBUSY();
}
```

## ⚡ Performance Characteristics

### Update Times (Estimated):
- **Full Update**: ~2-4 seconds (current)
- **Partial Update**: ~200-800ms (much faster!)
- **Fast Partial**: ~100-400ms (fastest, some ghosting)

### Region Size Impact:
- Smaller regions = faster updates
- Eye blink (40x60 region) = ~200ms
- Mouth change (40x40 region) = ~150ms
- Full face (160x160 region) = ~600ms

## 🎭 BMO Animation Strategies

### 1. **Eye Blinking Animation**
```cpp
void BMO_BlinkEyes() {
    // Update only eye regions
    EPD_PartialUpdate_Region(LEFT_EYE_X_START, LEFT_EYE_X_END,
                            EYES_REGION_Y_START, EYES_REGION_Y_END);
    delay(150);  // Closed
    EPD_PartialUpdate_Region(LEFT_EYE_X_START, LEFT_EYE_X_END,
                            EYES_REGION_Y_START, EYES_REGION_Y_END);
}
```

### 2. **Expression Transitions**
```cpp
void BMO_TransitionExpression(BMO_Expression from, BMO_Expression to) {
    // 1. Update eyes if different
    if (expressions[from].eyes != expressions[to].eyes) {
        updateEyeRegion(expressions[to].eyes);
    }
    
    // 2. Update mouth if different  
    if (expressions[from].mouth != expressions[to].mouth) {
        updateMouthRegion(expressions[to].mouth);
    }
    
    // 3. Update any text/symbols
    updateTextRegion(expressions[to].text);
}
```

### 3. **Talking Animation**
```cpp
void BMO_TalkAnimation(const char* text) {
    // Animate mouth while displaying text
    for (int i = 0; text[i]; i++) {
        updateMouthRegion(MOUTH_OPEN);
        delay(100);
        updateMouthRegion(MOUTH_CLOSED); 
        delay(50);
        updateTextRegion(text, i); // Show character by character
    }
}
```

## ⚠️ Important Considerations

### **Ghosting Issues:**
- Partial updates can cause ghosting (faint previous images)
- Solution: Occasional full refresh to clear ghosting
- Recommended: Full refresh every 10-20 partial updates

### **Temperature Dependency:**
- E-ink performance varies with temperature
- May need temperature compensation for refresh timing
- SSD1681 has built-in temperature sensor (0x18 command)

### **Power Consumption:**
- Partial updates use less power
- Good for battery-powered BMO
- Can enable more frequent animations

### **Memory Management:**
- Need separate buffers for each face region
- Could use compression for face data
- SPIFFS can store pre-computed face regions

## 🔬 Testing Strategy

### Phase 1: Basic Partial Refresh
1. Test simple rectangle updates
2. Verify region boundaries work correctly
3. Test different update modes (0xCF vs 0xFF)

### Phase 2: BMO Face Regions
1. Define and test eye regions
2. Define and test mouth regions  
3. Test text overlay regions

### Phase 3: Animation Testing
1. Simple eye blink test
2. Mouth movement test
3. Expression transition test
4. Ghosting evaluation and mitigation

### Phase 4: Optimization
1. Minimize region sizes
2. Optimize update timing
3. Implement smart refresh strategy
4. Battery life testing

## 📚 References
- SSD1681 Datasheet (Solomon Systech)
- Good Display Application Notes
- E-ink Partial Refresh Best Practices
- Adventure Time BMO Character Reference 🤖

---
*Research compiled for making BMO animations as smooth as butter!* ✨
