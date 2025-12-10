#include "spinbot.h"
#include <iostream>


const uintptr_t YAW = 0x34;    // Cam horizontal
const uintptr_t PITCH = 0x38;  // Cam vertical

static float targetH = 0.0f;

void UpdateSpinbot(int speed, HANDLE hProcess, uintptr_t localPlayer, bool enabled) {

    targetH += speed;

    if (targetH > 360.0f) {
        targetH  = 0.0f;
    }
    WriteProcessMemory(hProcess, (LPVOID)(localPlayer + YAW), &targetH, sizeof(float), nullptr);
}
