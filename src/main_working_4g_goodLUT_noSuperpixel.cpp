// #include <Arduino.h>
// #include <SPI.h>
// #include <FS.h>
// #include <SPIFFS.h>
// #include "debug.h"
// #include "config_gooddisplay.h"

// // Official GDEM0154I61 Driver - Based on Good Display sample code
// // This bypasses GxEPD2 and uses the exact official initialization

// #define EPD_WIDTH   200
// #define EPD_HEIGHT  200
// #define EPD_ARRAY   (EPD_WIDTH * EPD_HEIGHT / 8)

// // Pin macros adapted for ESP32 Dev Kit V1
// #define isEPD_BUSY    digitalRead(EPD_BUSY)     
// #define EPD_RST_0     digitalWrite(EPD_RST, LOW)    
// #define EPD_RST_1     digitalWrite(EPD_RST, HIGH)
// #define EPD_DC_0      digitalWrite(EPD_DC, LOW)     
// #define EPD_DC_1      digitalWrite(EPD_DC, HIGH)
// #define EPD_CS_0      digitalWrite(EPD_CS, LOW)     
// #define EPD_CS_1      digitalWrite(EPD_CS, HIGH)


// // Function declarations
// void initializePins();
// void SPI_Write(unsigned char value);
// void EPD_WriteDATA(unsigned char data);
// void EPD_WriteCMD(unsigned char command);
// void Epaper_READBUSY(void);
// void delay_xms(unsigned int xms);
// void EPD_HW_Init(void);
// void EPD_Update(void);
// void EPD_WhiteScreen_White(void);
// void EPD_WhiteScreen_Black(void);
// void EPD_DeepSleep(void);
// void testOfficialSequence(void);
// void drawSimplePattern(void);
// void debugPinStates(void);
// void testBasicCommunication(void);
// void testOfficialSequence(void);
// void testScreenWipe(void);
// // 4-gray specific
// void EPD_HW_Init_4GRAY(void);
// void EPD_Update_4GRAY(void);
// void EPD_select_LUT(const unsigned char * wave_data);

// // --- New: helpers for frame writing and 4-level grayscale (via 2x2 superpixels) ---
// static void EPD_DisplayBWBuffer(const uint8_t* buf) {
//     EPD_WriteCMD(0x24);   // Write RAM for black(0)/white(1)
//     for (uint32_t i = 0; i < EPD_ARRAY; i++) {
//         EPD_WriteDATA(buf[i]);
//     }
//     EPD_Update();
// }

// static inline void setPixelInBuffer(uint8_t* buf, int x, int y, bool black) {
//     if (x < 0 || x >= EPD_WIDTH || y < 0 || y >= EPD_HEIGHT) return;
//     uint32_t idx = (y * (EPD_WIDTH / 8)) + (x >> 3);
//     uint8_t mask = (uint8_t)(0x80u >> (x & 7));
//     if (black) {
//         buf[idx] &= (uint8_t)~mask; // black = 0
//     } else {
//         buf[idx] |= mask;          // white = 1
//     }
// }

// // Send two 1-bpp planes for native 4-gray (SSD1681):
// // plane0 = LSB (1=white, 0=black), written to 0x24
// // plane1 = MSB (1=white, 0=black), written to 0x26
// static void EPD_Display4GrayPlanes(const uint8_t* plane0, const uint8_t* plane1) {
//     // write LSB plane to 0x24
//     EPD_WriteCMD(0x24);
//     for (uint32_t i = 0; i < EPD_ARRAY; i++) EPD_WriteDATA(plane0[i]);
//     // write MSB plane to 0x26
//     EPD_WriteCMD(0x26);
//     for (uint32_t i = 0; i < EPD_ARRAY; i++) EPD_WriteDATA(plane1[i]);
//     // trigger 4-gray update (use "both planes" sequence)
//     EPD_WriteCMD(0x22);
//     EPD_WriteDATA(0xC7); // enable CLK/CP/LOAD LUT/Display Update for both planes
//     EPD_WriteCMD(0x20);
//     Epaper_READBUSY();
// }


