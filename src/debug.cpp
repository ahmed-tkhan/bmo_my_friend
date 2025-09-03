#include "debug.h"
#include <ESP.h>

void printSeparator(const String& title) {
    Serial.println("=====================================");
    if (title.length() > 0) {
        Serial.println(title);
        Serial.println("=====================================");
    }
}

void printDebugInfo() {
    printSeparator("System Information");
    DEBUG_INFO("Chip Model: %s", ESP.getChipModel());
    DEBUG_INFO("Chip Revision: %d", ESP.getChipRevision());
    DEBUG_INFO("CPU Frequency: %d MHz", ESP.getCpuFreqMHz());
    DEBUG_INFO("Flash Size: %.2f MB", ESP.getFlashChipSize() / 1024.0 / 1024.0);
    DEBUG_INFO("Free Heap: %d bytes", ESP.getFreeHeap());
    DEBUG_INFO("SDK Version: %s", ESP.getSdkVersion());
}
