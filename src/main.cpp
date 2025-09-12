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


// 
#define EPD_BUSY    5     // GPIO5  - BUSY (first in cable order)
#define EPD_RST     4     // GPIO4  - RES  (Reset)
#define EPD_DC      2     // GPIO2  - D/C  (Data/Command)  
#define EPD_CS      15    // GPIO15 - CS   (Chip Select)
#define EPD_SCK     18    // GPIO18 - SCK  (SPI Clock)
#define EPD_SDI     23    // GPIO23 - SDI  (SPI Data)
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
}

/**
 * Arduino main loop - runs continuously
 */
void loop() {

    static File binFiles[16];
    static int numFiles = 0;
    static int currentFile = 0;
    static bool filesListed = false;
    static unsigned long lastSwitch = 0;

    if (!filesListed) {
        File root = SPIFFS.open("/");
        if (!root || !root.isDirectory()) {
            Serial.println("/ directory not found in SPIFFS!");
            while (1) delay(1000);
        }
        File file = root.openNextFile();
        while (file && numFiles < 16) {
            String name = file.name();
            if (name.endsWith(".bin")) {
                // Ensure absolute path
                if (!name.startsWith("/")) name = "/" + name;
                binFiles[numFiles++] = SPIFFS.open(name, "r");
                Serial.print("Found bin: "); Serial.println(name);
            }
            file = root.openNextFile();
        }
        filesListed = true;
        if (numFiles == 0) {
            Serial.println("No .bin files found in /data!");
            while (1) delay(1000);
        }
        lastSwitch = millis();
    }

    unsigned long now = millis();
    if (now - lastSwitch < 1000) return;
    lastSwitch = now;

    // Read and display the current .bin file
    File& f = binFiles[currentFile];
        if (!f) {
            Serial.print("File handle invalid for: ");
            Serial.println(f.name());
            currentFile = (currentFile + 1) % numFiles;
            return;
        }
        size_t fsize = f.size();
        Serial.print("File: "); Serial.print(f.name()); Serial.print(", size: "); Serial.println(fsize);
        if (fsize < 16) {
            Serial.println("File too small for header!");
            currentFile = (currentFile + 1) % numFiles;
            return;
        }
    f.seek(0);
    int32_t x, y, w, h;
    uint8_t header[16];
    int readBytes = f.read(header, 16);
    if (readBytes != 16) {
        Serial.println("Failed to read header!");
        currentFile = (currentFile + 1) % numFiles;
        return;
    }
    x = header[0] | (header[1]<<8) | (header[2]<<16) | (header[3]<<24);
    y = header[4] | (header[5]<<8) | (header[6]<<16) | (header[7]<<24);
    w = header[8] | (header[9]<<8) | (header[10]<<16) | (header[11]<<24);
    h = header[12] | (header[13]<<8) | (header[14]<<16) | (header[15]<<24);
    Serial.print("Header for "); Serial.print(f.name());
    Serial.print(": x="); Serial.print(x);
    Serial.print(", y="); Serial.print(y);
    Serial.print(", w="); Serial.print(w);
    Serial.print(", h="); Serial.println(h);
    if (w <= 0 || h <= 0 || x < 0 || y < 0 || x + w > 200 || y + h > 200) {
        Serial.println("Invalid region in bin file!");
        currentFile = (currentFile + 1) % numFiles;
        return;
    }
    int bytes = ((w * h) + 7) / 8;
    static uint8_t regionBuf[200*200/8];
    if (bytes > sizeof(regionBuf)) {
        Serial.println("Region too large!");
        currentFile = (currentFile + 1) % numFiles;
        return;
    }
    f.seek(16);
    f.read(regionBuf, bytes);
    display.updatePartialRegion(x, 200-y, regionBuf, w, h);
    // display.updatePartialRegion(0, 200, regionBuf, w, h);
    // delay(1000); // Small delay to ensure display processes commands
    // display.updatePartialRegion(200-w, 200, regionBuf, w, h);
    // delay(2000); // Small delay to ensure display processes commands
    // display.updatePartialRegion(200-w, 0+h, regionBuf, w, h);
    // delay(4000); // Small delay to ensure display processes commands    
    // display.updatePartialRegion(0, 0+h, regionBuf, w, h);

    Serial.print("Displayed: "); Serial.println(f.name());
    currentFile = (currentFile + 1) % numFiles;

    // End of new animation logic. Remove unreachable legacy code below.
    return;
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
    // Serial.print(minutes_low);
    // Serial.print(":");
    // Serial.print(seconds_high);
    // Serial.println(seconds_low);
    
    // if (!display.updateMultipleRegions(regions)) {
    //     Serial.print("ERROR: Failed to update regions - ");
    //     Serial.println(display.getLastError());
    // }
    
    // // Increment time counters (simulate clock progression)
    // seconds_low++;
    // if (seconds_low >= 10) {
    //     seconds_low = 0;
    //     seconds_high++;
    //     if (seconds_high >= 6) {
    //         seconds_high = 0;
    //         minutes_low++;
    //         if (minutes_low >= 10) {
    //             minutes_low = 0;
    //             minutes_high++;
    //             if (minutes_high >= 6) {
    //                 // After 59:59, perform a full refresh to clear any ghosting
    //                 minutes_high = 0;
    //                 Serial.println("\\n=== Performing full refresh to clear ghosting ===");
    //                 display.initializeMonochrome();
    //                 display.clearScreen();
    //                 display.setPartialRefreshBase(gImage_basemap);
    //                 Serial.println("=== Full refresh completed, resuming clock ===\\n");
    //             }
    //         }
    //     }
    // }
    
    // // Perform periodic full refresh every 100 updates to prevent ghosting
    // // This is recommended best practice for e-paper displays
    // if (update_count % 100 == 0) {
    //     Serial.println("\\n--- Periodic maintenance refresh ---");
    //     display.initializeMonochrome();
    //     display.setPartialRefreshBase(gImage_basemap);
    //     Serial.println("--- Maintenance refresh completed ---\\n");
    // }
    
    // // Optional: Stop after a certain number of updates for demo purposes
    // if (update_count >= 300) {  // Run for 5 minutes then stop
    //     Serial.println("\\n=== Demo completed after 5 minutes ===");
    //     Serial.println("Entering final deep sleep...");
    //     display.enterDeepSleep();
    //     Serial.println("Demo finished. Reset to run again.");
        
    //     // Infinite loop to stop execution
    //     while (1) {
    //         delay(10000);
    //     }
    // }
}

/**
 * Emergency error handler
 */
void handleError(const char* error_message) {
    Serial.println("\\n!!! CRITICAL ERROR !!!");
    Serial.print("Error: ");
    Serial.println(error_message);
    Serial.print("Display Error: ");
    Serial.println(display.getLastError());
    
    // Attempt to safely shut down display
    display.enterDeepSleep();
    
    Serial.println("System halted. Please reset to try again.");
    
    // Flash built-in LED to indicate error state
    pinMode(LED_BUILTIN, OUTPUT);
    while (1) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
    }
}

/**
 * Print system information
 */
void printSystemInfo() {
    Serial.println("\\n=== System Information ===");
    Serial.print("Free heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    
    Serial.print("Chip Model: ");
    Serial.println(ESP.getChipModel());
    Serial.print("Chip Revision: ");
    Serial.println(ESP.getChipRevision());
    
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
