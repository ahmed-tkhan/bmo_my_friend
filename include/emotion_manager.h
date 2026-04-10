#ifndef EMOTION_MANAGER_H
#define EMOTION_MANAGER_H

#include <Arduino.h>
#include "GDEH0154D67_Display.h"

class EmotionManager {
public:
    EmotionManager(GDEH0154D67_Display* disp);
    // Scan SPIFFS root for .bin files and populate internal list
    void begin();

    // Return number of available emotion files
    int count() const { return fileCount_; }
    // Get filename by index
    const String& nameAt(int idx) const;

    // Show emotion by index (0-based)
    bool showByIndex(int idx);
    // Show emotion by (partial) name match
    bool showByName(const String& name);

private:
    GDEH0154D67_Display* display_;
    static const int MAX_FILES = 64;
    String fileList_[MAX_FILES];
    int fileCount_;
};

#endif // EMOTION_MANAGER_H
