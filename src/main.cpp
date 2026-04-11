/**
 * @file main.cpp
 * @brief Main application for BMO E-Paper Display Demo
 * 
 * This file contains the main application logic that demonstrates the capabilities
 * of the GDEH0154D67 e-paper display controller. It includes examples of:
 * - Full screen monochrome display
 * - 4-grayscale image display  
 * - Partial refresh for digital clock display
 * - Proper power management and sleep cycles
 * 
 * @author Generated from manufacturer demo code
 * @date 2025
 * @version 1.0
 */


#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include "GDEH0154D67_Display.h"
#include "emotion_manager.h"
#include "serial_command.h"

// (Removed hardcoded boot-time burn loop; maintenance burn can be
// triggered manually over serial with the `BURN <n>` command.)


// GPIO Pin Configuration for Seeed XIAO ESP32-C3
// Using hardware SPI pins for proper communication
#define EPD_BUSY    D1     // GPIO3  - BUSY
#define EPD_RST     D0     // GPIO2  - RES  (Reset)
#define EPD_DC      D2     // GPIO4  - D/C  (Data/Command)  
#define EPD_CS      D3     // GPIO5  - CS   (Chip Select)
#define EPD_SCK     D8     // GPIO8  - SCK  (Hardware SPI Clock)
#define EPD_SDI     D10    // GPIO10 - MOSI (Hardware SPI Data)
// GND  = GND
// 3.3V = 3.3V

// Create display controller with default pin assignments:
// BUSY=5, RST=4, DC=2, CS=15, SCK=18, SDI=23
GDEH0154D67_Display display(EPD_BUSY, EPD_RST, EPD_DC, EPD_CS, EPD_SCK, EPD_SDI);

/*
 * PARTIAL REFRESH RATE GUIDELINES:
 * - 50ms (20 FPS): Maximum for smooth animation, but may stress display
 * - 100ms (10 FPS): Excellent for responsive emotions and expressions
 * - 200ms (5 FPS): Good balance for animated BMO faces
 * - 500ms (2 FPS): Conservative for digital clocks
 * - 1000ms (1 FPS): Very safe for simple status displays
 * 
 * Note: E-paper displays can typically handle 100+ partial refreshes
 * before needing a full refresh to clear ghosting artifacts.
 */

// Demo state variables
static bool first_run = true;

static unsigned long last_update_time = 0;
static const unsigned long UPDATE_INTERVAL = 1000;  // Update every 100ms (10 FPS) - much faster for BMO emotions!

// State variable to track sequential partial updates
static int partial_update_count = 0;
static const int PARTIAL_UPDATE_THRESHOLD = 5;

/**
 * Arduino setup function - runs once at startup
 */
void setup() {
    // Initialize serial communication for debug output
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        // Wait up to 3 seconds for serial connection (for debugging)
    }
    
    Serial.println("=== BMO E-Paper Display Demo Starting ===");
    Serial.println("Display: GDEH0154D67 1.54\" E-Ink");
    Serial.println("Resolution: 200x200 pixels");
    Serial.println("==========================================");
    
    // Enable debug output from display controller
    display.setDebugMode(true);

    // Initialize GPIO pins for display communication
    display.initializePins();

    Serial.println("\n--- Phase 4: Partial Refresh Demo Starting ---");
    Serial.println("BMO Face Animation from SPIFFS .bin files");

    // Mount SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed!");
        while (1) delay(1000);
    }

    // Create a white base image (all pixels white)
    static uint8_t white_base[5000]; // 200x200 / 8 = 5000 bytes
    memset(white_base, 0xFF, sizeof(white_base));
    display.initializeMonochrome();
    display.clearScreen();
    display.refreshFull();
    display.setPartialRefreshBase(white_base); // Use white canvas as base

    Serial.println("Setup completed successfully!");
    Serial.println("Starting main loop with .bin face animation...");
    last_update_time = millis();

// Hard burn removed; use serial `BURN <n>` if needed.
}

/**
 * Arduino main loop - runs continuously
 */
