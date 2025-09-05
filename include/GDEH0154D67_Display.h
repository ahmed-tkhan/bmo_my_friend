/**
 * @file GDEH0154D67_Display.h
 * @brief Professional E-Paper Display Controller for GDEH0154D67 1.54" E-Ink Display
 * 
 * This class provides comprehensive control for the GDEH0154D67 e-paper display,
 * featuring both monochrome and 4-grayscale display modes with partial refresh capabilities.
 * 
 * Display Specifications:
 * - Resolution: 200x200 pixels
 * - Colors: Monochrome (Black/White) or 4-Level Grayscale
 * - Interface: SPI
 * - Partial Refresh: Supported for fast updates
 * - Memory: 5000 bytes for full screen buffer
 * 
 * Key Features:
 * - Full screen refresh with flicker (clears ghosting)
 * - Partial refresh without flicker (fast updates)
 * - 4-grayscale mode support
 * - Multi-region partial updates
 * - Hardware SPI and bit-banged SPI support
 * - Comprehensive error handling and busy state monitoring
 * 
 * @author Generated from manufacturer code
 * @date 2025
 * @version 1.0
 */

#ifndef GDEH0154D67_DISPLAY_H
#define GDEH0154D67_DISPLAY_H

#include <Arduino.h>

/**
 * @brief Structure defining a partial refresh region
 */
struct PartialRegion {
    unsigned int x_start;        ///< Starting X coordinate
    unsigned int y_start;        ///< Starting Y coordinate  
    unsigned int width;          ///< Region width in pixels
    unsigned int height;         ///< Region height in pixels
    const unsigned char* data;   ///< Pointer to image data for this region
    
    /**
     * @brief Default constructor - creates invalid region
     */
    PartialRegion() : x_start(0), y_start(0), width(0), height(0), data(nullptr) {}
    
    /**
     * @brief Constructor with parameters
     */
    PartialRegion(unsigned int x, unsigned int y, unsigned int w, unsigned int h, const unsigned char* d)
        : x_start(x), y_start(y), width(w), height(h), data(d) {}
    
    /**
     * @brief Check if region is valid (has non-zero dimensions and data)
     */
    bool isValid() const { return width > 0 && height > 0 && data != nullptr; }
};

/**
 * @brief Main display controller class for GDEH0154D67 E-Paper Display
 * 
 * This class encapsulates all functionality needed to control the 1.54" e-paper display,
 * including initialization, data transfer, refresh modes, and power management.
 */
class GDEH0154D67_Display {
public:
    // ===== CONSTRUCTOR & DESTRUCTOR =====
    
    /**
     * @brief Construct a new display controller
     * @param busy_pin GPIO pin connected to BUSY signal (input)
     * @param rst_pin GPIO pin connected to RST/Reset signal (output) 
     * @param dc_pin GPIO pin connected to D/C (Data/Command) signal (output)
     * @param cs_pin GPIO pin connected to CS (Chip Select) signal (output)
     * @param sck_pin GPIO pin connected to SCK (SPI Clock) signal (output)
     * @param sdi_pin GPIO pin connected to SDI (SPI Data) signal (output)
     */
    GDEH0154D67_Display(int busy_pin = 5, int rst_pin = 4, int dc_pin = 2, 
                       int cs_pin = 15, int sck_pin = 18, int sdi_pin = 23);
    
    /**
     * @brief Destructor - ensures display is properly put to sleep
     */
    ~GDEH0154D67_Display();

    // ===== INITIALIZATION & SETUP =====
    
    /**
     * @brief Initialize GPIO pins for display communication
     * Must be called in setup() before any display operations
     */
    void initializePins();
    
    /**
     * @brief Initialize display for monochrome (black/white) mode
     * Performs hardware reset and configures display registers for 1-bit operation
     * @return true if initialization successful, false otherwise
     */
    bool initializeMonochrome();
    
    /**
     * @brief Initialize display for 4-grayscale mode
     * Configures display for 2-bit per pixel operation with custom LUT
     * @return true if initialization successful, false otherwise
     */
    bool initialize4Grayscale();

    // ===== FULL SCREEN OPERATIONS =====
    
    /**
     * @brief Display a full screen monochrome image
     * @param image_data Pointer to 5000-byte image buffer (200x200 pixels, 1 bit per pixel)
     * @param refresh_immediately If true, triggers display refresh after loading data
     * @note Image data should be in 1-bit format: 0x00=black, 0xFF=white
     */
    void displayFullScreenMono(const unsigned char* image_data, bool refresh_immediately = true);
    
