/**
 * @file GDEH0154D67_Display.cpp
 * @brief Implementation of professional E-Paper Display Controller
 * 
 * This implementation provides comprehensive control for the GDEH0154D67 1.54" e-paper display.
 * The code is based on manufacturer reference implementation with significant improvements
 * in structure, documentation, and error handling.
 * 
 * @author Generated from manufacturer code
 * @date 2025
 * @version 1.0
 */

#include "GDEH0154D67_Display.h"

// ===== 4-GRAYSCALE LOOKUP TABLE =====
// This 159-byte LUT defines the voltage waveforms for 4-level grayscale operation
static const unsigned char LUT_DATA_4Gray[159] = {
    // Voltage transition sequences for grayscale rendering
    0x40, 0x48, 0x80, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x8,  0x48, 0x10, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x2,  0x48, 0x4,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x20, 0x48, 0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0xA,  0x19, 0x0,  0x3,  0x8,  0x0,  0x0,          
    0x14, 0x1,  0x0,  0x14, 0x1,  0x0,  0x3,          
    0xA,  0x3,  0x0,  0x8,  0x19, 0x0,  0x0,          
    0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,          
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x0,  0x0,  0x0,      
    0x22, 0x17, 0x41, 0x0,  0x32, 0x1C
};

// ===== CONSTRUCTOR & DESTRUCTOR =====

GDEH0154D67_Display::GDEH0154D67_Display(int busy_pin, int rst_pin, int dc_pin, 
                                         int cs_pin, int sck_pin, int sdi_pin)
    : busy_pin_(busy_pin), rst_pin_(rst_pin), dc_pin_(dc_pin),
      cs_pin_(cs_pin), sck_pin_(sck_pin), sdi_pin_(sdi_pin),
      initialized_(false), debug_enabled_(false), last_error_("No error") {
    
    debugPrint("Display controller created with pin configuration");
}

GDEH0154D67_Display::~GDEH0154D67_Display() {
    if (initialized_) {
        debugPrint("Destructor: Putting display to sleep");
        enterDeepSleep();
    }
}

// ===== INITIALIZATION & SETUP =====

void GDEH0154D67_Display::initializePins() {
    debugPrint("Initializing GPIO pins");
    
    // Configure pin directions
    pinMode(busy_pin_, INPUT);   // BUSY is an input from display
    pinMode(rst_pin_, OUTPUT);   // Reset control output
    pinMode(dc_pin_, OUTPUT);    // Data/Command control output
    pinMode(cs_pin_, OUTPUT);    // SPI Chip Select output
    pinMode(sck_pin_, OUTPUT);   // SPI Clock output
    pinMode(sdi_pin_, OUTPUT);   // SPI Data output
    
    // Set initial states - CS and RST inactive (high)
    setCS_Inactive();
    setRST_Inactive();
    
    debugPrint("GPIO pins initialized successfully");
}

