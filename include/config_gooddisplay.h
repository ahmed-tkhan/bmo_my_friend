#ifndef CONFIG_GOODDISPLAY_H
#define CONFIG_GOODDISPLAY_H

// Good Display GDEM0154I61 Configuration for ESP32 Dev Kit V1
// Display: 200x200 4-Grayscale (not just Black/White!)
// Interface: 4-wire SPI
// HAT: Good Display custom HAT with switch

// Display Configuration - 4 GRAYSCALE LEVELS
#define DISPLAY_WIDTH 200
#define DISPLAY_HEIGHT 200
#define DISPLAY_ROTATION 0
#define DISPLAY_GRAYSCALE_LEVELS 4  // NOT binary black/white!

// Good Display HAT Pin Configuration (ESP32 Dev Kit V1)
// Cable order: BUSY, RES, D/C, CS, SCK, SDI, GND, 3.3V
// Switch position: ? (you mentioned switch between 0.47 and 3)

#define EPD_BUSY    5     // GPIO5  - BUSY (first in cable order)
#define EPD_RST     4     // GPIO4  - RES  (Reset)
#define EPD_DC      2     // GPIO2  - D/C  (Data/Command)  
#define EPD_CS      15    // GPIO15 - CS   (Chip Select)
// SCK  = GPIO18 (Hardware SPI)
// SDI  = GPIO23 (Hardware SPI MOSI)
// GND  = GND
// 3.3V = 3.3V

// Alternative pin configuration (if above doesn't work)
// #define EPD_BUSY    16    // Alternative BUSY
// #define EPD_RST     17    // Alternative RST
// #define EPD_DC      21    // Alternative DC
// #define EPD_CS      22    // Alternative CS

// Device Configuration
#define DEVICE_ID "bmo_my_friend"
#define FIRMWARE_VERSION "1.0.0-bmo"

// Debug Configuration
#define ENABLE_DEBUG 1
#define DEBUG_LEVEL 3

// Good Display Specific Settings
#define GOOD_DISPLAY_MODEL "GDEM0154I61"
#define CONTROLLER_TYPE "SSD1681"
#define INTERFACE_TYPE "4-wire SPI"

#endif // CONFIG_GOODDISPLAY_H
