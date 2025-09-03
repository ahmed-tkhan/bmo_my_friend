#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>
#include "config_gooddisplay.h"

// Debug levels
#define DEBUG_LEVEL_ERROR 0
#define DEBUG_LEVEL_WARN  1 
#define DEBUG_LEVEL_INFO  2
#define DEBUG_LEVEL_DEBUG 3

// Debug macros
#if ENABLE_DEBUG
  #define DEBUG_ERROR(fmt, ...) if(DEBUG_LEVEL >= DEBUG_LEVEL_ERROR) { Serial.printf("[ERROR] " fmt "\n", ##__VA_ARGS__); }
  #define DEBUG_WARN(fmt, ...)  if(DEBUG_LEVEL >= DEBUG_LEVEL_WARN)  { Serial.printf("[WARN]  " fmt "\n", ##__VA_ARGS__); }
  #define DEBUG_INFO(fmt, ...)  if(DEBUG_LEVEL >= DEBUG_LEVEL_INFO)  { Serial.printf("[INFO]  " fmt "\n", ##__VA_ARGS__); }
  #define DEBUG_DBG(fmt, ...)   if(DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG) { Serial.printf("[DEBUG] " fmt "\n", ##__VA_ARGS__); }
#else
  #define DEBUG_ERROR(fmt, ...)
  #define DEBUG_WARN(fmt, ...)
  #define DEBUG_INFO(fmt, ...)
  #define DEBUG_DBG(fmt, ...)
#endif

// Helper functions
void printSeparator(const String& title);
void printDebugInfo();

#endif // DEBUG_H