bool GDEH0154D67_Display::initializeMonochrome() {
    debugPrint("Starting monochrome display initialization");
    
    // Hardware reset sequence - essential for reliable operation
    setRST_Active();     // Assert reset (low)
    delay(10);           // Hold reset for at least 10ms
    setRST_Inactive();   // Release reset (high)
    delay(10);           // Wait for display to boot
    
    // Wait for display to be ready after reset
    waitBusy();
    
    // Soft reset command - clears internal state
    writeCommand(0x12);  // SWRESET
    waitBusy();
    
    // Configure display driver output control
    writeCommand(0x01);  // Driver output control
    writeData(0xC7);     // Set to 200 lines (0xC7 = 199+1)
    writeData(0x00);     // Additional settings
    writeData(0x00);     // Additional settings
    
    // Set data entry mode - controls how RAM addresses increment
    writeCommand(0x11);  // Data entry mode
    writeData(0x01);     // X increment, Y increment
    
    // Define the active display window - X address range
    writeCommand(0x44);  // Set RAM X address start/end position
    writeData(0x00);     // X start = 0
    writeData(0x18);     // X end = 24 (25*8 = 200 pixels)
    
    // Define the active display window - Y address range  
    writeCommand(0x45);  // Set RAM Y address start/end position
    writeData(0xC7);     // Y start = 199 (bottom-up addressing)
    writeData(0x00);     // Y start high byte
    writeData(0x00);     // Y end = 0
    writeData(0x00);     // Y end high byte
    
    // Configure border waveform - controls border color during refresh
    writeCommand(0x3C);  // BorderWaveform
    writeData(0x05);     // Border follows display content
    
    // Configure built-in temperature sensor for optimal refresh
    writeCommand(0x18);  // Read built-in temperature sensor
    writeData(0x80);     // Use internal temperature sensor
    
    // Set initial RAM address pointers
    writeCommand(0x4E);  // Set RAM X address counter
    writeData(0x00);     // Start at X = 0
    writeCommand(0x4F);  // Set RAM Y address counter
    writeData(0xC7);     // Start at Y = 199 (bottom)
    writeData(0x00);     // High byte
    
    // Final ready check
    waitBusy();
    
    initialized_ = true;
    debugPrint("Monochrome initialization completed successfully");
    return true;
}

bool GDEH0154D67_Display::initialize4Grayscale() {
    debugPrint("Starting 4-grayscale display initialization");
    
    // Hardware reset sequence
    setRST_Active();
    delay(10);
    setRST_Inactive();
    delay(10);
    
    waitBusy();
    writeCommand(0x12); // Soft reset
    waitBusy();
    
    // Configure analog and digital blocks for grayscale operation
    writeCommand(0x74); // Set analog block control
    writeData(0x54);    // Optimal settings for grayscale
    writeCommand(0x7E); // Set digital block control
    writeData(0x3B);    // Optimal settings for grayscale
    
    // Driver output control (same as monochrome)
    writeCommand(0x01);
    writeData(0xC7);
    writeData(0x00);
    writeData(0x00);
    
    // Data entry mode
    writeCommand(0x11);
    writeData(0x01);
    
    // Set RAM address ranges
    writeCommand(0x44);
    writeData(0x00);
    writeData(0x18);
    
    writeCommand(0x45);
    writeData(0xC7);
    writeData(0x00);
    writeData(0x00);
    writeData(0x00);
    
    // Border waveform for grayscale
    writeCommand(0x3C);
    writeData(0x00);    // Different setting for grayscale
    
    // Configure voltage levels for grayscale operation
    writeCommand(0x2C);  // VCOM voltage
    writeData(LUT_DATA_4Gray[158]);  // Use value from LUT
    
    writeCommand(0x3F);  // EOPQ
    writeData(LUT_DATA_4Gray[153]);
    
    writeCommand(0x03);  // VGH
    writeData(LUT_DATA_4Gray[154]);
    
    writeCommand(0x04);  // VSH1, VSH2, VSL
    writeData(LUT_DATA_4Gray[155]);
    writeData(LUT_DATA_4Gray[156]);
    writeData(LUT_DATA_4Gray[157]);
    
    // Load the custom lookup table for grayscale waveforms
    loadGrayscaleLUT(LUT_DATA_4Gray);
    
    // Set initial RAM addresses
    writeCommand(0x4E);
    writeData(0x00);
    writeCommand(0x4F);
    writeData(0xC7);
    writeData(0x00);
    
    waitBusy();
    
    initialized_ = true;
    debugPrint("4-grayscale initialization completed successfully");
    return true;
}

// ===== FULL SCREEN OPERATIONS =====