    /**
     * @brief Display a full screen 4-grayscale image  
     * @param image_data Pointer to 10000-byte image buffer (200x200 pixels, 2 bits per pixel)
     * @param refresh_immediately If true, triggers display refresh after loading data
     * @note Image data should be in 2-bit format: 0x00=black, 0x55=dark gray, 0xAA=light gray, 0xFF=white
     */
    void displayFullScreen4Gray(const unsigned char* image_data, bool refresh_immediately = true);
    
    /**
     * @brief Fill entire screen with white color
     * Useful for clearing the display or resetting image retention
     */
    void clearScreen();

    // ===== PARTIAL REFRESH OPERATIONS =====
    
    /**
     * @brief Set the base image for partial refresh operations
     * @param base_image Pointer to 5000-byte base image that will remain static
     * @note This image serves as background for all subsequent partial updates
     */
    void setPartialRefreshBase(const unsigned char* base_image);
    
    /**
     * @brief Update a rectangular region of the display (single region)
     * @param x_start Starting X coordinate (pixels)
     * @param y_start Starting Y coordinate (pixels) 
     * @param image_data Pointer to image data for this region
     * @param width Width of region in pixels
     * @param height Height of region in pixels
     * @return true if update successful, false if coordinates invalid
     */
    bool updatePartialRegion(unsigned int x_start, unsigned int y_start, 
                           const unsigned char* image_data, 
                           unsigned int width, unsigned int height);
    
    /**
     * @brief Update up to 5 different regions simultaneously
     * @param regions Array of 5 region definitions (unused regions should have width=0)
     * @return true if all valid regions updated successfully
     * @note This is optimized for applications like digital clocks with multiple digits
     */
    bool updateMultipleRegions(const PartialRegion regions[5]);

    // ===== DISPLAY REFRESH & UPDATE =====
    
    /**
     * @brief Trigger full screen refresh (with flicker)
     * Use this for complete image changes or to clear ghosting every 5-10 partial updates
     */
    void refreshFull();
    
    /**
     * @brief Trigger partial refresh (no flicker)  
     * Use this for fast updates of small regions
     */
    void refreshPartial();
    
    /**
     * @brief Trigger 4-grayscale refresh
     * Special refresh sequence required for grayscale mode
     */
    void refresh4Grayscale();

    // ===== POWER MANAGEMENT =====
    
    /**
     * @brief Put display into deep sleep mode
     * @note Essential for power saving and display longevity - always call when done
     * @warning Display will need re-initialization after wake-up
     */
    void enterDeepSleep();
    
    /**
     * @brief Check if display is currently busy with an operation
     * @return true if display is busy, false if ready for new commands
     */
    bool isBusy();
    
    /**
     * @brief Wait for display to finish current operation
     * @param timeout_ms Maximum time to wait in milliseconds (0 = infinite)
     * @return true if display became ready, false if timeout occurred
     */
    bool waitForReady(unsigned long timeout_ms = 0);

    // ===== UTILITY FUNCTIONS =====
    
    /**
     * @brief Get display width in pixels
     * @return Display width (200 pixels)
     */
    static constexpr unsigned int getWidth() { return DISPLAY_WIDTH; }
    
    /**
     * @brief Get display height in pixels  
     * @return Display height (200 pixels)
     */
    static constexpr unsigned int getHeight() { return DISPLAY_HEIGHT; }
    
    /**
     * @brief Get full screen buffer size in bytes
     * @return Buffer size for monochrome mode (5000 bytes)
     */
    static constexpr unsigned int getMonoBufferSize() { return MONO_BUFFER_SIZE; }
    
    /**
     * @brief Get 4-grayscale buffer size in bytes
     * @return Buffer size for 4-gray mode (10000 bytes)  
     */
    static constexpr unsigned int getGrayBufferSize() { return GRAY_BUFFER_SIZE; }

    // ===== DEBUGGING & DIAGNOSTICS =====
    
    /**
     * @brief Enable or disable debug output
     * @param enable true to enable debug messages, false to disable
     */
    void setDebugMode(bool enable) { debug_enabled_ = enable; }
    
    /**
     * @brief Get last error message
     * @return String describing the last error that occurred
     */
    const char* getLastError() { return last_error_; }

private:
    // ===== HARDWARE CONSTANTS =====
    static constexpr unsigned int DISPLAY_WIDTH = 200;    ///< Display width in pixels
    static constexpr unsigned int DISPLAY_HEIGHT = 200;   ///< Display height in pixels  
    static constexpr unsigned int MONO_BUFFER_SIZE = 5000; ///< Monochrome buffer size
    static constexpr unsigned int GRAY_BUFFER_SIZE = 10000; ///< 4-grayscale buffer size
    static constexpr unsigned int MAX_LINE_BYTES = 25;    ///< Bytes per line (200/8)
    static constexpr unsigned int MAX_COLUMN_BYTES = 200; ///< Bytes per column