// // //4GRAY/////////////////////////////////////////////////////
// static unsigned char LUT_DATA_4Gray[159] =    //159bytes
// {                      
// 0x40, 0x48, 0x80, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
// 0x8,  0x48, 0x10, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
// 0x2,  0x48, 0x4,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
// 0x20, 0x48, 0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
// 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
// 0xA,  0x19, 0x0,  0x3,  0x8,  0x0,  0x0,          
// 0x14, 0x1,  0x0,  0x14, 0x1,  0x0,  0x3,          
// 0xA,  0x3,  0x0,  0x8,  0x19, 0x0,  0x0,          
// 0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1,          
// 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
// 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
// 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
// 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
// 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
// 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
// 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
// 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,          
// 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x0,  0x0,  0x0,      
// 0x22, 0x17, 0x41, 0x0,  0x32, 0x1C};    

// void EPD_select_LUT(const unsigned char * wave_data) {
//     unsigned char count;
//     EPD_WriteCMD(0x32);
//     for (count = 0; count < 153 && count < sizeof(LUT_DATA_4Gray); count++) {
//         EPD_WriteDATA(wave_data[count]);
//     }
// }

// void EPD_HW_Init_4GRAY(void) {
//     // Hard reset
//     EPD_RST_0;     
//     delay_xms(10); 
//     EPD_RST_1;     
//     delay_xms(10);

//     Epaper_READBUSY();
//     EPD_WriteCMD(0x12); // soft reset
//     Epaper_READBUSY();

//     // Analog/Digital block control per Good Display sample
//     EPD_WriteCMD(0x74); EPD_WriteDATA(0x54);
//     EPD_WriteCMD(0x7E); EPD_WriteDATA(0x3B);

//     // Driver output control
//     EPD_WriteCMD(0x01);
//     EPD_WriteDATA((EPD_HEIGHT-1) & 0xFF);
//     EPD_WriteDATA(((EPD_HEIGHT-1) >> 8) & 0xFF);
//     EPD_WriteDATA(0x00);

//     EPD_WriteCMD(0x11); // data entry mode
//     EPD_WriteDATA(0x01);

//     // RAM X start/end (bytes)
//     EPD_WriteCMD(0x44);
//     EPD_WriteDATA(0x00);
//     EPD_WriteDATA((EPD_WIDTH/8 - 1) & 0xFF);

//     // RAM Y start/end (lines)
//     EPD_WriteCMD(0x45);
//     EPD_WriteDATA((EPD_HEIGHT-1) & 0xFF);
//     EPD_WriteDATA(((EPD_HEIGHT-1) >> 8) & 0xFF);
//     EPD_WriteDATA(0x00);
//     EPD_WriteDATA(0x00);

//     EPD_WriteCMD(0x3C); // Border waveform (4-gray)
//     EPD_WriteDATA(0x00);

//     // Power/voltage from LUT tail (fallback defaults if array smaller)
//     const size_t N = sizeof(LUT_DATA_4Gray);
//     uint8_t EOPQ = (N >= 154) ? LUT_DATA_4Gray[153] : 0x22;
//     uint8_t VGH  = (N >= 155) ? LUT_DATA_4Gray[154] : 0x17;
//     uint8_t VSH1 = (N >= 156) ? LUT_DATA_4Gray[155] : 0x41;
//     uint8_t VSH2 = (N >= 157) ? LUT_DATA_4Gray[156] : 0x00;
//     uint8_t VSL  = (N >= 158) ? LUT_DATA_4Gray[157] : 0x32;
//     uint8_t VCOM = (N >= 159) ? LUT_DATA_4Gray[158] : 0x1C;

//     EPD_WriteCMD(0x2C); // VCOM
//     EPD_WriteDATA(VCOM);

