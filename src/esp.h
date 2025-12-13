#pragma once
#include <windows.h>


struct Vec2 { 
    float x, y; 
};

struct Vec3 { 
    float x, y, z; 
};


namespace Offsets {
    const uintptr_t LOCALPLAYER = 0x17E0A8;
    const uintptr_t ENTITYLIST = 0x18AC04;
    const uintptr_t VIEWMATRIX = 0x57DFD0;
    const uintptr_t HEALTH = 0xEC;
    const uintptr_t TEAM = 0x30C;
    const uintptr_t HEAD_X = 0x04;
    const uintptr_t HEAD_Y = 0x08;
    const uintptr_t HEAD_Z = 0x0C;
    const uintptr_t FEET_X = 0x28;
    const uintptr_t FEET_Y = 0x2C;
    const uintptr_t FEET_Z = 0x30;
    const uintptr_t ATTACK_DELAY = 0x160;  // Rapid fire
}


template<typename T>
T ReadMemory(HANDLE hProcess, uintptr_t address) {
    T value = {};
    ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), nullptr);
    return value;
}


bool WorldToScreen(Vec3 pos, float matrix[16], Vec2& screen, int screenWidth, int screenHeight);


void RenderESP(bool teamesp, HANDLE hProcess, uintptr_t moduleBase, int screenWidth, int screenHeight, bool snaplines, bool snaplinesall);