    // ===== GPIO PIN ASSIGNMENTS =====
    int busy_pin_;  ///< BUSY signal pin (input)
    int rst_pin_;   ///< Reset signal pin (output)
    int dc_pin_;    ///< Data/Command signal pin (output) 
    int cs_pin_;    ///< Chip select signal pin (output)
    int sck_pin_;   ///< SPI clock signal pin (output)
    int sdi_pin_;   ///< SPI data signal pin (output)

    // ===== STATE VARIABLES =====
    bool initialized_;     ///< Whether display has been initialized
    bool debug_enabled_;   ///< Whether debug output is enabled
    const char* last_error_; ///< Last error message

    // ===== LOW-LEVEL SPI COMMUNICATION =====
    
    /**
     * @brief Write a single byte via bit-banged SPI
     * @param value Byte to transmit
     */
    void spiWrite(unsigned char value);
    
    /**
     * @brief Send command to display
     * @param cmd Command byte to send
     */
    void writeCommand(unsigned char cmd);
    
    /**
     * @brief Send data byte to display
     * @param data Data byte to send  
     */
    void writeData(unsigned char data);
    
    /**
     * @brief Wait for display BUSY signal to go low
     * Blocks until display is ready for next operation
     */
    void waitBusy();

    // ===== TIMING & DELAY FUNCTIONS =====
    
    /**
     * @brief Microsecond delay
     * @param microseconds Delay time in microseconds
     */
    void delayMicroseconds(unsigned int microseconds);
    
    /**
     * @brief Millisecond delay
     * @param milliseconds Delay time in milliseconds  
     */
    void delayMilliseconds(unsigned long milliseconds);
    
    /**
     * @brief SPI timing delay
     * @param rate Delay rate (higher = longer delay)
     */
    void spiDelay(unsigned char rate);

    // ===== 4-GRAYSCALE PROCESSING =====
    
    /**
     * @brief Convert 2 grayscale bytes to 1 RAM1 byte
     * @param data1 First grayscale byte
     * @param data2 Second grayscale byte
     * @return Processed byte for RAM1
     */
    unsigned char convertGray2ToRam1(unsigned char data1, unsigned char data2);
    
    /**
     * @brief Convert 2 grayscale bytes to 1 RAM2 byte  
     * @param data1 First grayscale byte
     * @param data2 Second grayscale byte
     * @return Processed byte for RAM2
     */
    unsigned char convertGray2ToRam2(unsigned char data1, unsigned char data2);
    
    /**
     * @brief Load custom lookup table for 4-grayscale mode
     * @param wave_data Pointer to 159-byte LUT data
     */
    void loadGrayscaleLUT(const unsigned char* wave_data);

    // ===== GPIO CONTROL MACROS =====
    
    /**
     * @brief Set MOSI line high
     */
    void setMOSI_High() { digitalWrite(sdi_pin_, HIGH); }
    
    /**
     * @brief Set MOSI line low  
     */
    void setMOSI_Low() { digitalWrite(sdi_pin_, LOW); }
    
    /**
     * @brief Set clock line high
     */
    void setCLK_High() { digitalWrite(sck_pin_, HIGH); }
    
    /**
     * @brief Set clock line low
     */
    void setCLK_Low() { digitalWrite(sck_pin_, LOW); }
    
    /**
     * @brief Set chip select low (active)
     */
    void setCS_Active() { digitalWrite(cs_pin_, LOW); }
    
    /**
     * @brief Set chip select high (inactive)
     */
    void setCS_Inactive() { digitalWrite(cs_pin_, HIGH); }
    
    /**
     * @brief Set data/command line for command mode
     */
    void setDC_Command() { digitalWrite(dc_pin_, LOW); }
    
    /**
     * @brief Set data/command line for data mode
     */
    void setDC_Data() { digitalWrite(dc_pin_, HIGH); }
    
    /**
     * @brief Set reset line low (active reset)
     */
    void setRST_Active() { digitalWrite(rst_pin_, LOW); }
    
    /**
     * @brief Set reset line high (normal operation)
     */
    void setRST_Inactive() { digitalWrite(rst_pin_, HIGH); }
    
    /**
     * @brief Read BUSY signal state
     * @return true if display is busy, false if ready
     */
    bool readBusy() { return digitalRead(busy_pin_) == HIGH; }

    // ===== DEBUG HELPERS =====
    
    /**
     * @brief Print debug message if debug mode enabled
     * @param message Message to print
     */
    void debugPrint(const char* message);
    
    /**
     * @brief Set last error message
     * @param error Error message to store
     */
    void setError(const char* error);
};

#endif // GDEH0154D67_DISPLAY_H
