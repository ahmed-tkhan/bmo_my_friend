// /**
//  * @file main.cpp
//  * @brief Main application for BMO E-Paper Display Demo
//  * 
//  * This file contains the main application logic that demonstrates the capabilities
//  * of the GDEH0154D67 e-paper display controller. It includes examples of:
//  * - Full screen monochrome display
//  * - 4-grayscale image display  
//  * - Partial refresh for digital clock display
//  * - Proper power management and sleep cycles
//  * 
//  * @author Generated from manufacturer demo code
//  * @date 2025
//  * @version 1.0
//  */

// #include <Arduino.h>
// #include "GDEH0154D67_Display.h"
// #include "Ap_29demo.h"  // Contains image data and digit arrays

// // Create display controller with default pin assignments:
// // BUSY=5, RST=4, DC=2, CS=15, SCK=18, SDI=23
// GDEH0154D67_Display display;

// // Demo state variables
// static bool first_run = true;
// static unsigned long last_update_time = 0;
// static const unsigned long UPDATE_INTERVAL = 1000;  // Update every second

// /**
//  * Arduino setup function - runs once at startup
//  */
// void setup() {
//     // Initialize serial communication for debug output
//     Serial.begin(115200);
//     while (!Serial && millis() < 3000) {
//         // Wait up to 3 seconds for serial connection (for debugging)
//     }
    
//     Serial.println("=== BMO E-Paper Display Demo Starting ===");
//     Serial.println("Display: GDEH0154D67 1.54\" E-Ink");
//     Serial.println("Resolution: 200x200 pixels");
//     Serial.println("==========================================");
    
//     // Enable debug output from display controller
//     display.setDebugMode(true);
    
//     // Initialize GPIO pins for display communication
//     display.initializePins();
    
//     Serial.println("\\n--- Phase 1: Monochrome Display Demo ---");
    
//     // Initialize display for monochrome operation
//     if (!display.initializeMonochrome()) {
//         Serial.println("ERROR: Failed to initialize monochrome mode!");
//         Serial.print("Last error: ");
//         Serial.println(display.getLastError());
//         while (1) {
//             delay(1000);  // Halt on initialization failure
//         }
//     }
    
//     // Display first demo image (full screen monochrome)
//     Serial.println("Displaying sample image 1...");
//     display.displayFullScreenMono(gImage_1);  // From Ap_29demo.h
//     display.enterDeepSleep();  // Always sleep between operations
//     delay(2000);  // Wait 2 seconds for viewing
    
//     Serial.println("\\n--- Phase 2: 4-Grayscale Display Demo ---");
    
//     // Re-initialize for 4-grayscale mode
//     if (!display.initialize4Grayscale()) {
//         Serial.println("ERROR: Failed to initialize 4-grayscale mode!");
//         Serial.print("Last error: ");
//         Serial.println(display.getLastError());
//         while (1) {
//             delay(1000);  // Halt on initialization failure
//         }
//     }
    
//     // Display grayscale demo image
//     Serial.println("Displaying 4-grayscale image...");
//     display.displayFullScreen4Gray(gImage_11);  // From Ap_29demo.h
//     display.enterDeepSleep();
//     delay(2000);  // Wait 2 seconds for viewing
    
//     Serial.println("\\n--- Phase 3: Screen Clear Demo ---");
    
//     // Re-initialize for screen clearing
//     display.initializeMonochrome();
//     Serial.println("Clearing screen to white...");
//     display.clearScreen();
//     display.enterDeepSleep();
//     delay(2000);  // Wait 2 seconds for viewing
    
//     Serial.println("\\n--- Phase 4: Partial Refresh Demo Starting ---");
//     Serial.println("Beginning digital clock simulation...");
//     Serial.println("Note: Partial refresh reduces flicker and extends display life");
    
//     // Initialize for partial refresh demonstration
//     display.initializeMonochrome();
    
//     // Set up base image for partial refresh (background pattern)
//     display.setPartialRefreshBase(gImage_basemap);  // From Ap_29demo.h
    
//     Serial.println("Setup completed successfully!");
//     Serial.println("Starting main loop with digital clock demo...");
    
//     last_update_time = millis();
// }

// /**
//  * Arduino main loop - runs continuously
//  */
// void loop() {
//     static unsigned char minutes_high = 0;  // 10s of minutes (0-5)
//     static unsigned char minutes_low = 0;   // 1s of minutes (0-9)
//     static unsigned char seconds_high = 0;  // 10s of seconds (0-5)
//     static unsigned char seconds_low = 0;   // 1s of seconds (0-9)
//     static unsigned int update_count = 0;   // Track number of updates
    
//     // Check if it's time for the next update
//     unsigned long current_time = millis();
//     if (current_time - last_update_time < UPDATE_INTERVAL) {
//         return;  // Not time for update yet
//     }
    
//     last_update_time = current_time;
//     update_count++;
    
//     // Create array of regions for multi-region update
//     PartialRegion regions[5];
    