//     EPD_WriteCMD(0x3F); // EOPQ
//     EPD_WriteDATA(EOPQ);

//     EPD_WriteCMD(0x03); // VGH
//     EPD_WriteDATA(VGH);

//     EPD_WriteCMD(0x04); // VSH1, VSH2, VSL
//     EPD_WriteDATA(VSH1);
//     EPD_WriteDATA(VSH2);
//     EPD_WriteDATA(VSL);

//     // Load LUT
//     EPD_select_LUT(LUT_DATA_4Gray);

//     // Set RAM address counters to origin
//     EPD_WriteCMD(0x4E); EPD_WriteDATA(0x00);
//     EPD_WriteCMD(0x4F); EPD_WriteDATA((EPD_HEIGHT-1) & 0xFF);
//     EPD_WriteDATA(((EPD_HEIGHT-1) >> 8) & 0xFF);
//     Epaper_READBUSY();
// }

// void EPD_Update_4GRAY(void) {
//     EPD_WriteCMD(0x22);
//     EPD_WriteDATA(0xC7);
//     EPD_WriteCMD(0x20);
//     Epaper_READBUSY();
// }

// // Render a native 4-level grayscale demo using two bitplanes on SSD1681
// // Levels: 0=white, 1=light gray, 2=dark gray, 3=black
// static void drawGrayscale4LevelDemo_Superpixel() {
//     DEBUG_INFO("Drawing native 4-level grayscale (no superpixels)...");

//     // Build a horizontal gradient in a 2-bpp buffer (packed 4 pixels/byte)
//     static uint8_t gray2bpp[EPD_WIDTH * EPD_HEIGHT / 4];
//     memset(gray2bpp, 0x00, sizeof(gray2bpp));

//     for (int y = 0; y < EPD_HEIGHT; ++y) {
//         for (int x = 0; x < EPD_WIDTH; ++x) {
//             int level = (x * 4) / (EPD_WIDTH - 1); // 0..3 left->right
//             if (level < 0) level = 0; if (level > 3) level = 3;
//             int pi = y * EPD_WIDTH + x;
//             int bi = pi >> 2;                 // 4 pixels per byte
//             int sh = (3 - (pi & 3)) * 2;      // top bits first
//             gray2bpp[bi] = (gray2bpp[bi] & ~(0x3 << sh)) | ((level & 0x3) << sh);
//         }
//     }

//     // Convert 2-bpp to two 1-bpp planes (1=white, 0=black)
//     static uint8_t plane0[EPD_ARRAY]; // LSB
//     static uint8_t plane1[EPD_ARRAY]; // MSB
//     memset(plane0, 0xFF, sizeof(plane0));
//     memset(plane1, 0xFF, sizeof(plane1));

//     for (int y = 0; y < EPD_HEIGHT; ++y) {
//         for (int x = 0; x < EPD_WIDTH; ++x) {
//             int pi = y * EPD_WIDTH + x;
//             int bi = pi >> 2;
//             int sh = (3 - (pi & 3)) * 2;
//             uint8_t level = (gray2bpp[bi] >> sh) & 0x3;
//             // Plane bits are white-active; invert level bits
//             bool p0_white = ((level & 0x1) == 0); // LSB
//             bool p1_white = ((level & 0x2) == 0); // MSB
//             setPixelInBuffer(plane0, x, y, !p0_white); // setPixelInBuffer expects black=true
//             setPixelInBuffer(plane1, x, y, !p1_white);
//         }
//     }

//     // Push both planes and update (LUT already loaded by EPD_HW_Init_4GRAY)
//     EPD_Display4GrayPlanes(plane0, plane1);
//     DEBUG_INFO("Native 4-level grayscale demo complete");
// }

