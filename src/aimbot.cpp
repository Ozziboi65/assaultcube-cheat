#include "aimbot.h"
#include <cmath>
#include <limits>

const uintptr_t ENTITYLIST = 0x18AC04;
const uintptr_t HEALTH = 0xEC;
const uintptr_t TEAM = 0x30C;
const uintptr_t HEAD_X = 0x04;
const uintptr_t HEAD_Y = 0x08;
const uintptr_t HEAD_Z = 0x0C;
const uintptr_t YAW = 0x34;
const uintptr_t PITCH = 0x38;

struct Vec3 {
    float x, y, z;
};

float GetDistance(Vec3 a, Vec3 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

void CalculateAngles(Vec3 from, Vec3 to, float& yaw, float& pitch) {
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float dz = to.z - from.z;
    
    float distance = sqrtf(dx * dx + dy * dy);
    
    yaw = (atan2f(dy, dx) * (180.0f / 3.14159265f)) + 90.0f; //DO NOT FUCKIN TOUCH THIS SHIT IT WAS SO FUCKING ANNOYING TO FUCKING GET HOLY FUCKING SHIT OMG
    pitch = atan2f(dz, distance) * (180.0f / 3.14159265f);
}

void UpdateAimbot(bool all, HANDLE hProcess, uintptr_t moduleBase, uintptr_t localPlayer, bool enabled, float fov, float maxDistance, float headoffset, int legit_aim, int aimbot_aimat) {
    if (!enabled || !localPlayer) return;

    //bool is_random = false;

    int localTeam = 0;


    Vec3 localHead = { 0, 0, 0 };
    float currentYaw = 0.0f, currentPitch = 0.0f;
    ReadProcessMemory(hProcess, (LPCVOID)(localPlayer + TEAM), &localTeam, sizeof(int), nullptr);
    ReadProcessMemory(hProcess, (LPCVOID)(localPlayer + HEAD_X), &localHead.x, sizeof(float), nullptr);
    ReadProcessMemory(hProcess, (LPCVOID)(localPlayer + HEAD_Y), &localHead.y, sizeof(float), nullptr);
    ReadProcessMemory(hProcess, (LPCVOID)(localPlayer + HEAD_Z), &localHead.z, sizeof(float), nullptr);
    ReadProcessMemory(hProcess, (LPCVOID)(localPlayer + YAW), &currentYaw, sizeof(float), nullptr);
    ReadProcessMemory(hProcess, (LPCVOID)(localPlayer + PITCH), &currentPitch, sizeof(float), nullptr);


    uintptr_t entityList = 0;
    ReadProcessMemory(hProcess, (LPCVOID)(moduleBase + ENTITYLIST), &entityList, sizeof(uintptr_t), nullptr);
    if (!entityList) return;



    uintptr_t closestEnemy = 0;
    float closestDistance = 9999.0f;
    uintptr_t lowestHealthEnemy = 0;
    int lowestHealth = 9999;


    for (int i = 0; i < 64; i++) {
        uintptr_t entity = 0;
        ReadProcessMemory(hProcess, (LPCVOID)(entityList + i * 0x4), &entity, sizeof(uintptr_t), nullptr);
        if (!entity || entity == localPlayer) continue;

        int health = 0;
        int team = 0;
        ReadProcessMemory(hProcess, (LPCVOID)(entity + HEALTH), &health, sizeof(int), nullptr);
        ReadProcessMemory(hProcess, (LPCVOID)(entity + TEAM), &team, sizeof(int), nullptr);

        // skips enemy over the max distance for aimbot :3
        Vec3 enemyHead = { 0, 0, 0 };
        ReadProcessMemory(hProcess, (LPCVOID)(entity + HEAD_X), &enemyHead.x, sizeof(float), nullptr);
        ReadProcessMemory(hProcess, (LPCVOID)(entity + HEAD_Y), &enemyHead.y, sizeof(float), nullptr);
        ReadProcessMemory(hProcess, (LPCVOID)(entity + HEAD_Z), &enemyHead.z, sizeof(float), nullptr);

        float distance = GetDistance(localHead, enemyHead);
        if (distance > maxDistance) continue;

        if (health <= 0 || health > 100) continue;

        if(!all && team == localTeam) continue;

        // Calculate angles to this enemy
        float targetYaw, targetPitch;
        CalculateAngles(localHead, enemyHead, targetYaw, targetPitch);

        // Calculate angle difference (FOV check)
        float yawDiff = abs(targetYaw - currentYaw);
        float pitchDiff = abs(targetPitch - currentPitch);
        // Normalize yaw difference (handle wrapping at 180/-180)
        if (yawDiff > 180.0f) yawDiff = 360.0f - yawDiff;

        if (yawDiff > fov / 2.0f || pitchDiff > fov / 2.0f) continue;

        // Find closest enemy
        if (distance < closestDistance && distance > 1.0f) {
            closestDistance = distance;
            closestEnemy = entity;
        }
        // Find lowest health enemy
        if (health < lowestHealth && distance > 1.0f) {
            lowestHealth = health;
            lowestHealthEnemy = entity;
        }
    }

    // Aim at enemy
    uintptr_t targetEnemy = 0;
    if (aimbot_aimat == 0) {
        targetEnemy = lowestHealthEnemy;
    } else if (aimbot_aimat == 1) {
        targetEnemy = closestEnemy;
    }

    if (targetEnemy) {
        Vec3 enemyHead = { 0, 0, 0 };
        ReadProcessMemory(hProcess, (LPCVOID)(targetEnemy + HEAD_X), &enemyHead.x, sizeof(float), nullptr);
        ReadProcessMemory(hProcess, (LPCVOID)(targetEnemy + HEAD_Y), &enemyHead.y, sizeof(float), nullptr);
        ReadProcessMemory(hProcess, (LPCVOID)(targetEnemy + HEAD_Z), &enemyHead.z, sizeof(float), nullptr);

        enemyHead.z += 0.0f; //     aim up/down
        enemyHead.z += headoffset;

        float targetYaw, targetPitch;
        CalculateAngles(localHead, enemyHead, targetYaw, targetPitch);

        float smooth = legit_aim / 100.0f;
        if (smooth < 0.05f) smooth = 0.05f; // Prevent zero or too slow
        if (smooth > 1.0f) smooth = 1.0f;   // Prevent overshoot
        float deltaYaw = targetYaw - currentYaw;
        float deltaPitch = targetPitch - currentPitch;

        // Normalize yaw to handle wrap-around
        if (deltaYaw > 180.0f) deltaYaw -= 360.0f;
        if (deltaYaw < -180.0f) deltaYaw += 360.0f;

        // Calculate newYaw and newPitch with smoothing
        float newYaw = currentYaw + deltaYaw * smooth;
        float newPitch = currentPitch + deltaPitch * smooth;

        WriteProcessMemory(hProcess, (LPVOID)(localPlayer + YAW), &newYaw, sizeof(float), nullptr); // smooth yaw
        WriteProcessMemory(hProcess, (LPVOID)(localPlayer + PITCH), &newPitch, sizeof(float), nullptr); // smooth pitch
    }
}