void GDEH0154D67_Display::displayFullScreenMono(const unsigned char* image_data, bool refresh_immediately) {
    if (!initialized_) {
        setError("Display not initialized");
        return;
    }
    
    debugPrint("Loading full screen monochrome image");
    
    // Send command to write to display RAM
    writeCommand(0x24);  // Write RAM for black(0)/white(1)
    
    // Transfer all 5000 bytes of image data
    for (unsigned int i = 0; i < MONO_BUFFER_SIZE; i++) {
        // Use pgm_read_byte for compatibility with program memory storage
        writeData(pgm_read_byte(&image_data[i]));
    }
    
    // Trigger refresh if requested
    if (refresh_immediately) {
        refreshFull();
    }
    
    debugPrint("Full screen monochrome image loaded");
}

void GDEH0154D67_Display::displayFullScreen4Gray(const unsigned char* image_data, bool refresh_immediately) {
    if (!initialized_) {
        setError("Display not initialized");
        return;
    }
    
    debugPrint("Loading full screen 4-grayscale image");
    
    // 4-grayscale requires writing to both RAM buffers with processed data
    unsigned char temp_byte;
    
    // Write to RAM buffer 1
    writeCommand(0x24);
    for (unsigned int i = 0; i < GRAY_BUFFER_SIZE; i += 2) {
        temp_byte = convertGray2ToRam1(*(image_data + i), *(image_data + i + 1));
        writeData(~temp_byte);  // Invert for correct display
    }
    
    // Write to RAM buffer 2  
    writeCommand(0x26);
    for (unsigned int i = 0; i < GRAY_BUFFER_SIZE; i += 2) {
        temp_byte = convertGray2ToRam2(*(image_data + i), *(image_data + i + 1));
        writeData(~temp_byte);  // Invert for correct display
    }
    
    // Trigger refresh if requested
    if (refresh_immediately) {
        refresh4Grayscale();
    }
    
    debugPrint("Full screen 4-grayscale image loaded");
}

void GDEH0154D67_Display::clearScreen() {
    debugPrint("Clearing screen to white");
    
    if (!initialized_) {
        setError("Display not initialized");
        return;
    }
    
    // Write white data to entire display RAM
    writeCommand(0x24);
    for (unsigned int row = 0; row < DISPLAY_HEIGHT; row++) {
        for (unsigned int col = 0; col < MAX_LINE_BYTES; col++) {
            writeData(0xFF);  // 0xFF = all white pixels
        }
    }
    
    refreshFull();
    debugPrint("Screen cleared to white");
}

// ===== PARTIAL REFRESH OPERATIONS =====

void GDEH0154D67_Display::setPartialRefreshBase(const unsigned char* base_image) {
    if (!initialized_) {
        setError("Display not initialized");
        return;
    }
    
    debugPrint("Setting partial refresh base image");
    
    // For partial refresh, we need to load the base image into both RAM buffers
    // This ensures partial updates work correctly against a known background
    
    // Load base image to RAM buffer 1
    writeCommand(0x24);
    for (unsigned int i = 0; i < MONO_BUFFER_SIZE; i++) {
        writeData(pgm_read_byte(&base_image[i]));
    }
    
    // Load same base image to RAM buffer 2 (for partial refresh comparison)
    writeCommand(0x26);
    for (unsigned int i = 0; i < MONO_BUFFER_SIZE; i++) {
        writeData(pgm_read_byte(&base_image[i]));
    }
    
    // Display the base image
    refreshFull();
    debugPrint("Partial refresh base image set");
}