// // --- New: richer 4-gray showcase with shapes, swatches, stripes, and checkerboard ---
// static inline void set2bppLevel(uint8_t* buf, int x, int y, uint8_t level) {
//     if ((unsigned)x >= EPD_WIDTH || (unsigned)y >= EPD_HEIGHT) return;
//     level &= 0x3;
//     int pi = y * EPD_WIDTH + x;
//     int bi = pi >> 2;            // 4 pixels/byte
//     int sh = (3 - (pi & 3)) * 2; // big-endian within byte
//     buf[bi] = (uint8_t)((buf[bi] & ~(0x3 << sh)) | (level << sh));
// }
// static inline uint8_t get2bppLevel(const uint8_t* buf, int x, int y) {
//     int pi = y * EPD_WIDTH + x;
//     int bi = pi >> 2;
//     int sh = (3 - (pi & 3)) * 2;
//     return (uint8_t)((buf[bi] >> sh) & 0x3);
// }
// static inline void set2bppLevelMax(uint8_t* buf, int x, int y, uint8_t newLevel) {
//     uint8_t cur = get2bppLevel(buf, x, y);
//     if (newLevel > cur) set2bppLevel(buf, x, y, newLevel); // darker wins (0..3)
// }
// static void send2bppTo4Gray(const uint8_t* gray2bpp) {
//     static uint8_t plane0[EPD_ARRAY]; // LSB
//     static uint8_t plane1[EPD_ARRAY]; // MSB
//     memset(plane0, 0xFF, sizeof(plane0));
//     memset(plane1, 0xFF, sizeof(plane1));
//     for (int y = 0; y < EPD_HEIGHT; ++y) {
//         for (int x = 0; x < EPD_WIDTH; ++x) {
//             int pi = y * EPD_WIDTH + x;
//             int bi = pi >> 2;
//             int sh = (3 - (pi & 3)) * 2;
//             uint8_t level = (gray2bpp[bi] >> sh) & 0x3; // 0..3 (white..black)
//             bool p0_white = ((level & 0x1) == 0);
//             bool p1_white = ((level & 0x2) == 0);
//             setPixelInBuffer(plane0, x, y, !p0_white);
//             setPixelInBuffer(plane1, x, y, !p1_white);
//         }
//     }
//     EPD_Display4GrayPlanes(plane0, plane1);
// }
// static void drawShowcase4Gray() {
//     DEBUG_INFO("Drawing 4-gray showcase pattern...");
//     static uint8_t gray2bpp[EPD_WIDTH * EPD_HEIGHT / 4];
//     memset(gray2bpp, 0x00, sizeof(gray2bpp));

//     // Background light gray
//     for (int y = 0; y < EPD_HEIGHT; ++y) {
//         for (int x = 0; x < EPD_WIDTH; ++x) set2bppLevel(gray2bpp, x, y, 1);
//     }

//     // Top swatches: 4 levels
//     int bandH = 24; int swW = EPD_WIDTH / 4;
//     for (int i = 0; i < 4; ++i) {
//         for (int y = 0; y < bandH; ++y) {
//             for (int x = i*swW; x < (i+1)*swW; ++x) set2bppLevel(gray2bpp, x, y, (uint8_t)i);
//         }
//     }

//     // Concentric diamond rings from center using Manhattan distance
//     int cx = EPD_WIDTH/2, cy = EPD_HEIGHT/2;
//     for (int y = 0; y < EPD_HEIGHT; ++y) {
//         int dy = abs(y - cy);
//         for (int x = 0; x < EPD_WIDTH; ++x) {
//             int dx = abs(x - cx);
//             int d = dx + dy;                 // diamond metric
//             uint8_t ring = (uint8_t)((d / 8) & 3); // 0..3 repeating
//             set2bppLevelMax(gray2bpp, x, y, ring);
//         }
//     }