void loop() {
    // New behavior: wait for serial commands from the host to display images.
    // Supported commands:
    //  - LIST               : list available .bin files
    //  - SHOW <filename>    : display the named .bin file (exact filename or without leading /)
    //  - SHOWIDX <n>        : display file by index from LIST (0-based)
    //  - BURN <n>           : perform n full refresh cycles (maintenance)
    // Any other input is ignored.

    
    // // Total width needed: 5*32 = 160 pixels, centered on 200px display
    // // Starting X position: (200-160)/2 = 20 pixels
    // // Y position: centered vertically (200-64)/2 = 68 pixels
    // regions[4] = PartialRegion(80,  32, 64, 32, Num[seconds_low]); // Seconds tens
    // regions[3] = PartialRegion(80,  64, 64, 32, Num[seconds_high]);  // Seconds ones
    // regions[2] = PartialRegion(80,  96, 64, 32, gImage_numdot);     // Colon separator
    // regions[1] = PartialRegion(80,  128, 64, 32, Num[minutes_low]); // Minutes ones
    // regions[0] = PartialRegion(80,  160, 64, 32, Num[minutes_high]);  // Minutes tens

    // // Update all regions simultaneously
    // Serial.print("Clock update #");
    // Serial.print(update_count);
    // Serial.print(" - Time: ");
    // Serial.print(minutes_high);
    static EmotionManager manager(&display);
    static SerialCommand serCmd;
    static bool initialized = false;
    if (!initialized) {
        manager.begin();
        serCmd.begin(&Serial);

        // Register commands under a modular interface
        serCmd.registerCommand("EMOTION.LIST", [&](const String& a){
            Serial.println("FILELIST_START");
            for (int i = 0; i < manager.count(); ++i) {
                Serial.print(i); Serial.print(": "); Serial.println(manager.nameAt(i));
            }
            Serial.println("FILELIST_END");
        });

        serCmd.registerCommand("EMOTION.SET", [&](const String& a){
            if (a.length() == 0) { serCmd.reply("ERR missing arg"); return; }
            if (manager.showByName(a)) serCmd.reply(String("OK " ) + a);
            else serCmd.reply(String("ERR show failed: ") + a);
        });

        serCmd.registerCommand("EMOTION.SETIDX", [&](const String& a){
            int idx = a.toInt();
            if (manager.showByIndex(idx)) serCmd.reply(String("OK idx ") + String(idx));
            else serCmd.reply(String("ERR idx ") + String(idx));
        });

        serCmd.registerCommand("STATUS", [&](const String& a){
            serCmd.reply(String("HEAP:") + String(ESP.getFreeHeap()) + " FILES:" + String(manager.count()));
        });

        serCmd.registerCommand("PING", [&](const String& a){ serCmd.reply("PONG"); });

        // Legacy/BURN command
        serCmd.registerCommand("BURN", [&](const String& a){
            int cnt = a.toInt(); if (cnt <= 0) cnt = 1;
            serCmd.reply(String("BURN starting ") + String(cnt));
            for (int i = 0; i < cnt; ++i) {
                display.initializeMonochrome();
                display.clearScreen();
                static uint8_t white_base_buf[5000];
                memset(white_base_buf, 0xFF, sizeof(white_base_buf));
                display.setPartialRefreshBase(white_base_buf);
                display.refreshFull();
                delay(1200);
            }
            serCmd.reply("BURN done");
        });

        initialized = true;
        Serial.print("EmotionManager initialized, files: ");
        Serial.println(manager.count());
    }

    serCmd.process();
    delay(10);
    Serial.print("CPU Frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");
    
    Serial.println("Display Configuration:");
    Serial.print("  Width: ");
    Serial.print(display.getWidth());
    Serial.println(" pixels");
    Serial.print("  Height: ");
    Serial.print(display.getHeight());
    Serial.println(" pixels");
    Serial.print("  Mono buffer size: ");
    Serial.print(display.getMonoBufferSize());
    Serial.println(" bytes");
    Serial.print("  Gray buffer size: ");
    Serial.print(display.getGrayBufferSize());
    Serial.println(" bytes");
    
    Serial.println("=========================\\n");
}