bool GDEH0154D67_Display::updatePartialRegion(unsigned int x_start, unsigned int y_start,
                                             const unsigned char* image_data,
                                             unsigned int width, unsigned int height) {
    if (!initialized_) {
        setError("Display not initialized");
        return false;
    }
    
    // Validate coordinates
    if (x_start + width > DISPLAY_WIDTH || y_start + height > DISPLAY_HEIGHT) {
        setError("Region coordinates exceed display bounds");
        return false;
    }
    
    debugPrint("Updating partial region");
    
    // Convert pixel coordinates to byte coordinates
    unsigned int x_start_byte = x_start / 8;
    unsigned int x_end_byte = x_start_byte + (width / 8) - 1;
    
    // Handle Y coordinate addressing (display uses bottom-up addressing)
    unsigned int y_start1 = 0;
    unsigned int y_start2 = y_start;
    if (y_start >= 256) {
        y_start1 = y_start2 / 256;
        y_start2 = y_start2 % 256;
    }
    
    unsigned int y_end1 = 0;
    unsigned int y_end2 = y_start + height - 1;
    if (y_end2 >= 256) {
        y_end1 = y_end2 / 256;
        y_end2 = y_end2 % 256;
    }
    
    // Reset display for partial update
    setRST_Active();
    delay(10);
    setRST_Inactive();
    delay(10);
    
    // Configure border for partial update
    writeCommand(0x3C);
    writeData(0x80);  // Border setting for partial refresh
    
    // Set the partial window X range
    writeCommand(0x44);
    writeData(x_start_byte);  // X start
    writeData(x_end_byte);    // X end
    
    // Set the partial window Y range
    writeCommand(0x45);
    writeData(y_start2);  // Y start low
    writeData(y_start1);  // Y start high
    writeData(y_end2);    // Y end low
    writeData(y_end1);    // Y end high
    
    // Set RAM address pointers to start of region
    writeCommand(0x4E);
    writeData(x_start_byte);
    writeCommand(0x4F);
    writeData(y_start2);
    writeData(y_start1);
    
    // Write the partial image data
    writeCommand(0x24);
    unsigned int data_size = (height * width) / 8;
    for (unsigned int i = 0; i < data_size; i++) {
        writeData(pgm_read_byte(&image_data[i]));
    }
    
    // Trigger partial refresh
    refreshPartial();
    
    debugPrint("Partial region update completed");
    return true;
}

bool GDEH0154D67_Display::updateMultipleRegions(const PartialRegion regions[5]) {
    if (!initialized_) {
        setError("Display not initialized");
        return false;
    }
    
    debugPrint("Starting multiple region update");
    
    // Reset display for partial update
    setRST_Active();
    delay(10);
    setRST_Inactive();
    delay(10);
    
    // Configure border for partial update
    writeCommand(0x3C);
    writeData(0x80);
    
    // Process each valid region
    for (int region_idx = 0; region_idx < 5; region_idx++) {
        const PartialRegion& region = regions[region_idx];
        
        // Skip invalid regions
        if (!region.isValid()) {
            continue;
        }
        
        // Validate coordinates
        if (region.x_start + region.width > DISPLAY_WIDTH || 
            region.x_start < 0 || region.y_start < 0 ||
            region.y_start - region.height > DISPLAY_HEIGHT ||
            region.height == 0 || region.width == 0 ||
            region.height > DISPLAY_HEIGHT || region.width > DISPLAY_WIDTH) {
            setError("Region coordinates exceed display bounds");
            return false;
        }
        
        // Convert coordinates and configure window
        unsigned int x_start_byte = region.x_start / 8;
        unsigned int x_end_byte = x_start_byte + (region.width / 8) - 1;
        
        unsigned int y_start1 = 0;
        unsigned int y_start2 = region.y_start - 1;  // Adjust for display addressing
        if (region.y_start >= 256) {
            y_start1 = y_start2 / 256;
            y_start2 = y_start2 % 256;
        }
        
        unsigned int y_end1 = 0;
        unsigned int y_end2 = region.y_start + region.height - 1;
        if (y_end2 >= 256) {
            y_end1 = y_end2 / 256;
            y_end2 = y_end2 % 256;
        }
        
        // Set window for this region
        writeCommand(0x44);
        writeData(x_start_byte);
        writeData(x_end_byte);
        
        writeCommand(0x45);
        writeData(y_start2);
        writeData(y_start1);
        writeData(y_end2);
        writeData(y_end1);
        
        // Set RAM address
        writeCommand(0x4E);
        writeData(x_start_byte);
        writeCommand(0x4F);
        writeData(y_start2);
        writeData(y_start1);
        
        // Write region data
        writeCommand(0x24);
        unsigned int data_size = (region.height * region.width) / 8;
        for (unsigned int i = 0; i < data_size; i++) {
            writeData(pgm_read_byte(&region.data[i]));
        }
    }
    
    // Trigger partial refresh for all regions
    refreshPartial();
    
    debugPrint("Multiple region update completed");
    return true;
}