//     // Diagonal hatch overlay (both directions) - dark gray
//     for (int y = 0; y < EPD_HEIGHT; ++y) {
//         for (int x = 0; x < EPD_WIDTH; ++x) {
//             if (((x + y) % 20) == 0 || ((x - y + 1000) % 20) == 0) {
//                 set2bppLevelMax(gray2bpp, x, y, 2);
//             }
//         }
//     }

//     // Bottom-right checkerboard block
//     int bx = 116, by = 116, bw = 84, bh = 84, sq = 10;
//     for (int y = by; y < by + bh; ++y) {
//         for (int x = bx; x < bx + bw; ++x) {
//             int cxq = (x - bx) / sq;
//             int cyq = (y - by) / sq;
//             uint8_t lv = ((cxq + cyq) & 1) ? 3 : 0; // black/white
//             set2bppLevel(gray2bpp, x, y, lv);
//         }
//     }

//     // Central filled circle (black) and outer ring (dark gray)
//     int rFill = 28, rRing = 60, ringTh = 3;
//     for (int y = 0; y < EPD_HEIGHT; ++y) {
//         int dy = y - cy;
//         for (int x = 0; x < EPD_WIDTH; ++x) {
//             int dx = x - cx;
//             int d2 = dx*dx + dy*dy;
//             if (d2 <= rFill*rFill) {
//                 set2bppLevelMax(gray2bpp, x, y, 3);
//             } else {
//                 int rd = (int)sqrt((float)d2); // ok once per pixel for 200x200
//                 if (abs(rd - rRing) <= ringTh) set2bppLevelMax(gray2bpp, x, y, 2);
//             }
//         }
//     }

//     // Black border
//     for (int x = 0; x < EPD_WIDTH; ++x) {
//         set2bppLevel(gray2bpp, x, 0, 3);
//         set2bppLevel(gray2bpp, x, EPD_HEIGHT-1, 3);
//     }
//     for (int y = 0; y < EPD_HEIGHT; ++y) {
//         set2bppLevel(gray2bpp, 0, y, 3);
//         set2bppLevel(gray2bpp, EPD_WIDTH-1, y, 3);
//     }

//     // Send to display
//     send2bppTo4Gray(gray2bpp);
//     DEBUG_INFO("4-gray showcase complete");
// }

// // Path to 2-bpp RAW image in SPIFFS (place file in `data/` and upload FS)
// static const char* kImagePath = "/noise_200x200_2bpp.bin"; // 10,000 bytes, 200x200, 2-bpp, row-major, 4 px/byte

// // Preview helpers
// static uint32_t crc32_update(uint32_t crc, const uint8_t* data, size_t len) {
//     crc = ~crc;
//     for (size_t i = 0; i < len; ++i) {
//         uint32_t c = (crc ^ data[i]) & 0xFFu;
//         for (int b = 0; b < 8; ++b) c = (c >> 1) ^ (0xEDB88320u & (~(c & 1) + 1));
//         crc = (crc >> 8) ^ c;
//     }
//     return ~crc;
// }
// static void dumpHex(const uint8_t* data, size_t len, size_t maxBytes = 128) {
//     size_t n = len < maxBytes ? len : maxBytes;
//     for (size_t i = 0; i < n; i += 16) {
//         char line[3*16 + 1]; size_t p = 0;
//         size_t m = (n - i) < 16 ? (n - i) : 16;
//         for (size_t j = 0; j < m; ++j) {
//             p += snprintf(line + p, sizeof(line) - p, "%02X ", data[i + j]);
//         }
//         DEBUG_INFO("%04u: %s", (unsigned)(i), line);
//     }
//     if (n < len) DEBUG_INFO("... (%u bytes omitted)", (unsigned)(len - n));
// }

