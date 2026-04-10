#include "emotion_manager.h"
#include <FS.h>
#include <SPIFFS.h>

EmotionManager::EmotionManager(GDEH0154D67_Display* disp)
    : display_(disp), fileCount_(0) {}

void EmotionManager::begin() {
    fileCount_ = 0;
    File root = SPIFFS.open("/");
    if (!root || !root.isDirectory()) {
        return;
    }
    File file = root.openNextFile();
    while (file && fileCount_ < MAX_FILES) {
        String name = file.name();
        if (name.endsWith(".bin")) {
            if (!name.startsWith("/")) name = "/" + name;
            fileList_[fileCount_++] = name;
        }
        file = root.openNextFile();
    }
}

const String& EmotionManager::nameAt(int idx) const {
    static String empty = String("");
    if (idx < 0 || idx >= fileCount_) return empty;
    return fileList_[idx];
}

bool EmotionManager::showByIndex(int idx) {
    if (idx < 0 || idx >= fileCount_) return false;
    File f = SPIFFS.open(fileList_[idx], "r");
    if (!f) return false;
    size_t fsize = f.size();
    size_t monoSize = display_->getMonoBufferSize();
    size_t graySize = display_->getGrayBufferSize();
    bool ok = false;
    if (fsize == monoSize) {
        uint8_t* buf = (uint8_t*)malloc(fsize);
        if (buf) {
            f.read(buf, fsize);
            display_->initializeMonochrome();
            display_->displayFullScreenMono(buf, true);
            free(buf);
            ok = true;
        }
    } else if (fsize == graySize) {
        uint8_t* buf = (uint8_t*)malloc(fsize);
        if (buf) {
            f.read(buf, fsize);
            display_->displayFullScreen4Gray(buf, true);
            free(buf);
            ok = true;
        }
    }
    f.close();
    return ok;
}

bool EmotionManager::showByName(const String& name) {
    if (name.length() == 0) return false;
    String target = name;
    if (!target.startsWith("/")) target = "/" + target;
    String targetUp = target;
    targetUp.toUpperCase();
    for (int i = 0; i < fileCount_; ++i) {
        String cand = fileList_[i];
        String candUp = cand;
        candUp.toUpperCase();
        if (candUp == targetUp || candUp.endsWith(targetUp)) {
            return showByIndex(i);
        }
    }
    return false;
}