// ===== DISPLAY REFRESH & UPDATE =====

void GDEH0154D67_Display::refreshFull() {
    debugPrint("Triggering full screen refresh");
    
    writeCommand(0x22);  // Display Update Control
    writeData(0xF7);     // Full refresh with flicker
    writeCommand(0x20);  // Activate Display Update Sequence
    waitBusy();          // Wait for refresh to complete
    
    debugPrint("Full screen refresh completed");
}

void GDEH0154D67_Display::refreshPartial() {
    debugPrint("Triggering partial refresh");
    
    writeCommand(0x22);  // Display Update Control
    writeData(0xFF);     // Partial refresh without flicker
    writeCommand(0x20);  // Activate Display Update Sequence
    waitBusy();          // Wait for refresh to complete
    
    debugPrint("Partial refresh completed");
}

void GDEH0154D67_Display::refresh4Grayscale() {
    debugPrint("Triggering 4-grayscale refresh");
    
    writeCommand(0x22);  // Display Update Control
    writeData(0xC7);     // 4-grayscale refresh mode
    writeCommand(0x20);  // Activate Display Update Sequence
    waitBusy();          // Wait for refresh to complete
    
    debugPrint("4-grayscale refresh completed");
}

// ===== POWER MANAGEMENT =====

void GDEH0154D67_Display::enterDeepSleep() {
    debugPrint("Entering deep sleep mode");
    
    writeCommand(0x10);  // Enter deep sleep command
    writeData(0x01);     // Deep sleep mode parameter
    delay(100);          // Allow time for sleep transition
    
    initialized_ = false;  // Display will need re-initialization
    debugPrint("Deep sleep mode activated");
}

bool GDEH0154D67_Display::isBusy() {
    return readBusy();
}

bool GDEH0154D67_Display::waitForReady(unsigned long timeout_ms) {
    unsigned long start_time = millis();
    
    while (readBusy()) {
        if (timeout_ms > 0 && (millis() - start_time) > timeout_ms) {
            setError("Timeout waiting for display ready");
            return false;
        }
        delay(1);  // Small delay to prevent excessive polling
    }
    
    return true;
}

// ===== LOW-LEVEL SPI COMMUNICATION =====

void GDEH0154D67_Display::spiWrite(unsigned char value) {
    spiDelay(1);
    
    // Bit-bang SPI - send 8 bits MSB first
    for (unsigned char bit = 0; bit < 8; bit++) {
        setCLK_Low();
        spiDelay(1);
        
        // Set data line based on current bit (MSB first)
        if (value & 0x80) {
            setMOSI_High();
        } else {
            setMOSI_Low();
        }
        
        value <<= 1;  // Shift to next bit
        spiDelay(1);
        delayMicroseconds(1);  // Additional timing margin
        
        setCLK_High();  // Clock the bit
        spiDelay(1);
    }
}

void GDEH0154D67_Display::writeCommand(unsigned char cmd) {
    spiDelay(1);
    setCS_Active();     // Select display
    setDC_Command();    // Set to command mode
    spiWrite(cmd);      // Send command byte
    setCS_Inactive();   // Deselect display
}

void GDEH0154D67_Display::writeData(unsigned char data) {
    spiDelay(1);
    setCS_Active();     // Select display
    setDC_Data();       // Set to data mode
    spiWrite(data);     // Send data byte
    setCS_Inactive();   // Deselect display
}

void GDEH0154D67_Display::waitBusy() {
    // Wait while BUSY signal is high (display is busy)
    while (readBusy()) {
        // Small delay to prevent excessive polling
        delay(1);
    }
}

// ===== TIMING & DELAY FUNCTIONS =====