// // SPIFFS loader for 200x200 2-bpp RAW
// static bool load2bppFromSPIFFS(const char* path, uint8_t* outBuf, size_t expectLen) {
//     if (!SPIFFS.begin(true)) {
//         DEBUG_ERROR("SPIFFS mount failed");
//         return false;
//     }
//     File f = SPIFFS.open(path, "r");
//     if (!f) {
//         DEBUG_ERROR("File not found: %s", path);
//         // Try to find any .bin as fallback
//         File root = SPIFFS.open("/");
//         File file = root.openNextFile();
//         while (file) {
//             DEBUG_INFO("SPIFFS: %s (%u bytes)", file.name(), (unsigned)file.size());
//             file = root.openNextFile();
//         }
//         return false;
//     }
//     size_t sz = f.size();
//     DEBUG_INFO("Reading %s (%u bytes)", path, (unsigned)sz);
//     if (sz != expectLen) {
//         DEBUG_WARN("Unexpected size: got %u, expected %u", (unsigned)sz, (unsigned)expectLen);
//         if (sz < expectLen) {
//             memset(outBuf, 0x00, expectLen);
//         }
//     }
//     size_t toRead = sz < expectLen ? sz : expectLen;
//     size_t n = f.read(outBuf, toRead);
//     f.close();
//     if (n != toRead) {
//         DEBUG_ERROR("Read failed: %u/%u", (unsigned)n, (unsigned)toRead);
//         return false;
//     }
//     // Preview to Serial
//     dumpHex(outBuf, toRead, 128);
//     uint32_t crc = crc32_update(0, outBuf, toRead);
//     DEBUG_INFO("CRC32: 0x%08lX", (unsigned long)crc);
//     return true;
// }

// void setup() {
//     Serial.begin(115200);
//     delay(2000);


//     DEBUG_INFO("Official test complete - display should show activity");
//     printSeparator("GDEM0154I61 4-Gray Image Loader");
//     printDebugInfo();

//     // Initialize GPIO and SPI
//     initializePins();
//     // Test 4: 4-level grayscale showcase
//     DEBUG_INFO("Test 4: 4-level grayscale showcase (shapes + swatches)");


    
    
//     // Prepare display for native 4-gray
//     EPD_HW_Init_4GRAY();

//     // testOfficialSequence();
//     // delay(2000);


//     // Buffer for 200x200 2-bpp RAW
//     static uint8_t raw2bpp[EPD_WIDTH * EPD_HEIGHT / 4]; // 10,000 bytes

//     // Load from SPIFFS and display
//     if (load2bppFromSPIFFS(kImagePath, raw2bpp, sizeof(raw2bpp))) {
//         testScreenWipe();
//         delay(2000);
//         DEBUG_INFO("Displaying 2-bpp image -> 4-gray planes");
//         EPD_HW_Init_4GRAY();
//         send2bppTo4Gray(raw2bpp);
//         DEBUG_INFO("Display update done");
//     } else {
//         DEBUG_ERROR("Failed to load image; drawing fallback 4-gray showcase");
//         drawShowcase4Gray();
//     }

//     // Optional: deep sleep display controller
//     EPD_DeepSleep();

//     // delay(2000);
//     // DEBUG_INFO("Setup complete");

//     // EPD_HW_Init(); // Re-init to 1-bpp mode for any further tests
//     // EPD_HW_Init_4GRAY(); // Re-init to 4-gray mode
//     // drawShowcase4Gray(); // Draw showcase again
//     // EPD_DeepSleep(); // Deep sleep again
    
// }

// void loop() {
//     // Nothing to do. Image shown once at boot.
//     delay(1000);
// }

// void initializePins() {
//     DEBUG_INFO("Initializing GDEM0154I61 pins...");
    
//     // Set pin modes
//     pinMode(EPD_BUSY, INPUT);   
//     pinMode(EPD_RST, OUTPUT);   
//     pinMode(EPD_DC, OUTPUT);    
//     pinMode(EPD_CS, OUTPUT);    
    
//     // Initialize SPI with official settings
//     SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
//     SPI.begin();
    
//     // Set initial states
//     EPD_CS_1;  // CS high
//     EPD_RST_1; // RST high
    