//     // Define digit positions for digital clock display: MM:SS
//     // Display layout: [M][M][:][S][S] where each digit is 32x64 pixels
//     // Total width needed: 5*32 = 160 pixels, centered on 200px display
//     // Starting X position: (200-160)/2 = 20 pixels
//     // Y position: centered vertically (200-64)/2 = 68 pixels
//     regions[0] = PartialRegion(0,  32, 64, 32, Num[minutes_high]); // Minutes tens
//     regions[1] = PartialRegion(40,  52, 64, 32, Num[minutes_low]);  // Minutes ones
//     regions[2] = PartialRegion(80,  84, 64, 32, gImage_numdot);     // Colon separator
//     regions[3] = PartialRegion(120,  116, 64, 32, Num[seconds_high]); // Seconds tens
//     regions[4] = PartialRegion(136, 200, 64, 32, Num[seconds_low]);  // Seconds ones

//     // Update all regions simultaneously
//     Serial.print("Clock update #");
//     Serial.print(update_count);
//     Serial.print(" - Time: ");
//     Serial.print(minutes_high);
//     Serial.print(minutes_low);
//     Serial.print(":");
//     Serial.print(seconds_high);
//     Serial.println(seconds_low);
    
//     if (!display.updateMultipleRegions(regions)) {
//         Serial.print("ERROR: Failed to update regions - ");
//         Serial.println(display.getLastError());
//     }
    
//     // Increment time counters (simulate clock progression)
//     seconds_low++;
//     if (seconds_low >= 10) {
//         seconds_low = 0;
//         seconds_high++;
//         if (seconds_high >= 6) {
//             seconds_high = 0;
//             minutes_low++;
//             if (minutes_low >= 10) {
//                 minutes_low = 0;
//                 minutes_high++;
//                 if (minutes_high >= 6) {
//                     // After 59:59, perform a full refresh to clear any ghosting
//                     minutes_high = 0;
//                     Serial.println("\\n=== Performing full refresh to clear ghosting ===");
//                     display.initializeMonochrome();
//                     display.clearScreen();
//                     display.setPartialRefreshBase(gImage_basemap);
//                     Serial.println("=== Full refresh completed, resuming clock ===\\n");
//                 }
//             }
//         }
//     }
    
//     // Perform periodic full refresh every 100 updates to prevent ghosting
//     // This is recommended best practice for e-paper displays
//     if (update_count % 100 == 0) {
//         Serial.println("\\n--- Periodic maintenance refresh ---");
//         display.initializeMonochrome();
//         display.setPartialRefreshBase(gImage_basemap);
//         Serial.println("--- Maintenance refresh completed ---\\n");
//     }
    
//     // Optional: Stop after a certain number of updates for demo purposes
//     if (update_count >= 300) {  // Run for 5 minutes then stop
//         Serial.println("\\n=== Demo completed after 5 minutes ===");
//         Serial.println("Entering final deep sleep...");
//         display.enterDeepSleep();
//         Serial.println("Demo finished. Reset to run again.");
        
//         // Infinite loop to stop execution
//         while (1) {
//             delay(10000);
//         }
//     }
// }

// /**
//  * Emergency error handler
//  */
// void handleError(const char* error_message) {
//     Serial.println("\\n!!! CRITICAL ERROR !!!");
//     Serial.print("Error: ");
//     Serial.println(error_message);
//     Serial.print("Display Error: ");
//     Serial.println(display.getLastError());
    
//     // Attempt to safely shut down display
//     display.enterDeepSleep();
    
//     Serial.println("System halted. Please reset to try again.");
    
//     // Flash built-in LED to indicate error state
//     pinMode(LED_BUILTIN, OUTPUT);
//     while (1) {
//         digitalWrite(LED_BUILTIN, HIGH);
//         delay(200);
//         digitalWrite(LED_BUILTIN, LOW);
//         delay(200);
//     }
// }

// /**
//  * Print system information
//  */
// void printSystemInfo() {
//     Serial.println("\\n=== System Information ===");
//     Serial.print("Free heap: ");
//     Serial.print(ESP.getFreeHeap());
//     Serial.println(" bytes");
    
//     Serial.print("Chip Model: ");
//     Serial.println(ESP.getChipModel());
//     Serial.print("Chip Revision: ");
//     Serial.println(ESP.getChipRevision());
    
//     Serial.print("CPU Frequency: ");
//     Serial.print(ESP.getCpuFreqMHz());
//     Serial.println(" MHz");
    
//     Serial.println("Display Configuration:");
//     Serial.print("  Width: ");
//     Serial.print(display.getWidth());
//     Serial.println(" pixels");
//     Serial.print("  Height: ");
//     Serial.print(display.getHeight());
//     Serial.println(" pixels");
//     Serial.print("  Mono buffer size: ");
//     Serial.print(display.getMonoBufferSize());
//     Serial.println(" bytes");
//     Serial.print("  Gray buffer size: ");
//     Serial.print(display.getGrayBufferSize());
//     Serial.println(" bytes");
    
//     Serial.println("=========================\\n");
// }