void GDEH0154D67_Display::delayMicroseconds(unsigned int microseconds) {
    // Simple microsecond delay using busy loop
    for (; microseconds > 1; microseconds--) {
        // Empty loop for timing
    }
}

void GDEH0154D67_Display::delayMilliseconds(unsigned long milliseconds) {
    unsigned long i, j;
    for (j = 0; j < milliseconds; j++) {
        for (i = 0; i < 256; i++) {
            // Empty loop for timing
        }
    }
}

void GDEH0154D67_Display::spiDelay(unsigned char rate) {
    unsigned char i;
    while (rate) {
        for (i = 0; i < 2; i++) {
            // Empty loop for SPI timing
        }
        rate--;
    }
}

// ===== 4-GRAYSCALE PROCESSING =====

unsigned char GDEH0154D67_Display::convertGray2ToRam1(unsigned char data1, unsigned char data2) {
    unsigned char temp_data1 = data1;
    unsigned char temp_data2 = data2;
    unsigned char out_data = 0x00;
    
    // Process first byte (4 pixels)
    for (unsigned int i = 0; i < 4; i++) {
        out_data <<= 1;
        if (((temp_data1 & 0xC0) == 0xC0) || ((temp_data1 & 0xC0) == 0x40)) {
            out_data |= 0x01;
        } else {
            out_data |= 0x00;
        }
        temp_data1 <<= 2;  // Move to next 2-bit pixel
    }
    
    // Process second byte (4 pixels)
    for (unsigned int i = 0; i < 4; i++) {
        out_data <<= 1;
        if (((temp_data2 & 0xC0) == 0xC0) || ((temp_data2 & 0xC0) == 0x40)) {
            out_data |= 0x01;
        } else {
            out_data |= 0x00;
        }
        temp_data2 <<= 2;  // Move to next 2-bit pixel
    }
    
    return out_data;
}

unsigned char GDEH0154D67_Display::convertGray2ToRam2(unsigned char data1, unsigned char data2) {
    unsigned char temp_data1 = data1;
    unsigned char temp_data2 = data2;
    unsigned char out_data = 0x00;
    
    // Process first byte (4 pixels) - different bit pattern for RAM2
    for (unsigned int i = 0; i < 4; i++) {
        out_data <<= 1;
        if (((temp_data1 & 0xC0) == 0xC0) || ((temp_data1 & 0xC0) == 0x80)) {
            out_data |= 0x01;
        } else {
            out_data |= 0x00;
        }
        temp_data1 <<= 2;  // Move to next 2-bit pixel
    }
    
    // Process second byte (4 pixels) - different bit pattern for RAM2
    for (unsigned int i = 0; i < 4; i++) {
        out_data <<= 1;
        if (((temp_data2 & 0xC0) == 0xC0) || ((temp_data2 & 0xC0) == 0x80)) {
            out_data |= 0x01;
        } else {
            out_data |= 0x00;
        }
        temp_data2 <<= 2;  // Move to next 2-bit pixel
    }
    
    return out_data;
}

void GDEH0154D67_Display::loadGrayscaleLUT(const unsigned char* wave_data) {
    debugPrint("Loading 4-grayscale lookup table");
    
    writeCommand(0x32);  // Load LUT command
    
    // Load 153 bytes of LUT data (first 153 bytes of the 159-byte table)
    for (unsigned int count = 0; count < 153; count++) {
        writeData(wave_data[count]);
    }
    
    debugPrint("4-grayscale LUT loaded");
}

// ===== DEBUG HELPERS =====

void GDEH0154D67_Display::debugPrint(const char* message) {
    if (debug_enabled_) {
        Serial.print("[GDEH0154D67] ");
        Serial.println(message);
    }
}

void GDEH0154D67_Display::setError(const char* error) {
    last_error_ = error;
    if (debug_enabled_) {
        Serial.print("[GDEH0154D67 ERROR] ");
        Serial.println(error);
    }
}