//     DEBUG_INFO("Pins initialized with official SPI settings");
// }

// // SPI Communication (EXACT copy from official sample)
// void SPI_Write(unsigned char value) {
//     SPI.transfer(value);
// }

// void EPD_WriteDATA(unsigned char data) {
//     EPD_CS_0;
//     EPD_DC_1;  // DC = 1 for data
//     SPI_Write(data);
//     EPD_CS_1;
// }

// void EPD_WriteCMD(unsigned char command) {
//     EPD_CS_0;
//     EPD_DC_0;  // DC = 0 for command
//     SPI_Write(command);
//     EPD_CS_1;
// }

// void Epaper_READBUSY(void) {
//     while(1) {
//         if(isEPD_BUSY == 0) break;  // Wait until BUSY goes LOW
//     }
// }

// void delay_xms(unsigned int xms) {
//     delay(xms);  // At least 10ms delay
// }

// // EXACT COPY of official EPD_HW_Init from sample code
// void EPD_HW_Init(void) {
//     DEBUG_INFO("EPD_HW_Init: Official Good Display initialization");
    
//     EPD_RST_0;  // Module reset
//     delay_xms(10); // At least 10ms delay
//     EPD_RST_1;
//     delay_xms(10); // At least 10ms delay
    
//     Epaper_READBUSY();
//     EPD_WriteCMD(0x12);  // SWRESET
//     Epaper_READBUSY();
    
//     EPD_WriteCMD(0x01); // Driver output control
//     EPD_WriteDATA((EPD_HEIGHT-1) % 256);
//     EPD_WriteDATA((EPD_HEIGHT-1) / 256);
//     EPD_WriteDATA(0x00);
    
//     EPD_WriteCMD(0x11); // Data entry mode
//     EPD_WriteDATA(0x01);
    
//     EPD_WriteCMD(0x44); // Set Ram-X address start/end position
//     EPD_WriteDATA(0x00);
//     EPD_WriteDATA(EPD_WIDTH/8 - 1);
    
//     EPD_WriteCMD(0x45); // Set Ram-Y address start/end position
//     EPD_WriteDATA((EPD_HEIGHT-1) % 256);
//     EPD_WriteDATA((EPD_HEIGHT-1) / 256);
//     EPD_WriteDATA(0x00);
//     EPD_WriteDATA(0x00);
    
//     EPD_WriteCMD(0x3C); // BorderWaveform
//     EPD_WriteDATA(0x05);
    
//     EPD_WriteCMD(0x18); // Read built-in temperature sensor
//     EPD_WriteDATA(0x80);
    
//     EPD_WriteCMD(0x4E); // Set RAM x address count to 0
//     EPD_WriteDATA(0x00);
//     EPD_WriteCMD(0x4F); // Set RAM y address count
//     EPD_WriteDATA((EPD_HEIGHT-1) % 256);
//     EPD_WriteDATA((EPD_HEIGHT-1) / 256);
//     Epaper_READBUSY();
    
//     DEBUG_INFO("Official EPD_HW_Init complete");
// }

// // EXACT COPY of official update function
// void EPD_Update(void) {
//     EPD_WriteCMD(0x22); // Display Update Control
//     EPD_WriteDATA(0xF7);
//     EPD_WriteCMD(0x20); // Activate Display Update Sequence
//     Epaper_READBUSY();
// }

// // EXACT COPY of official white screen function
// void EPD_WhiteScreen_White(void) {
//     DEBUG_INFO("EPD_WhiteScreen_White: Official white screen");
//     unsigned int i;
//     EPD_WriteCMD(0x24);   // Write RAM for black(0)/white(1)
//     for(i = 0; i < EPD_ARRAY; i++) {
//         EPD_WriteDATA(0xFF);  // 0xFF = white
//     }
//     EPD_Update();
//     DEBUG_INFO("White screen complete");
// }

