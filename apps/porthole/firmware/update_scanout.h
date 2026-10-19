// ESP32-S3 sparse, internal-RAM scan-out. Called only from the loop core that initialized the panel.
#pragma once
namespace update_scanout {
bool begin();
void draw(const char* label, bool cancellable);
void stop();
void release();
bool hitCancel(int logicalX, int logicalY);
}