// // EXACT COPY of official black screen function
// void EPD_WhiteScreen_Black(void) {
//     DEBUG_INFO("EPD_WhiteScreen_Black: Official black screen");
//     unsigned int i;
//     EPD_WriteCMD(0x24);   // Write RAM for black(0)/white(1)
//     for(i = 0; i < EPD_ARRAY; i++) {
//         EPD_WriteDATA(0x00);  // 0x00 = black
//     }
//     EPD_Update();
//     DEBUG_INFO("Black screen complete");
// }

// // EXACT COPY of official deep sleep function
// void EPD_DeepSleep(void) {
//     EPD_WriteCMD(0x10); // Enter deep sleep
//     EPD_WriteDATA(0x01);
//     delay_xms(100);
//     DEBUG_INFO("Display in deep sleep mode");
// }

// // Draw a simple test pattern
// void drawSimplePattern(void) {
//     DEBUG_INFO("Drawing simple test pattern...");
//     unsigned int i;
//     EPD_WriteCMD(0x24);   // Write RAM
    
//     // Create a simple pattern: border and cross
//     for(i = 0; i < EPD_ARRAY; i++) {
//         unsigned int x = i % (EPD_WIDTH / 8);
//         unsigned int y = i / (EPD_WIDTH / 8);
        
//         unsigned char pattern = 0xFF;  // Start with white
        
//         // Black border (first/last rows and columns)
//         if (y < 2 || y >= (EPD_HEIGHT - 2) || x < 1 || x >= (EPD_WIDTH/8 - 1)) {
//             pattern = 0x00;  // Black border
//         }
//         // Cross pattern in middle
//         else if (y == EPD_HEIGHT/2 || x == (EPD_WIDTH/8)/2) {
//             pattern = 0x00;  // Black cross
//         }
//         // Checker pattern in corners
//         else if ((y < 20 && x < 3) || (y < 20 && x > EPD_WIDTH/8-4)) {
//             pattern = ((x + y/8) % 2) ? 0x00 : 0xFF;
//         }
        
//         EPD_WriteDATA(pattern);
//     }
    
//     EPD_Update();
//     DEBUG_INFO("Test pattern complete");
// }

// void testScreenWipe(void) {

    
//     DEBUG_INFO("Test 2: Full black screen");
//     uint32_t startTime = millis();
//     EPD_HW_Init();
//     EPD_WhiteScreen_Black();
//     uint32_t blackTime = millis() - startTime;
//     DEBUG_INFO("Black screen took: %lu ms", blackTime);
//     EPD_DeepSleep();
//     delay(2000);

//     DEBUG_INFO("Test 1: Full white screen");
//      startTime = millis();
//     EPD_HW_Init();
//     EPD_WhiteScreen_White();
//     uint32_t whiteTime = millis() - startTime;
//     DEBUG_INFO("White screen took: %lu ms", whiteTime);
//     EPD_DeepSleep();
//     delay(2000);
// }


// // Official test sequence from Good Display sample
// void testOfficialSequence(void) {
//     DEBUG_INFO("=== Official GDEM0154I61 Test Sequence ===");
    
//     testScreenWipe();
    
//     // // Test 3: Simple pattern
//     // DEBUG_INFO("Test 3: Simple test pattern");
//     // startTime = millis();
//     // EPD_HW_Init();
//     // drawSimplePattern();
//     // uint32_t patternTime = millis() - startTime;
//     // DEBUG_INFO("Pattern took: %lu ms", patternTime);
//     // EPD_DeepSleep();
//     // delay(2000);

//     // Test 4: 4-level grayscale showcase
//     DEBUG_INFO("Test 4: 4-level grayscale showcase (shapes + swatches)");
//     EPD_HW_Init_4GRAY(); // Use 4-gray initialization with LUT
//     drawShowcase4Gray();
//     EPD_DeepSleep();
//     delay(5000);
    
//     testScreenWipe(); // Final wipe before ending
// }
